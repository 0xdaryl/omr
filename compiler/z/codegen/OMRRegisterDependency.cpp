/*******************************************************************************
 * Copyright (c) 2000, 2020 IBM Corp. and others
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License 2.0 which accompanies this
 * distribution and is available at http://eclipse.org/legal/epl-2.0
 * or the Apache License, Version 2.0 which accompanies this distribution
 * and is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * This Source Code may also be made available under the following Secondary
 * Licenses when the conditions for such availability set forth in the
 * Eclipse Public License, v. 2.0 are satisfied: GNU General Public License,
 * version 2 with the GNU Classpath Exception [1] and GNU General Public
 * License, version 2 with the OpenJDK Assembly Exception [2].
 *
 * [1] https://www.gnu.org/software/classpath/license.html
 * [2] http://openjdk.java.net/legal/assembly-exception.html
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0 WITH Classpath-exception-2.0 OR LicenseRef-GPL-2.0 WITH Assembly-exception
 *******************************************************************************/

//On zOS XLC linker can't handle files with same name at link time
//This workaround with pragma is needed. What this does is essentially
//give a different name to the codesection (csect) for this file. So it
//doesn't conflict with another file with same name.

#pragma csect(CODE,"OMRZRegisterDependency#C")
#pragma csect(STATIC,"OMRZRegisterDependency#S")
#pragma csect(TEST,"OMRZRegisterDependency#T")

#include <stddef.h>
#include <stdint.h>
#include "codegen/BackingStore.hpp"
#include "codegen/CodeGenerator.hpp"
#include "env/FrontEnd.hpp"
#include "codegen/InstOpCode.hpp"
#include "codegen/Instruction.hpp"
#include "codegen/Machine.hpp"
#include "codegen/MemoryReference.hpp"
#include "codegen/RealRegister.hpp"
#include "codegen/Register.hpp"
#include "codegen/RegisterConstants.hpp"
#include "codegen/RegisterDependency.hpp"
#include "codegen/RegisterDependencyGroup.hpp"
#include "codegen/RegisterDependencyStruct.hpp"
#include "codegen/RegisterPair.hpp"
#include "compile/Compilation.hpp"
#include "control/Options.hpp"
#include "control/Options_inlines.hpp"
#include "env/CompilerEnv.hpp"
#include "env/ObjectModel.hpp"
#include "env/TRMemory.hpp"
#include "il/AutomaticSymbol.hpp"
#include "il/DataTypes.hpp"
#include "il/ILOpCodes.hpp"
#include "il/ILOps.hpp"
#include "il/LabelSymbol.hpp"
#include "il/Node.hpp"
#include "il/Node_inlines.hpp"
#include "il/Symbol.hpp"
#include "il/SymbolReference.hpp"
#include "infra/Assert.hpp"
#include "infra/List.hpp"
#include "ras/Debug.hpp"
#include "z/codegen/S390GenerateInstructions.hpp"
#include "z/codegen/S390Instruction.hpp"
#include "env/IO.hpp"

OMR::Z::RegisterDependencyConditions::RegisterDependencyConditions(TR::CodeGenerator * cg, TR::Node * node,
   uint32_t          extranum, TR::Instruction ** cursorPtr)
   {
   TR::Compilation *comp = cg->comp();
   _cg = cg;
   List<TR::Register> regList(cg->trMemory());
   TR::Instruction * iCursor = (cursorPtr == NULL) ? NULL : *cursorPtr;
   int32_t totalNum = node->getNumChildren() + extranum;
   int32_t i;

   comp->incVisitCount();

   int32_t numLongs = 0;
   int32_t numLongDoubles = 0;
   //
   // Pre-compute how many longs are global register candidates
   //
   for (i = 0; i < node->getNumChildren(); ++i)
      {
      TR::Node * child = node->getChild(i);
      TR::Register * reg = child->getRegister();
      if (reg->getKind() == TR_GPR)
         {
         if (child->getHighGlobalRegisterNumber() > -1)
            {
            numLongs++;
            }
         }
      }

   totalNum = totalNum + numLongs + numLongDoubles;

   _preConditions = TR::RegisterDependencyGroup::create(totalNum, cg->trMemory());
   _postConditions = TR::RegisterDependencyGroup::create(totalNum, cg->trMemory());
   _numPreConditions = totalNum;
   _addCursorForPre = 0;
   _numPostConditions = totalNum;
   _addCursorForPost = 0;
   _isUsed = false;

   for (i = 0; i < totalNum; i++)
      {
      _preConditions->clearDependencyInfo(i);
      _postConditions->clearDependencyInfo(i);
      }

   for (i = 0; i < node->getNumChildren(); i++)
      {
      TR::Node * child = node->getChild(i);
      TR::Register * reg = child->getRegister();
      TR::Register * highReg = NULL;
      TR::Register * copyReg = NULL;
      TR::Register * highCopyReg = NULL;

      TR::RealRegister::RegNum regNum = (TR::RealRegister::RegNum)
         cg->getGlobalRegister(child->getGlobalRegisterNumber());


      TR::RealRegister::RegNum highRegNum;
      if (child->getHighGlobalRegisterNumber() > -1)
         {
         highRegNum = (TR::RealRegister::RegNum) cg->getGlobalRegister(child->getHighGlobalRegisterNumber());
         }

      TR::RegisterPair * regPair = reg->getRegisterPair();
      resolveSplitDependencies(cg, node, child, regPair, regList, reg, highReg, copyReg, highCopyReg, iCursor, regNum);

      reg->setAssociation(regNum);
      cg->setRealRegisterAssociation(reg, regNum);

      addPreCondition(reg, regNum);
      addPostCondition(reg, regNum);

      if (copyReg != NULL)
         {
         cg->stopUsingRegister(copyReg);
         }

      if (highReg)
         {
         highReg->setAssociation(highRegNum);
         cg->setRealRegisterAssociation(highReg, highRegNum);
         addPreCondition(highReg, highRegNum);
         addPostCondition(highReg, highRegNum);
         if (highCopyReg != NULL)
            {
            cg->stopUsingRegister(highCopyReg);
            }
         }
      }

   if (iCursor != NULL && cursorPtr != NULL)
      {
      *cursorPtr = iCursor;
      }
   }

OMR::Z::RegisterDependencyConditions::RegisterDependencyConditions(TR::RegisterDependencyConditions* iConds, uint16_t numNewPreConds, uint16_t numNewPostConds, TR::CodeGenerator * cg)
   : _preConditions(TR::RegisterDependencyGroup::create((iConds?iConds->getNumPreConditions():0)+numNewPreConds, cg->trMemory())),
     _postConditions(TR::RegisterDependencyGroup::create((iConds?iConds->getNumPostConditions():0)+numNewPostConds, cg->trMemory())),
     _numPreConditions((iConds?iConds->getNumPreConditions():0)+numNewPreConds),
     _addCursorForPre(0),
     _numPostConditions((iConds?iConds->getNumPostConditions():0)+numNewPostConds),
     _addCursorForPost(0),
     _isUsed(false),
     _cg(cg)
   {
     int32_t i;

     TR::RegisterDependencyGroup* depGroup;
     uint16_t numPreConds = (iConds?iConds->getNumPreConditions():0)+numNewPreConds;
     uint16_t numPostConds = (iConds?iConds->getNumPostConditions():0)+numNewPostConds;
     uint32_t flag;
     TR::RealRegister::RegNum rr;
     TR::Register* vr;
     for(i=0;i<numPreConds;i++)
        {
        _preConditions->clearDependencyInfo(i);
        }
     for(int32_t j=0;j<numPostConds;j++)
        {
        _postConditions->clearDependencyInfo(j);
        }

     if (iConds != NULL)
        {
        depGroup = iConds->getPreConditions();
        //      depGroup->printDeps(stdout, iConds->getNumPreConditions());
        for(i=0;i<iConds->getAddCursorForPre();i++)
           {
          TR::RegisterDependency* dep = depGroup->getRegisterDependency(i);

           flag = dep->getFlags();
           vr   = dep->getRegister();
           rr   = dep->getRealRegister();

           _preConditions->setDependencyInfo(_addCursorForPre++, vr, rr, flag);
           }

        depGroup = iConds->getPostConditions();
        //        depGroup->printDeps(stdout, iConds->getNumPostConditions());
        for(i=0;i<iConds->getAddCursorForPost();i++)
           {
           TR::RegisterDependency* dep = depGroup->getRegisterDependency(i);

           flag = dep->getFlags();
           vr   = dep->getRegister();
           rr   = dep->getRealRegister();

           _postConditions->setDependencyInfo(_addCursorForPost++, vr, rr, flag);
           }
        }
   }

OMR::Z::RegisterDependencyConditions::RegisterDependencyConditions(TR::RegisterDependencyConditions * conds_1,
                                     TR::RegisterDependencyConditions * conds_2,
                                     TR::CodeGenerator * cg)
   : _preConditions(TR::RegisterDependencyGroup::create(conds_1->getNumPreConditions()+conds_2->getNumPreConditions(), cg->trMemory())),
     _postConditions(TR::RegisterDependencyGroup::create(conds_1->getNumPostConditions()+conds_2->getNumPostConditions(), cg->trMemory())),
     _numPreConditions(conds_1->getNumPreConditions()+conds_2->getNumPreConditions()),
     _addCursorForPre(0),
     _numPostConditions(conds_1->getNumPostConditions()+conds_2->getNumPostConditions()),
     _addCursorForPost(0),
     _isUsed(false),
     _cg(cg)
   {
   int32_t i;
   TR::RegisterDependencyGroup* depGroup;
   uint32_t flag;
   TR::RealRegister::RegNum rr;
   TR::Register* vr;

   for( i = 0; i < _numPreConditions; i++)
      {
      _preConditions->clearDependencyInfo(i);
      }

   for( i = 0; i < _numPostConditions; i++)
      {
      _postConditions->clearDependencyInfo(i);
      }

   // Merge pre conditions
   depGroup = conds_1->getPreConditions();
   for( i = 0; i < conds_1->getAddCursorForPre(); i++)
      {
      TR::RegisterDependency* dep = depGroup->getRegisterDependency(i);

      if( doesPreConditionExist( dep ) )
         {
         _numPreConditions--;
         }
      else if( !addPreConditionIfNotAlreadyInserted( dep ) )
         {
         _numPreConditions--;
         }
      }

   depGroup = conds_2->getPreConditions();
   for( i = 0; i < conds_2->getAddCursorForPre(); i++)
      {
      TR::RegisterDependency* dep = depGroup->getRegisterDependency(i);

      if( doesPreConditionExist( dep ) )
         {
         _numPreConditions--;
         }
      else if( !addPreConditionIfNotAlreadyInserted( dep ) )
         {
         _numPreConditions--;
         }
      }

   // Merge post conditions
   depGroup = conds_1->getPostConditions();
   for( i = 0; i < conds_1->getAddCursorForPost(); i++)
      {
      TR::RegisterDependency* dep = depGroup->getRegisterDependency(i);

      if( doesPostConditionExist( dep ) )
         {
         _numPostConditions--;
         }
      else if( !addPostConditionIfNotAlreadyInserted( dep ) )
         {
         _numPostConditions--;
         }
      }

   depGroup = conds_2->getPostConditions();
   for( i = 0; i < conds_2->getAddCursorForPost(); i++)
      {
      TR::RegisterDependency* dep = depGroup->getRegisterDependency(i);

      if( doesPostConditionExist( dep ) )
         {
         _numPostConditions--;
         }
      else if( !addPostConditionIfNotAlreadyInserted( dep ) )
         {
         _numPostConditions--;
         }
      }
   }

void OMR::Z::RegisterDependencyConditions::resolveSplitDependencies(
   TR::CodeGenerator* cg,
   TR::Node* node, TR::Node* child,
   TR::RegisterPair* regPair, List<TR::Register>& regList,
   TR::Register*& reg, TR::Register*& highReg,
   TR::Register*& copyReg, TR::Register*& highCopyReg,
   TR::Instruction*& iCursor,
   TR::RealRegister::RegNum& regNum)
   {
   TR::Compilation *comp = cg->comp();
   TR_Debug * debugObj = cg->getDebug();
   bool foundLow = false;
   bool foundHigh = false;
   if (regPair)
      {
      foundLow = regList.find(regPair->getLowOrder());
      foundHigh = regList.find(regPair->getHighOrder());
      }
   else
      {
      foundLow = regList.find(reg);
      }

   if (foundLow || foundHigh)
      {
      TR::InstOpCode::Mnemonic opCode;
      TR_RegisterKinds kind = reg->getKind();
      bool isVector = kind == TR_VRF ? true : false;
      if (kind == TR_GPR || kind == TR_FPR)
         {
         if (regPair)
            {
            highReg = regPair->getHighOrder();
            reg = regPair->getLowOrder();
            }
         else if (child->getHighGlobalRegisterNumber() > -1)
            {
            TR_ASSERT( 0,"Long child does not have a register pair\n");
            }
         }

      bool containsInternalPointer = false;
      if (reg->getPinningArrayPointer())
         {
         containsInternalPointer = true;
         }

      copyReg = (reg->containsCollectedReference() && !containsInternalPointer) ?
         cg->allocateCollectedReferenceRegister() :
         cg->allocateRegister(kind);

      if (containsInternalPointer)
         {
         copyReg->setContainsInternalPointer();
         copyReg->setPinningArrayPointer(reg->getPinningArrayPointer());
         }

      switch (kind)
         {
         case TR_GPR:
            if (reg->is64BitReg())
               {
               opCode = TR::InstOpCode::LGR;
               }
            else
               {
               opCode = TR::InstOpCode::LR;
               }
            break;
         case TR_FPR:
            opCode = TR::InstOpCode::LDR;
            break;
         case TR_VRF:
            opCode = TR::InstOpCode::VLR;
            break;
         default:
            TR_ASSERT( 0, "Invalid register kind.");
         }

      iCursor = isVector ? generateVRRaInstruction(cg, opCode, node, copyReg, reg, iCursor):
                           generateRRInstruction  (cg, opCode, node, copyReg, reg, iCursor);

      reg = copyReg;
      if (debugObj)
         {
         if (isVector)
            debugObj->addInstructionComment( toS390VRRaInstruction(iCursor), "VLR=Reg_deps2");
         else
            debugObj->addInstructionComment( toS390RRInstruction(iCursor), "LR=Reg_deps2");
         }

      if (highReg)
        {
        if(kind == TR_GPR)
         {
         containsInternalPointer = false;
         if (highReg->getPinningArrayPointer())
            {
            containsInternalPointer = true;
            }

         highCopyReg = (highReg->containsCollectedReference() && !containsInternalPointer) ?
            cg->allocateCollectedReferenceRegister() :
            cg->allocateRegister(kind);

         if (containsInternalPointer)
            {
            highCopyReg->setContainsInternalPointer();
            highCopyReg->setPinningArrayPointer(highReg->getPinningArrayPointer());
            }

         iCursor = generateRRInstruction(cg, opCode, node, highCopyReg, highReg, iCursor);
         highReg = highCopyReg;
         }
        else // kind == FPR
         {
         highCopyReg = cg->allocateRegister(kind);
         iCursor = generateRRInstruction  (cg, opCode, node, highCopyReg, highReg, iCursor);
         debugObj->addInstructionComment( toS390RRInstruction(iCursor), "LR=Reg_deps2");
         highReg = highCopyReg;
         }
        }
      }
   else
      {
      if (regPair)
         {
         regList.add(regPair->getHighOrder());
         regList.add(regPair->getLowOrder());
         }
      else
         {
         regList.add(reg);
         }

      if (regPair)
         {
         highReg = regPair->getHighOrder();
         reg = regPair->getLowOrder();
         }
      else if ((child->getHighGlobalRegisterNumber() > -1))
         {
         TR_ASSERT( 0, "Long child does not have a register pair\n");
         }
      }
   }

TR::RegisterDependencyConditions  *OMR::Z::RegisterDependencyConditions::clone(TR::CodeGenerator *cg, int32_t additionalRegDeps)
   {
   TR::RegisterDependencyConditions  *other =
      new (cg->trHeapMemory()) TR::RegisterDependencyConditions(_numPreConditions  + additionalRegDeps,
                                                                   _numPostConditions + additionalRegDeps, cg);
   int32_t i = 0;

   for (i = _numPreConditions-1; i >= 0; --i)
      {
      TR::RegisterDependency  *dep = getPreConditions()->getRegisterDependency(i);
      other->getPreConditions()->setDependencyInfo(i, dep->getRegister(), dep->getRealRegister(), dep->getFlags());
      }

   for (i = _numPostConditions-1; i >= 0; --i)
      {
      TR::RegisterDependency  *dep = getPostConditions()->getRegisterDependency(i);
      other->getPostConditions()->setDependencyInfo(i, dep->getRegister(), dep->getRealRegister(), dep->getFlags());
      }

   other->setAddCursorForPre(_addCursorForPre);
   other->setAddCursorForPost(_addCursorForPost);
   return other;
   }

bool
OMR::Z::RegisterDependencyConditions::refsRegister(TR::Register * r)
   {
   for (int32_t i = 0; i < _numPreConditions; i++)
      {
      if (_preConditions->getRegisterDependency(i)->getRegister() == r &&
         _preConditions->getRegisterDependency(i)->getRefsRegister())
         {
         return true;
         }
      }
   for (int32_t j = 0; j < _numPostConditions; j++)
      {
      if (_postConditions->getRegisterDependency(j)->getRegister() == r &&
         _postConditions->getRegisterDependency(j)->getRefsRegister())
         {
         return true;
         }
      }
   return false;
   }

bool
OMR::Z::RegisterDependencyConditions::defsRegister(TR::Register * r)
   {
   for (int32_t i = 0; i < _numPreConditions; i++)
      {
      if (_preConditions->getRegisterDependency(i)->getRegister() == r &&
         _preConditions->getRegisterDependency(i)->getDefsRegister())
         {
         return true;
         }
      }
   for (int32_t j = 0; j < _numPostConditions; j++)
      {
      if (_postConditions->getRegisterDependency(j)->getRegister() == r &&
         _postConditions->getRegisterDependency(j)->getDefsRegister())
         {
         return true;
         }
      }
   return false;
   }

bool
OMR::Z::RegisterDependencyConditions::usesRegister(TR::Register * r)
   {
   for (int32_t i = 0; i < _numPreConditions; i++)
      {
      if (_preConditions->getRegisterDependency(i)->getRegister() == r &&
          (_preConditions->getRegisterDependency(i)->getRefsRegister() || _preConditions->getRegisterDependency(i)->getDefsRegister()))
         {
         return true;
         }
      }

   for (int32_t j = 0; j < _numPostConditions; j++)
      {
      if (_postConditions->getRegisterDependency(j)->getRegister() == r &&
          (_postConditions->getRegisterDependency(j)->getRefsRegister() || _postConditions->getRegisterDependency(j)->getDefsRegister()))
         {
         return true;
         }
      }
   return false;
   }

/**
 * Reason for oldPreCursor/oldPostCursor:
 * When the dependencies have been created by merging new registers into an existing list then only call useRegister for the new registers in the list.
 * Calling useRegister again on the existing registers will cause the totalUseCount to be too high and the virtual register will remain set to the real register
 * for the entire duration of register assignment.
 */
void
OMR::Z::RegisterDependencyConditions::bookKeepingRegisterUses(TR::Instruction * instr, TR::CodeGenerator * cg, int32_t oldPreCursor, int32_t oldPostCursor)
   {
   if (instr->getOpCodeValue() != TR::InstOpCode::ASSOCREGS)
      {
      // We create a register association directive for each dependency

      if (cg->enableRegisterAssociations() && !cg->isOutOfLineColdPath())
         {
         createRegisterAssociationDirective(instr, cg);
         }

      if (_preConditions != NULL)
         {
         for (int32_t i = oldPreCursor; i < _addCursorForPre; i++)
            {
            instr->useRegister(_preConditions->getRegisterDependency(i)->getRegister(), true);
            }
         }

      if (_postConditions != NULL)
         {
         for (int32_t j = oldPostCursor; j < _addCursorForPost; j++)
            {
            instr->useRegister(_postConditions->getRegisterDependency(j)->getRegister(), true);
            }
         }
      }
   }


/**
 * Register association
 */
void
OMR::Z::RegisterDependencyConditions::createRegisterAssociationDirective(TR::Instruction * instruction, TR::CodeGenerator * cg)
   {
   TR::Machine *machine = cg->machine();

   machine->createRegisterAssociationDirective(instruction->getPrev());

   // Now set up the new register associations as required by the current
   // dependent register instruction onto the machine.
   // Only the registers that this instruction interferes with are modified.
   //
   TR::RegisterDependencyGroup * depGroup = getPreConditions();
   for (int32_t j = 0; j < getNumPreConditions(); j++)
      {
      TR::RegisterDependency * dependency = depGroup->getRegisterDependency(j);
      if (dependency->getRegister())
         {
         machine->setVirtualAssociatedWithReal(dependency->getRealRegister(), dependency->getRegister());
         }
      }

   depGroup = getPostConditions();
   for (int32_t k = 0; k < getNumPostConditions(); k++)
      {
      TR::RegisterDependency * dependency = depGroup->getRegisterDependency(k);
      if (dependency->getRegister())
         {
         machine->setVirtualAssociatedWithReal(dependency->getRealRegister(), dependency->getRegister());
         }
      }
   }

bool OMR::Z::RegisterDependencyConditions::doesConditionExist( TR::RegisterDependencyGroup * regDepArr, TR::RegisterDependency *depToCheck, uint32_t numberOfRegisters )
   {
   uint32_t flag = depToCheck->getFlags();
   TR::Register *vr = depToCheck->getRegister();
   TR::RealRegister::RegNum rr = depToCheck->getRealRegister();

   for( int32_t i = 0; i < numberOfRegisters; i++ )
      {
      TR::RegisterDependency * regDep = regDepArr->getRegisterDependency(i);

      if( ( regDep->getRegister() == vr ) && ( regDep->getFlags() == flag ) )
         {
         // If all properties of these two dependencies are the same, then it exists
         // already in the dependency group.
         if( regDep->getRealRegister() == rr )
            {
            return true;
            }

         // Post-condition number: Virtual register pointer, RegNum number, condition flags
         // 0: 4AE60068, 71(AssignAny), 00000003
         // 1: 4AE5F04C, 71(AssignAny), 00000003
         // 2: 4AE60158, 15(GPR14), 00000003
         //
         // merged with:
         //
         // 0: 4AE60068, 1(GPR0), 00000003
         //
         // Produces an assertion. The fix for this is to overwrite the AssignAny dependency.
         //
         else if( regDep->isAssignAny() )
            {
            // AssignAny exists already, and the argument real register is not AssignAny
            // - overwrite the existing real register with the argument real register
            regDep->setRealRegister( rr );
            return true;
            }
         else if( depToCheck->isAssignAny() )
            {
            // The existing register dependency is not AssignAny but the argument real register is
            // - simply return true
            return true;
            }
         }
      }

   return false;
   }

bool OMR::Z::RegisterDependencyConditions::doesPreConditionExist( TR::RegisterDependency *depToCheck )
   {
   return doesConditionExist( _preConditions, depToCheck, _addCursorForPre );
   }

bool OMR::Z::RegisterDependencyConditions::doesPostConditionExist( TR::RegisterDependency *depToCheck )
   {
   return doesConditionExist( _postConditions, depToCheck, _addCursorForPost );
   }

bool OMR::Z::RegisterDependencyConditions::addPreConditionIfNotAlreadyInserted(TR::RegisterDependency *regDep)
   {
   uint32_t flag = regDep->getFlags();
   TR::Register* vr = regDep->getRegister();
   TR::RealRegister::RegNum rr = regDep->getRealRegister();

   return addPreConditionIfNotAlreadyInserted(vr, static_cast<TR::RealRegister::RegDep>(rr), flag);
   }

bool OMR::Z::RegisterDependencyConditions::addPreConditionIfNotAlreadyInserted(TR::Register *vr,
                                                                                  TR::RealRegister::RegNum rr,
                                                                                  uint8_t flag)
   {
   return addPreConditionIfNotAlreadyInserted(vr, static_cast<TR::RealRegister::RegDep>(rr), flag);
   }

/**
 * Checks for an existing pre-condition for given virtual register.  If found,
 * the flags are updated.  If not found, a new pre-condition is created with
 * the given virtual register and associated real register / property.
 *
 * @param vr    The virtual register of the pre-condition.
 * @param rr    The real register or real register property of the pre-condition.
 * @param flags Flags to be updated with associated pre-condition.
 * @sa addPostConditionIfNotAlreadyInserted()
 */
bool OMR::Z::RegisterDependencyConditions::addPreConditionIfNotAlreadyInserted(TR::Register *vr,
                                                                                  TR::RealRegister::RegDep rr,
                                                                                  uint8_t flag)
   {
   int32_t pos = -1;
   if (_preConditions != NULL && (pos = _preConditions->searchForRegisterPos(vr, RefsAndDefsDependentRegister, _addCursorForPre, _cg)) < 0)
      {
      if (!(vr->getRealRegister() && vr->getRealRegister()->getState() == TR::RealRegister::Locked))
         addPreCondition(vr, rr, flag);
      return true;
      }
   else if (pos >= 0 && _preConditions->getRegisterDependency(pos)->getFlags() != flag)
      {
      _preConditions->getRegisterDependency(pos)->setFlags(flag);
      return false;
      }
   return false;
   }


bool OMR::Z::RegisterDependencyConditions::addPostConditionIfNotAlreadyInserted(TR::RegisterDependency *regDep)
   {
   uint32_t flag = regDep->getFlags();
   TR::Register* vr = regDep->getRegister();
   TR::RealRegister::RegNum rr = regDep->getRealRegister();

   return addPostConditionIfNotAlreadyInserted(vr, static_cast<TR::RealRegister::RegDep>(rr), flag);
   }

bool OMR::Z::RegisterDependencyConditions::addPostConditionIfNotAlreadyInserted(TR::Register *vr,
                                                                                   TR::RealRegister::RegNum rr,
                                                                                   uint8_t flag)
   {
   return addPostConditionIfNotAlreadyInserted(vr, static_cast<TR::RealRegister::RegDep>(rr), flag);
   }

/**
 * Checks for an existing post-condition for given virtual register.  If found,
 * the flags are updated.  If not found, a new post condition is created with
 * the given virtual register and associated real register / property.
 *
 * @param vr    The virtual register of the post-condition.
 * @param rr    The real register or real register property of the post-condition.
 * @param flags Flags to be updated with associated post-condition.
 * @sa addPreConditionIfNotAlreadyInserted()
 */
bool OMR::Z::RegisterDependencyConditions::addPostConditionIfNotAlreadyInserted(TR::Register *vr,
                                                                                   TR::RealRegister::RegDep rr,
                                                                                   uint8_t flag)
   {
   int32_t pos = -1;
   if (_postConditions != NULL && (pos = _postConditions->searchForRegisterPos(vr, RefsAndDefsDependentRegister, _addCursorForPost, _cg)) < 0)
      {
      if (!(vr->getRealRegister() && vr->getRealRegister()->getState() == TR::RealRegister::Locked))
         addPostCondition(vr, rr, flag);
      return true;
      }
   else if (pos >= 0 && _postConditions->getRegisterDependency(pos)->getFlags() != flag)
      {
      _postConditions->getRegisterDependency(pos)->setFlags(flag);
      return false;
      }
   return false;
   }
