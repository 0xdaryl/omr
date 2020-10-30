/*******************************************************************************
 * Copyright (c) 2018, 2020 IBM Corp. and others
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

#include <stddef.h>
#include "codegen/ARM64Instruction.hpp"
#include "codegen/BackingStore.hpp"
#include "codegen/CodeGenerator.hpp"
#include "codegen/GenerateInstructions.hpp"
#include "codegen/Instruction.hpp"
#include "codegen/MemoryReference.hpp"
#include "codegen/RealRegister.hpp"
#include "codegen/RegisterDependency.hpp"
#include "codegen/RegisterDependencyGroup.hpp"
#include "il/Node.hpp"
#include "il/Node_inlines.hpp"

// Replace this by #include "codegen/Instruction.hpp" when available for aarch64
namespace TR { class Instruction; }

OMR::ARM64::RegisterDependencyConditions::RegisterDependencyConditions(
                                       TR::CodeGenerator *cg,
                                       TR::Node          *node,
                                       uint32_t          extranum,
                                       TR::Instruction  **cursorPtr)
   {
   List<TR::Register>  regList(cg->trMemory());
   TR::Instruction    *iCursor = (cursorPtr==NULL)?NULL:*cursorPtr;
   int32_t totalNum = node->getNumChildren() + extranum;
   int32_t i;

   cg->comp()->incVisitCount();

   _preConditions = TR::RegisterDependencyGroup::create(totalNum, cg->trMemory());
   _postConditions = TR::RegisterDependencyGroup::create(totalNum, cg->trMemory());
   _numPreConditions = totalNum;
   _addCursorForPre = 0;
   _numPostConditions = totalNum;
   _addCursorForPost = 0;

   // First, handle dependencies that match current association
   for (i = 0; i < node->getNumChildren(); i++)
      {
      TR::Node *child = node->getChild(i);
      TR::Register *reg = child->getRegister();
      TR::RealRegister::RegNum regNum = (TR::RealRegister::RegNum)cg->getGlobalRegister(child->getGlobalRegisterNumber());

      if (reg->getAssociation() != regNum)
         {
         continue;
         }

      addPreCondition(reg, regNum);
      addPostCondition(reg, regNum);
      regList.add(reg);
      }

   // Second pass to handle dependencies for which association does not exist
   // or does not match
   for (i = 0; i < node->getNumChildren(); i++)
      {
      TR::Node *child = node->getChild(i);
      TR::Register *reg = child->getRegister();
      TR::Register *copyReg = NULL;
      TR::RealRegister::RegNum regNum = (TR::RealRegister::RegNum)cg->getGlobalRegister(child->getGlobalRegisterNumber());

      if (reg->getAssociation() == regNum)
         {
         continue;
         }

      if (regList.find(reg))
         {
         TR_RegisterKinds kind = reg->getKind();

         TR_ASSERT_FATAL((kind == TR_GPR) || (kind == TR_FPR), "Invalid register kind.");

         if (kind == TR_GPR)
            {
            bool containsInternalPointer = reg->getPinningArrayPointer();
            copyReg = (reg->containsCollectedReference() && !containsInternalPointer) ?
                        cg->allocateCollectedReferenceRegister() : cg->allocateRegister();
            if (containsInternalPointer)
               {
               copyReg->setContainsInternalPointer();
               copyReg->setPinningArrayPointer(reg->getPinningArrayPointer());
               }
            iCursor = generateMovInstruction(cg, node, copyReg, reg, true, iCursor);
            }
         else
            {
            bool isSinglePrecision = reg->isSinglePrecision();
            copyReg = isSinglePrecision ? cg->allocateSinglePrecisionRegister() : cg->allocateRegister(TR_FPR);
            iCursor = generateTrg1Src1Instruction(cg, isSinglePrecision ? TR::InstOpCode::fmovs : TR::InstOpCode::fmovd, node, copyReg, reg, iCursor);
            }

         reg = copyReg;
         }

      addPreCondition(reg, regNum);
      addPostCondition(reg, regNum);
      if (copyReg != NULL)
         {
         cg->stopUsingRegister(copyReg);
         }
      else
         {
         regList.add(reg);
         }
      }

   if (iCursor != NULL && cursorPtr != NULL)
      {
      *cursorPtr = iCursor;
      }
   }

void OMR::ARM64::RegisterDependencyConditions::unionNoRegPostCondition(TR::Register *reg, TR::CodeGenerator *cg)
   {
   addPostCondition(reg, TR::RealRegister::NoReg);
   }

bool OMR::ARM64::RegisterDependencyConditions::refsRegister(TR::Register *r)
   {
   for (uint16_t i = 0; i < _addCursorForPre; i++)
      {
      if (_preConditions->getRegisterDependency(i)->getRegister() == r &&
          _preConditions->getRegisterDependency(i)->getRefsRegister())
         {
         return true;
         }
      }
   for (uint16_t j = 0; j < _addCursorForPost; j++)
      {
      if (_postConditions->getRegisterDependency(j)->getRegister() == r &&
          _postConditions->getRegisterDependency(j)->getRefsRegister())
         {
         return true;
         }
      }
   return false;
   }

bool OMR::ARM64::RegisterDependencyConditions::defsRegister(TR::Register *r)
   {
   for (uint16_t i = 0; i < _addCursorForPre; i++)
      {
      if (_preConditions->getRegisterDependency(i)->getRegister() == r &&
          _preConditions->getRegisterDependency(i)->getDefsRegister())
         {
         return true;
         }
      }
   for (uint16_t j = 0; j < _addCursorForPost; j++)
      {
      if (_postConditions->getRegisterDependency(j)->getRegister() == r &&
          _postConditions->getRegisterDependency(j)->getDefsRegister())
         {
         return true;
         }
      }
   return false;
   }

bool OMR::ARM64::RegisterDependencyConditions::defsRealRegister(TR::Register *r)
   {
   for (uint16_t i = 0; i < _addCursorForPre; i++)
      {
      if (_preConditions->getRegisterDependency(i)->getRegister()->getAssignedRegister() == r &&
          _preConditions->getRegisterDependency(i)->getDefsRegister())
         {
         return true;
         }
      }
   for (uint16_t j = 0; j < _addCursorForPost; j++)
      {
      if (_postConditions->getRegisterDependency(j)->getRegister()->getAssignedRegister() == r &&
          _postConditions->getRegisterDependency(j)->getDefsRegister())
         {
         return true;
         }
      }
   return false;
   }

bool OMR::ARM64::RegisterDependencyConditions::usesRegister(TR::Register *r)
   {
   for (uint16_t i = 0; i < _addCursorForPre; i++)
      {
      if (_preConditions->getRegisterDependency(i)->getRegister() == r &&
          _preConditions->getRegisterDependency(i)->getUsesRegister())
         {
         return true;
         }
      }
   for (uint16_t j = 0; j < _addCursorForPost; j++)
      {
      if (_postConditions->getRegisterDependency(j)->getRegister() == r &&
          _postConditions->getRegisterDependency(j)->getUsesRegister())
         {
         return true;
         }
      }
   return false;
   }

void OMR::ARM64::RegisterDependencyConditions::bookKeepingRegisterUses(TR::Instruction *instr, TR::CodeGenerator *cg)
   {
   if (instr->getOpCodeValue() == TR::InstOpCode::assocreg)
      return;

   // We create a register association directive for each dependency
   bool isOOL = cg->isOutOfLineColdPath();
   if (cg->enableRegisterAssociations() && !isOOL)
      {
      TR::Machine *machine = cg->machine();

      machine->createRegisterAssociationDirective(instr->getPrev());

      // Now set up the new register associations as required by the current
      // dependent register instruction onto the machine.
      // Only the registers that this instruction interferes with are modified.
      //
      for (int i = 0; i < _addCursorForPre; i++)
         {
         TR::RegisterDependency *dependency = _preConditions->getRegisterDependency(i);
         if (dependency->getRegister())
            {
            machine->setVirtualAssociatedWithReal(dependency->getRealRegister(), dependency->getRegister());
            }
         }

      for (int j = 0; j < _addCursorForPost; j++)
         {
         TR::RegisterDependency *dependency = _postConditions->getRegisterDependency(j);
         if (dependency->getRegister())
            {
            machine->setVirtualAssociatedWithReal(dependency->getRealRegister(), dependency->getRegister());
            }
         }
      }

   for (int i = 0; i < _addCursorForPre; i++)
      {
      auto dependency = _preConditions->getRegisterDependency(i);
      TR::Register *virtReg = dependency->getRegister();
      TR::RealRegister::RegNum regNum = dependency->getRealRegister();
      instr->useRegister(virtReg, true);
      if (!isOOL)
         cg->setRealRegisterAssociation(virtReg, regNum);
      }
   for (int j = 0; j < _addCursorForPost; j++)
      {
      auto dependency = _postConditions->getRegisterDependency(j);
      TR::Register *virtReg = dependency->getRegister();
      TR::RealRegister::RegNum regNum = dependency->getRealRegister();
      instr->useRegister(virtReg, true);
      if (!isOOL)
         cg->setRealRegisterAssociation(virtReg, regNum);
      }
   }

TR::RegisterDependencyConditions *
OMR::ARM64::RegisterDependencyConditions::clone(
   TR::CodeGenerator *cg,
   TR::RegisterDependencyConditions *added,
   bool omitPre, bool omitPost)
   {
   int preNum = omitPre ? 0 : getAddCursorForPre();
   int postNum = omitPost ? 0 : getAddCursorForPost();
   int addPreNum = 0;
   int addPostNum = 0;

   if (added != NULL)
      {
      addPreNum = omitPre ? 0 : added->getAddCursorForPre();
      addPostNum = omitPost ? 0 : added->getAddCursorForPost();
      }

   TR::RegisterDependencyConditions *result =  new (cg->trHeapMemory()) TR::RegisterDependencyConditions(preNum + addPreNum, postNum + addPostNum, cg->trMemory());
   for (int i = 0; i < preNum; i++)
      {
      auto singlePair = getPreConditions()->getRegisterDependency(i);
      result->addPreCondition(singlePair->getRegister(), singlePair->getRealRegister(), singlePair->getFlags());
      }

   for (int i = 0; i < postNum; i++)
      {
      auto singlePair = getPostConditions()->getRegisterDependency(i);
      result->addPostCondition(singlePair->getRegister(), singlePair->getRealRegister(), singlePair->getFlags());
      }

   for (int i = 0; i < addPreNum; i++)
      {
      auto singlePair = added->getPreConditions()->getRegisterDependency(i);
      result->addPreCondition(singlePair->getRegister(), singlePair->getRealRegister(), singlePair->getFlags());
      }

   for (int i = 0; i < addPostNum; i++)
      {
      auto singlePair = added->getPostConditions()->getRegisterDependency(i);
      result->addPostCondition(singlePair->getRegister(), singlePair->getRealRegister(), singlePair->getFlags());
      }

   return result;
   }

void OMR::ARM64::RegisterDependencyConditions::stopUsingDepRegs(TR::CodeGenerator *cg, TR::Register *returnRegister)
   {
   if (_preConditions != NULL)
      _preConditions->stopUsingDepRegs(getAddCursorForPre(), returnRegister, cg);
   if (_postConditions != NULL)
      _postConditions->stopUsingDepRegs(getAddCursorForPost(), returnRegister, cg);
   }

