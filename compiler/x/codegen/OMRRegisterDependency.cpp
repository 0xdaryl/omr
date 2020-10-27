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

#include <algorithm>
#include <stdint.h>
#include <string.h>
#include "codegen/BackingStore.hpp"
#include "codegen/CodeGenerator.hpp"
#include "codegen/Instruction.hpp"
#include "codegen/LiveRegister.hpp"
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
#include "env/TRMemory.hpp"
#include "il/Block.hpp"
#include "il/DataTypes.hpp"
#include "il/ILOpCodes.hpp"
#include "il/Node.hpp"
#include "il/Node_inlines.hpp"
#include "infra/Assert.hpp"
#include "infra/List.hpp"
#include "ras/Debug.hpp"
#include "ras/DebugCounter.hpp"
#include "x/codegen/X86Instruction.hpp"
#include "x/codegen/X86Ops.hpp"
#include "x/codegen/X86Register.hpp"

////////////////////////
// Hack markers
//

// TODO:AMD64: IA32 FPR GRA currently interferes XMM GRA in a somewhat
// confusing way that doesn't occur on AMD64.
#define XMM_GPRS_USE_DISTINCT_NUMBERS(cg) (cg->comp()->target().is64Bit())

static void generateRegcopyDebugCounter(TR::CodeGenerator *cg, const char *category)
   {
   if (!cg->comp()->getOptions()->enableDebugCounters())
      return;
   TR::TreeTop *tt = cg->getCurrentEvaluationTreeTop();
   const char *fullName = TR::DebugCounter::debugCounterName(cg->comp(),
      "regcopy/cg.genRegDepConditions/%s/(%s)/%s/block_%d",
      category,
      cg->comp()->signature(),
      cg->comp()->getHotnessName(cg->comp()->getMethodHotness()),
      tt->getEnclosingBlock()->getNumber());
   TR::Instruction *cursor = cg->lastInstructionBeforeCurrentEvaluationTreeTop();
   cg->generateDebugCounter(cursor, fullName, 1, TR::DebugCounter::Undetermined);
   }

OMR::X86::RegisterDependencyConditions::RegisterDependencyConditions(
      TR::Node *node,
      TR::CodeGenerator *cg,
      uint32_t additionalRegDeps,
      List<TR::Register> *popRegisters)
   :_numPreConditions(-1),_numPostConditions(-1),
    _addCursorForPre(0),_addCursorForPost(0)
   {
   TR::Register      *copyReg = NULL;
   TR::Register      *highCopyReg = NULL;
   List<TR::Register> registers(cg->trMemory());
   uint32_t          numFPGlobalRegs = 0,
                     totalNumFPGlobalRegs = 0;
   int32_t           i;
   TR::Machine *machine = cg->machine();
   TR::Compilation *comp = cg->comp();

   int32_t numLongs = 0;

   // Pre-compute how many x87 FP stack slots we need to accommodate all the FP global registers.
   //
   for (i = 0; i < node->getNumChildren(); ++i)
      {
      TR::Node     *child = node->getChild(i);
      TR::Register *reg   = child->getRegister();

      if (reg->getKind() == TR_GPR)
         {
         if (child->getHighGlobalRegisterNumber() > -1)
            numLongs++;
         }
      else if (reg->getKind() == TR_X87)
         {
         totalNumFPGlobalRegs++;
         }
      }

   _preConditions = TR::RegisterDependencyGroup::create(node->getNumChildren() + additionalRegDeps + numLongs, cg->trMemory());
   _postConditions = TR::RegisterDependencyGroup::create(node->getNumChildren() + additionalRegDeps + numLongs, cg->trMemory());
   _numPreConditions = node->getNumChildren() + additionalRegDeps + numLongs;
   _numPostConditions = node->getNumChildren() + additionalRegDeps + numLongs;

   int32_t numLongsAdded = 0;

   for (i = node->getNumChildren()-1; i >= 0; i--)
      {
      TR::Node                 *child        = node->getChild(i);
      TR::Register             *globalReg    = child->getRegister();
      TR::Register             *highGlobalReg = NULL;
      TR_GlobalRegisterNumber  globalRegNum = child->getGlobalRegisterNumber();
      TR_GlobalRegisterNumber  highGlobalRegNum = child->getHighGlobalRegisterNumber();

      TR::RealRegister::RegNum realRegNum = TR::RealRegister::NoReg, realHighRegNum = TR::RealRegister::NoReg;
      if (globalReg->getKind() == TR_GPR || globalReg->getKind() == TR_VRF || XMM_GPRS_USE_DISTINCT_NUMBERS(cg))
         {
         realRegNum = (TR::RealRegister::RegNum) cg->getGlobalRegister(globalRegNum);

         if (highGlobalRegNum > -1)
            realHighRegNum = (TR::RealRegister::RegNum)
                             cg->getGlobalRegister(highGlobalRegNum);
         }
      else if (globalReg->getKind() == TR_FPR)
         {
         // Convert the global register number from an st0-based value to an xmm0-based one.
         //
         realRegNum = (TR::RealRegister::RegNum)
                         (cg->getGlobalRegister(globalRegNum)
                            - TR::RealRegister::FirstFPR
                            + TR::RealRegister::FirstXMMR);

         // Find the global register that has been allocated in this XMMR, if any.
         //
         TR::Register *xmmGlobalReg = cg->machine()->getXMMGlobalRegister(realRegNum - TR::RealRegister::FirstXMMR);
         if (xmmGlobalReg)
            globalReg = xmmGlobalReg;
         }
      else
         {
         TR_ASSERT(globalReg->getKind() == TR_X87, "invalid global register kind\n");

         // Compute the real global register number based on the number of global FPRs allocated so far.
         //
         realRegNum = (TR::RealRegister::RegNum)
                         (TR::RealRegister::FirstFPR + (totalNumFPGlobalRegs - numFPGlobalRegs - 1));

         // Find the global register that has been allocated in this FP stack slot, if any.
         //
         int32_t fpStackSlot = child->getGlobalRegisterNumber() - machine->getNumGlobalGPRs();
         TR::Register *fpGlobalReg = cg->machine()->getFPStackRegister(fpStackSlot);
         if (fpGlobalReg)
            globalReg = fpGlobalReg;

         numFPGlobalRegs++;
         }

      if (registers.find(globalReg))
         {
         if (globalReg->getKind() == TR_GPR)
            {
            bool containsInternalPointer = false;
            TR::RegisterPair *globalRegPair = globalReg->getRegisterPair();
            if (globalRegPair)
               {
               highGlobalReg = globalRegPair->getHighOrder();
               globalReg = globalRegPair->getLowOrder();
               }
            else if (child->getHighGlobalRegisterNumber() > -1)
               TR_ASSERT(0, "Long child does not have a register pair\n");

            if (globalReg->getPinningArrayPointer())
               containsInternalPointer = true;

            copyReg = (globalReg->containsCollectedReference() && !containsInternalPointer) ?
                      cg->allocateCollectedReferenceRegister() : cg->allocateRegister();

            if (containsInternalPointer)
               {
               copyReg->setContainsInternalPointer();
               copyReg->setPinningArrayPointer(globalReg->getPinningArrayPointer());
               }

            TR::Instruction *prevInstr = cg->getAppendInstruction();
            TR::Instruction *placeToAdd = prevInstr;
            if (prevInstr && prevInstr->getOpCode().isFusableCompare())
               {
               TR::Instruction *prevPrevInstr = prevInstr->getPrev();
               if (prevPrevInstr)
                  {
                  if (comp->getOption(TR_TraceCG))
                     traceMsg(comp, "Moving reg reg copy earlier (after %p) in %s\n", prevPrevInstr, comp->signature());
                  placeToAdd = prevPrevInstr;
                  }
               }

            generateRegRegInstruction(placeToAdd, MOVRegReg(), copyReg, globalReg, cg);

            if (highGlobalReg)
               {
               generateRegcopyDebugCounter(cg, "gpr-pair");
               containsInternalPointer = false;
               if (highGlobalReg->getPinningArrayPointer())
                  containsInternalPointer = true;

               highCopyReg = (highGlobalReg->containsCollectedReference() && !containsInternalPointer) ?
                             cg->allocateCollectedReferenceRegister() : cg->allocateRegister();

               if (containsInternalPointer)
                  {
                  highCopyReg->setContainsInternalPointer();
                  highCopyReg->setPinningArrayPointer(highGlobalReg->getPinningArrayPointer());
                  }

               generateRegRegInstruction(MOV4RegReg, node, highCopyReg, highGlobalReg, cg);
               }
            else
               {
               generateRegcopyDebugCounter(cg, "gpr");
               highCopyReg = NULL;
               }
            }
         else if (globalReg->getKind() == TR_FPR)
            {
            generateRegcopyDebugCounter(cg, "fpr");
            if (globalReg->isSinglePrecision())
               {
               copyReg = cg->allocateSinglePrecisionRegister(TR_FPR);
               generateRegRegInstruction(MOVAPSRegReg, node, copyReg, child->getRegister(), cg);
               }
            else
               {
               copyReg = cg->allocateRegister(TR_FPR);
               generateRegRegInstruction(MOVAPDRegReg, node, copyReg, child->getRegister(), cg);
               }
            }
         else if (globalReg->getKind() == TR_VRF)
            {
            generateRegcopyDebugCounter(cg, "vrf");
            copyReg = cg->allocateRegister(TR_VRF);
            generateRegRegInstruction(MOVDQURegReg, node, copyReg, child->getRegister(), cg);
            }
         else
            {
            generateRegcopyDebugCounter(cg, "x87");
            if (globalReg->isSinglePrecision())
               {
               copyReg = cg->allocateSinglePrecisionRegister(TR_X87);
               }
            else
               {
               copyReg = cg->allocateRegister(TR_X87);
               }

            generateFPST0STiRegRegInstruction(FLDRegReg, node, copyReg, child->getRegister(), cg);

            int32_t fpRegNum = child->getGlobalRegisterNumber() - machine->getNumGlobalGPRs();
            //dumpOptDetails("FP reg num %d global reg num %d global reg %s\n", fpRegNum, child->getGlobalRegisterNumber(), getDebug()->getName(globalReg));
            cg->machine()->setCopiedFPStackRegister(fpRegNum, toX86FPStackRegister(globalReg));
            if (child->getOpCodeValue() == TR::PassThrough)
               {
               TR::Node *fpChild = child->getFirstChild();
               cg->machine()->setFPStackRegisterNode(fpRegNum, fpChild);
               }
            else
               cg->machine()->setFPStackRegisterNode(fpRegNum, child);

            int32_t i;
            for (i=0;i<TR_X86FPStackRegister::NumRegisters;i++)
               {
               TR::Register *reg = cg->machine()->getFPStackRegister(i);
               if ((reg == globalReg) &&
                   (i != fpRegNum))
                  {
                    //dumpOptDetails("FP reg num %d global reg num %d global reg %s\n", i, child->getGlobalRegisterNumber(), getDebug()->getName(copyReg));
                  cg->machine()->setCopiedFPStackRegister(i, toX86FPStackRegister(copyReg));
                  if (child->getOpCodeValue() == TR::PassThrough)
                     {
                     TR::Node *fpChild = child->getFirstChild();
                     cg->machine()->setFPStackRegisterNode(i, fpChild);
                     }
                  else
                     cg->machine()->setFPStackRegisterNode(i, child);

                  break;
                  }
               }

            }

         globalReg = copyReg;
         highGlobalReg = highCopyReg;
         }
      else
         {
         // Change the register of the floating-point child of a PassThrough,
         // as we must have a register that is live as the global register
         // corresponding to the child.
         //
         if (child->getOpCodeValue() == TR::PassThrough && globalReg->getKind() == TR_X87)
            {
            TR::Node *fpChild = child->getFirstChild();

            if ((fpChild->getDataType() == TR::Float) || (fpChild->getDataType() == TR::Double))
               {

               // if the child's register is live on another stack slot, dont change the register.
               TR::Register *fpGlobalReg = NULL;
               for (int j = 0 ; j < TR_X86FPStackRegister::NumRegisters && fpGlobalReg != child->getRegister(); j++)
                  {
                  fpGlobalReg = cg->machine()->getFPStackRegister(j);
                  }

               if (fpGlobalReg != child->getRegister())
                  {
                  fpChild->setRegister(globalReg);
                  machine->setFPStackRegister(child->getGlobalRegisterNumber() - machine->getNumGlobalGPRs(), toX86FPStackRegister(globalReg));
                  }
               }
            }

         registers.add(globalReg);
         TR::RegisterPair *globalRegPair = globalReg->getRegisterPair();
         if (globalRegPair)
            {
            highGlobalReg = globalRegPair->getHighOrder();
            globalReg = globalRegPair->getLowOrder();
            }
         else if (child->getHighGlobalRegisterNumber() > -1)
            TR_ASSERT(0, "Long child does not have a register pair\n");
         }

      if (globalReg->getKind() == TR_GPR)
         {
         addPreCondition(globalReg, realRegNum, cg);
         addPostCondition(globalReg, realRegNum, cg);
         if (highGlobalReg)
            {
            numLongsAdded++;
            addPreCondition(highGlobalReg, realHighRegNum, cg);
            addPostCondition(highGlobalReg, realHighRegNum, cg);
            }
         }
      else if (globalReg->getKind() == TR_FPR || globalReg->getKind() == TR_VRF)
         {
         addPreCondition(globalReg, realRegNum, cg);
         addPostCondition(globalReg, realRegNum, cg);
         cg->machine()->setXMMGlobalRegister(realRegNum - TR::RealRegister::FirstXMMR, globalReg);
         }
      else if (globalReg->getKind() == TR_X87)
         {
         addPreCondition(globalReg, realRegNum, cg, UsesGlobalDependentFPRegister);
         addPostCondition(globalReg, realRegNum, cg, UsesGlobalDependentFPRegister);
         int32_t fpRegNum = child->getGlobalRegisterNumber() - machine->getNumGlobalGPRs();
         cg->machine()->setFPStackRegister(fpRegNum, toX86FPStackRegister(globalReg));
         }

      // If the register dependency isn't actually used
      // (dead store elimination probably removed it)
      // and this is a float then we have to pop it off the stack.
      //
      if (child->getOpCodeValue() == TR::PassThrough)
         {
         child = child->getFirstChild();
         }

      if (popRegisters &&
          globalReg->getKind() == TR_X87 &&
          child->getReferenceCount() == 0 &&
          (child->getDataType() == TR::Float || child->getDataType() == TR::Double))
         {
         popRegisters->add(globalReg);
         }
      else if (copyReg)
         {
         cg->stopUsingRegister(copyReg);
         if (highCopyReg)
            cg->stopUsingRegister(highCopyReg);
         }
      }

   if (numLongs != numLongsAdded)
     TR_ASSERT(0, "Mismatch numLongs %d numLongsAdded %d\n", numLongs, numLongsAdded);
   }


uint32_t OMR::X86::RegisterDependencyConditions::unionDependencies(
   TR::RegisterDependencyGroup *deps,
   uint32_t                     cursor,
   TR::Register                *vr,
   TR::RealRegister::RegNum     rr,
   TR::CodeGenerator           *cg,
   uint8_t                      flag,
   bool                         isAssocRegDependency)
   {
   TR::Machine *machine = cg->machine();

   // If vr is already in deps, combine the existing association with rr.
   //
   if (vr)
      {
      if (vr->getRealRegister())
         {
         TR::RealRegister::RegNum vrIndex = toRealRegister(vr)->getRegisterNumber();
         switch (vrIndex)
            {
            case TR::RealRegister::esp:
            case TR::RealRegister::vfp:
               // esp and vfp are ok, and are ignorable, since local RA already
               // deals with those without help from regdeps.
               //
               //
               TR_ASSERT(rr == vrIndex || rr == TR::RealRegister::NoReg, "esp and vfp can be assigned only to themselves or NoReg, not %s", cg->getDebug()->getRealRegisterName(rr));
               break;
            default:
               TR_ASSERT(0, "Unexpected real register %s in regdep", cg->getDebug()->getName(vr, TR_UnknownSizeReg));
               break;
            }
         return cursor;
         }

      for (uint32_t candidate = 0; candidate < cursor; candidate++)
         {
         TR::RegisterDependency  *dep = deps->getRegisterDependency(candidate);
         if (dep->getRegister() == vr)
            {
            // Keep the stronger of the two constraints
            //
            TR::RealRegister::RegNum min = std::min(rr, dep->getRealRegister());
            TR::RealRegister::RegNum max = std::max(rr, dep->getRealRegister());
            if (min == TR::RealRegister::NoReg)
               {
               // Anything is stronger than NoReg
               deps->setDependencyInfo(candidate, vr, max, cg, flag, isAssocRegDependency);
               }
            else if(max == TR::RealRegister::ByteReg)
               {
               // Specific regs are stronger than ByteReg
               TR_ASSERT(min <= TR::RealRegister::Last8BitGPR, "Only byte registers are compatible with ByteReg dependency");
               deps->setDependencyInfo(candidate, vr, min, cg, flag, isAssocRegDependency);
               }
            else
               {
               // This assume fires for some assocRegs instructions
               //TR_ASSERT(min == max, "Specific register dependency is only compatible with itself");
               if (min != max)
                  continue;

               // Nothing to do
               }

            return cursor;
            }
         }
      }

   // vr is not already in deps, so add a new dep
   //
   deps->setDependencyInfo(cursor++, vr, rr, cg, flag, isAssocRegDependency);
   return cursor;
   }

void OMR::X86::RegisterDependencyConditions::unionNoRegPostCondition(TR::Register *reg, TR::CodeGenerator *cg)
   {
   unionPostCondition(reg, TR::RealRegister::NoReg, cg);
   }


uint32_t OMR::X86::RegisterDependencyConditions::unionRealDependencies(
   TR::RegisterDependencyGroup *deps,
   uint32_t                     cursor,
   TR::Register                *vr,
   TR::RealRegister::RegNum     rr,
   TR::CodeGenerator           *cg,
   uint8_t                      flag,
   bool                         isAssocRegDependency)
   {
   TR::Machine *machine = cg->machine();
   // A vmThread/ebp dependency can be ousted by any other ebp dependency.
   // This situation should only occur when ebp is assigned as a global register.
   static TR::RealRegister::RegNum vmThreadRealRegisterIndex = TR::RealRegister::ebp;
   if (rr == vmThreadRealRegisterIndex)
      {
      depsize_t candidate;
      TR::Register *vmThreadRegister = cg->getVMThreadRegister();
      for (candidate = 0; candidate < cursor; candidate++)
         {
         TR::RegisterDependency  *dep = deps->getRegisterDependency(candidate);
         // Check for a pre-existing ebp dependency.
         if (dep->getRealRegister() == vmThreadRealRegisterIndex)
            {
            // Any ebp dependency is stronger than a vmThread/ebp dependency.
            // Oust any vmThread dependency.
            if (dep->getRegister() == vmThreadRegister)
               {
               //diagnostic("\nEnvicting virt reg %s dep for %s replaced with virt reg %s\n      {\"%s\"}",
               //   getDebug()->getName(dep->getRegister()),getDebug()->getName(machine->getRealRegister(rr)),getDebug()->getName(vr), cg->comp()->getCurrentMethod()->signature());
               deps->setDependencyInfo(candidate, vr, rr, cg, flag, isAssocRegDependency);
               }
            else
               {
               //diagnostic("\nSkipping virt reg %s dep for %s in favour of %s\n     {%s}}\n",
               //   getDebug()->getName(vr),getDebug()->getName(machine->getRealRegister(rr)),getDebug()->getName(dep->getRegister()), cg->comp()->getCurrentMethod()->signature());
               TR_ASSERT(vr == vmThreadRegister, "Conflicting EBP register dependencies.\n");
               }
            return cursor;
            }
         }
      }

   // Not handled above, so add a new dependency.
   //
   deps->setDependencyInfo(cursor++, vr, rr, cg, flag, isAssocRegDependency);
   return cursor;
   }


TR::RegisterDependencyConditions  *OMR::X86::RegisterDependencyConditions::clone(TR::CodeGenerator *cg, uint32_t additionalRegDeps)
   {
   TR::RegisterDependencyConditions  *other =
      new (cg->trHeapMemory()) TR::RegisterDependencyConditions(_numPreConditions  + additionalRegDeps,
                                              _numPostConditions + additionalRegDeps, cg->trMemory());
   int32_t i;

   for (i = _numPreConditions-1; i >= 0; --i)
      {
      TR::RegisterDependency  *dep = getPreConditions()->getRegisterDependency(i);
      other->getPreConditions()->setDependencyInfo(i, dep->getRegister(), dep->getRealRegister(), cg, dep->getFlags());
      }

   for (i = _numPostConditions-1; i >= 0; --i)
      {
      TR::RegisterDependency  *dep = getPostConditions()->getRegisterDependency(i);
      other->getPostConditions()->setDependencyInfo(i, dep->getRegister(), dep->getRealRegister(), cg, dep->getFlags());
      }

   other->setAddCursorForPre(_addCursorForPre);
   other->setAddCursorForPost(_addCursorForPost);
   return other;
   }


bool OMR::X86::RegisterDependencyConditions::refsRegister(TR::Register *r)
   {

   for (int i = 0; i < _numPreConditions; i++)
      {
      if (_preConditions->getRegisterDependency(i)->getRegister() == r &&
          _preConditions->getRegisterDependency(i)->getRefsRegister())
         {
         return true;
         }
      }

   for (int j = 0; j < _numPostConditions; j++)
      {
      if (_postConditions->getRegisterDependency(j)->getRegister() == r &&
          _postConditions->getRegisterDependency(j)->getRefsRegister())
         {
         return true;
         }
      }

   return false;
   }


bool OMR::X86::RegisterDependencyConditions::defsRegister(TR::Register *r)
   {

   for (int i = 0; i < _numPreConditions; i++)
      {
      if (_preConditions->getRegisterDependency(i)->getRegister() == r &&
          _preConditions->getRegisterDependency(i)->getDefsRegister())
         {
         return true;
         }
      }

   for (int j = 0; j < _numPostConditions; j++)
      {
      if (_postConditions->getRegisterDependency(j)->getRegister() == r &&
          _postConditions->getRegisterDependency(j)->getDefsRegister())
         {
         return true;
         }
      }

   return false;
   }


bool OMR::X86::RegisterDependencyConditions::usesRegister(TR::Register *r)
   {
   for (int i = 0; i < _numPreConditions; i++)
      {
      if (_preConditions->getRegisterDependency(i)->getRegister() == r &&
          _preConditions->getRegisterDependency(i)->getUsesRegister())
         {
         return true;
         }
      }

   for (int j = 0; j < _numPostConditions; j++)
      {
      if (_postConditions->getRegisterDependency(j)->getRegister() == r &&
          _postConditions->getRegisterDependency(j)->getUsesRegister())
         {
         return true;
         }
      }

   return false;
   }


void OMR::X86::RegisterDependencyConditions::useRegisters(TR::Instruction *instr, TR::CodeGenerator *cg)
   {
   int32_t i;

   for (i = 0; i < _numPreConditions; i++)
      {
      TR::Register *virtReg = _preConditions->getRegisterDependency(i)->getRegister();
      if (virtReg)
         {
         instr->useRegister(virtReg);
         }
      }

   for (i = 0; i < _numPostConditions; i++)
      {
      TR::Register *virtReg = _postConditions->getRegisterDependency(i)->getRegister();
      if (virtReg)
         {
         instr->useRegister(virtReg);
         }
      }
   }


void OMR::X86::RegisterDependencyConditions::createRegisterAssociationDirective(TR::Instruction *instruction, TR::CodeGenerator *cg)
   {

   TR::Machine *machine = cg->machine();

   machine->createRegisterAssociationDirective(instruction->getPrev());

   // Now set up the new register associations as required by the current
   // dependent register instruction onto the machine.
   // Only the registers that this instruction interferes with are modified.
   //
   TR::RegisterDependencyGroup *depGroup = getPreConditions();
   for (int j = 0; j < getNumPreConditions(); j++)
      {
      TR::RegisterDependency  *dependency = depGroup->getRegisterDependency(j);
      if (dependency->getRegister())
         machine->setVirtualAssociatedWithReal(dependency->getRealRegister(), dependency->getRegister());
      }

   depGroup = getPostConditions();
   for (int k = 0; k < getNumPostConditions(); k++)
      {
      TR::RegisterDependency  *dependency = depGroup->getRegisterDependency(k);
      if (dependency->getRegister())
         machine->setVirtualAssociatedWithReal(dependency->getRealRegister(), dependency->getRegister());
      }
   }


TR::RealRegister *OMR::X86::RegisterDependencyConditions::getRealRegisterFromVirtual(TR::Register *virtReg, TR::CodeGenerator *cg)
   {
   TR::Machine *machine = cg->machine();

   TR::RegisterDependencyGroup *depGroup = getPostConditions();
   for (int j = 0; j < getNumPostConditions(); j++)
      {
      TR::RegisterDependency  *dependency = depGroup->getRegisterDependency(j);

      if (dependency->getRegister() == virtReg)
         {
         return machine->getRealRegister(dependency->getRealRegister());
         }
      }

   depGroup = getPreConditions();
   for (int k = 0; k < getNumPreConditions(); k++)
      {
      TR::RegisterDependency  *dependency = depGroup->getRegisterDependency(k);

      if (dependency->getRegister() == virtReg)
         {
         return machine->getRealRegister(dependency->getRealRegister());
         }
      }

   TR_ASSERT(0, "getRealRegisterFromVirtual: shouldn't get here");
   return 0;
   }


#if defined(DEBUG) || defined(PROD_WITH_ASSUMES)
uint32_t OMR::X86::RegisterDependencyConditions::numReferencedFPRegisters(TR::CodeGenerator * cg)
   {
   TR::Machine *machine = cg->machine();
   uint32_t i, total = 0;
   TR::Register *reg;

   for (i=0; i<_numPreConditions; i++)
      {
      reg = _preConditions->getRegisterDependency(i)->getRegister();
      if ((reg && reg->getKind() == TR_X87) ||
          (!reg && _preConditions->getRegisterDependency(i)->isAllFPRegisters()))
         {
         total++;
         }
      }

   for (i=0; i<_numPostConditions; i++)
      {
      reg = _postConditions->getRegisterDependency(i)->getRegister();
      if ((reg && reg->getKind() == TR_X87) ||
          (!reg && _postConditions->getRegisterDependency(i)->isAllFPRegisters()))
         {
         total++;
         }
      }

   return total;
   }

uint32_t OMR::X86::RegisterDependencyConditions::numReferencedGPRegisters(TR::CodeGenerator * cg)
   {
   uint32_t i, total = 0;
   TR::Register *reg;

   for (i=0; i<_numPreConditions; i++)
      {
      reg = _preConditions->getRegisterDependency(i)->getRegister();
      if (reg && (reg->getKind() == TR_GPR || reg->getKind() == TR_FPR || reg->getKind() == TR_VRF))
         {
         total++;
         }
      }

   for (i=0; i<_numPostConditions; i++)
      {
      reg = _postConditions->getRegisterDependency(i)->getRegister();
      if (reg && (reg->getKind() == TR_GPR || reg->getKind() == TR_FPR || reg->getKind() == TR_VRF))
         {
         total++;
         }
      }

   return total;
   }

#endif // defined(DEBUG) || defined(PROD_WITH_ASSUMES)


////////////////////////////////////////////////////////////////////////////////
// Generate methods
////////////////////////////////////////////////////////////////////////////////

TR::RegisterDependencyConditions  *
generateRegisterDependencyConditions(uint32_t numPreConds,
                                     uint32_t numPostConds,
                                     TR::CodeGenerator * cg)
   {
   return new (cg->trHeapMemory()) TR::RegisterDependencyConditions(numPreConds, numPostConds, cg->trMemory());
   }

TR::RegisterDependencyConditions  *
generateRegisterDependencyConditions(TR::Node           *node,
                                     TR::CodeGenerator  *cg,
                                     uint32_t            additionalRegDeps,
                                     List<TR::Register> *popRegisters)
   {
   return new (cg->trHeapMemory()) TR::RegisterDependencyConditions(node, cg, additionalRegDeps, popRegisters);
   }
