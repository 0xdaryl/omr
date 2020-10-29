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

#include "codegen/RegisterDependencyGroup.hpp"

#include "codegen/BackingStore.hpp"
#include "codegen/CodeGenerator.hpp"
#include "codegen/LiveRegister.hpp"
#include "codegen/Machine.hpp"
#include "codegen/RealRegister.hpp"
#include "codegen/Register.hpp"
#include "codegen/RegisterDependency.hpp"
#include "codegen/X86Instruction.hpp"
#include "compile/Compilation.hpp"


void
OMR::X86::RegisterDependencyGroup::blockRealDependencyRegisters(uint32_t numberOfRegisters, TR::CodeGenerator *cg)
   {
   TR::Machine *machine = cg->machine();
   for (uint32_t i = 0; i < numberOfRegisters; i++)
      {
      if (!_dependencies[i].isNoReg())
         {
         machine->getRealRegister(_dependencies[i].getRealRegister())->block();
         }
      }
   }


void
OMR::X86::RegisterDependencyGroup::unblockRealDependencyRegisters(uint32_t numberOfRegisters, TR::CodeGenerator *cg)
   {
   TR::Machine *machine = cg->machine();
   for (uint32_t i = 0; i < numberOfRegisters; i++)
      {
      if (!_dependencies[i].isNoReg())
         {
         machine->getRealRegister(_dependencies[i].getRealRegister())->unblock();
         }
      }
   }


void
OMR::X86::RegisterDependencyGroup::assignRegisters(
      TR::Instruction   *currentInstruction,
      TR_RegisterKinds  kindsToBeAssigned,
      uint32_t          numberOfRegisters,
      TR::CodeGenerator *cg)
   {
   TR::Register             *virtReg              = NULL;
   TR::RealRegister         *assignedReg          = NULL;
   TR::RealRegister::RegNum  dependentRegNum      = TR::RealRegister::NoReg;
   TR::RealRegister         *dependentRealReg     = NULL;
   TR::RealRegister         *bestFreeRealReg      = NULL;
   TR::RealRegister::RegNum  bestFreeRealRegIndex = TR::RealRegister::NoReg;
   bool                      changed;
   int                       i, j;
   TR::Compilation *comp = cg->comp();

   TR::Machine *machine = cg->machine();
   blockRegisters(numberOfRegisters);

   // Build a work list of assignable register dependencies so the test does not
   // have to be repeated on each pass.  Also, order the list so that real
   // associations appear first, followed by byte reg associations, followed by
   // NoReg associations.
   //
   TR::RegisterDependency  **dependencies =
      (TR::RegisterDependency  **)cg->trMemory()->allocateStackMemory(numberOfRegisters * sizeof(TR::RegisterDependency  *));

   bool    hasByteDeps = false;
   bool    hasNoRegDeps = false;
   bool    hasBestFreeRegDeps = false;
   int32_t numRealAssocRegisters;
   int32_t numDependencyRegisters = 0;
   int32_t firstByteRegAssoc = 0, lastByteRegAssoc = 0;
   int32_t firstNoRegAssoc = 0, lastNoRegAssoc = 0;
   int32_t bestFreeRegAssoc = 0;

   for (i=0; i<numberOfRegisters; i++)
      {
      TR::RegisterDependency &regDep = _dependencies[i];
      virtReg = regDep.getRegister();

      if (virtReg && (kindsToBeAssigned & virtReg->getKindAsMask())
          && !regDep.isNoReg()
          && !regDep.isByteReg()
          && !regDep.isSpilledReg()
          && !regDep.isBestFreeReg())
         {
         dependencies[numDependencyRegisters++] = getRegisterDependency(i);
         }
      else if (regDep.isNoReg())
         hasNoRegDeps = true;
      else if (regDep.isByteReg())
         hasByteDeps = true;
      else if (regDep.isBestFreeReg())
         hasBestFreeRegDeps = true;

      // Handle spill registers first.
      //
      if (regDep.isSpilledReg())
         {
         if (cg->getUseNonLinearRegisterAssigner())
            {
            if (virtReg->getAssignedRegister())
               {
               TR_ASSERT(virtReg->getBackingStorage(), "should have a backing store for spilled reg virtuals");
               assignedReg = toRealRegister(virtReg->getAssignedRegister());

               TR::MemoryReference *tempMR = generateX86MemoryReference(virtReg->getBackingStorage()->getSymbolReference(), cg);

               TR_X86OpCodes op;
               if (assignedReg->getKind() == TR_FPR)
                  {
                  op = (assignedReg->isSinglePrecision()) ? MOVSSRegMem : (cg->getXMMDoubleLoadOpCode());
                  }
               else if (assignedReg->getKind() == TR_VRF)
                  {
                  op = MOVDQURegMem;
                  }
               else
                  {
                  op = LRegMem();
                  }

               TR::X86RegMemInstruction *inst =
                  new (cg->trHeapMemory()) TR::X86RegMemInstruction(currentInstruction, op, assignedReg, tempMR, cg);

               assignedReg->setAssignedRegister(NULL);
               virtReg->setAssignedRegister(NULL);
               assignedReg->setState(TR::RealRegister::Free);

               if (comp->getOption(TR_TraceNonLinearRegisterAssigner))
                  {
                  cg->traceRegisterAssignment("Generate reload of virt %s due to spillRegIndex dep at inst %p\n",comp->getDebug()->getName(virtReg),currentInstruction);
                  cg->traceRAInstruction(inst);
                  }
               }

            if (!(std::find(cg->getSpilledRegisterList()->begin(), cg->getSpilledRegisterList()->end(), virtReg) != cg->getSpilledRegisterList()->end()))
               {
               cg->getSpilledRegisterList()->push_front(virtReg);
               }
            }
         }
      }

   numRealAssocRegisters = numDependencyRegisters;

   if (hasByteDeps)
      {
      firstByteRegAssoc = numDependencyRegisters;
      for (i=0; i<numberOfRegisters; i++)
         {
         virtReg = _dependencies[i].getRegister();
         if (virtReg && (kindsToBeAssigned & virtReg->getKindAsMask()) &&
             _dependencies[i].isByteReg())
            {
            dependencies[numDependencyRegisters++] = getRegisterDependency(i);
            }
         }

      lastByteRegAssoc = numDependencyRegisters-1;
      }

   if (hasNoRegDeps)
      {
      firstNoRegAssoc = numDependencyRegisters;
      for (i=0; i<numberOfRegisters; i++)
         {
         virtReg = _dependencies[i].getRegister();
         if (virtReg && (kindsToBeAssigned & virtReg->getKindAsMask()) &&
             _dependencies[i].isNoReg())
            {
            dependencies[numDependencyRegisters++] = getRegisterDependency(i);
            }
         }

      lastNoRegAssoc = numDependencyRegisters-1;
      }

   if (hasBestFreeRegDeps)
      {
      bestFreeRegAssoc = numDependencyRegisters;
      for (i=0; i<numberOfRegisters; i++)
         {
         virtReg = _dependencies[i].getRegister();
         if (virtReg && (kindsToBeAssigned & virtReg->getKindAsMask()) &&
             _dependencies[i].isBestFreeReg())
            {
            dependencies[numDependencyRegisters++] = getRegisterDependency(i);
            }
         }

      TR_ASSERT((bestFreeRegAssoc == numDependencyRegisters-1), "there can only be one bestFreeRegDep in a dependency list");
      }

   // First look for long shot where two registers can be xchg'ed into their
   // proper real registers in one shot (seems to happen more often than one would think)
   //
   if (numRealAssocRegisters > 1)
      {
      for (i = 0; i < numRealAssocRegisters - 1; i++)
         {
         virtReg = dependencies[i]->getRegister();

         // We can only perform XCHG on GPRs.
         //
         if (virtReg->getKind() == TR_GPR)
            {
            dependentRegNum  = dependencies[i]->getRealRegister();
            dependentRealReg = machine->getRealRegister(dependentRegNum);
            assignedReg      = NULL;
            TR::RealRegister::RegNum assignedRegNum = TR::RealRegister::NoReg;
            if (virtReg->getAssignedRegister())
               {
               assignedReg    = toRealRegister(virtReg->getAssignedRegister());
               assignedRegNum = assignedReg->getRegisterNumber();
               }

            if (dependentRealReg->getState() == TR::RealRegister::Blocked &&
                dependentRegNum != assignedRegNum)
               {
               for (j = i+1; j < numRealAssocRegisters; j++)
                  {
                  TR::Register             *virtReg2         = dependencies[j]->getRegister();
                  TR::RealRegister::RegNum  dependentRegNum2 = dependencies[j]->getRealRegister();
                  TR::RealRegister         *assignedReg2     = NULL;
                  TR::RealRegister::RegNum  assignedRegNum2  = TR::RealRegister::NoReg;
                  if (virtReg2 && virtReg2->getAssignedRegister())
                     {
                     assignedReg2    = toRealRegister(virtReg2->getAssignedRegister());
                     assignedRegNum2 = assignedReg2->getRegisterNumber();
                     }

                  if (assignedRegNum2 == dependentRegNum &&
                      assignedRegNum == dependentRegNum2)
                     {
                     machine->swapGPRegisters(currentInstruction, assignedRegNum, assignedRegNum2);
                     break;
                     }
                  }
               }
            }
         }
      }

   // Next find registers for which there is an identified real register
   // and that register is currently free.  Want to do these first to get these
   // registers out of their existing registers and possibly freeing those registers
   // up to also be filled by the proper candidates as simply as possible
   //
   do
      {
      changed = false;
      for (i = 0; i < numRealAssocRegisters; i++)
         {
         virtReg = dependencies[i]->getRegister();
         dependentRegNum = dependencies[i]->getRealRegister();
         dependentRealReg = machine->getRealRegister(dependentRegNum);
         if (dependentRealReg->getState() == TR::RealRegister::Free)
            {
            if (virtReg->getKind() == TR_FPR || virtReg->getKind() == TR_VRF)
               machine->coerceXMMRegisterAssignment(currentInstruction, virtReg, dependentRegNum);
            else
               machine->coerceGPRegisterAssignment(currentInstruction, virtReg, dependentRegNum);
            virtReg->block();
            changed = true;
            }
         }
      } while (changed);

   // Next find registers for which there is an identified real register, but the
   // register is not yet assigned.  We could do this at the same time as the previous
   // loop, but that could lead to some naive changes that would have been easier once
   // all the opportunities found by the first loop have been found.
   //
   do
      {
      changed = false;
      for (i = 0; i < numRealAssocRegisters; i++)
         {
         virtReg = dependencies[i]->getRegister();
         assignedReg = toRealRegister(virtReg->getAssignedRealRegister());
         dependentRegNum = dependencies[i]->getRealRegister();
         dependentRealReg = machine->getRealRegister(dependentRegNum);
         if (dependentRealReg != assignedReg)
            {
            if (virtReg->getKind() == TR_FPR || virtReg->getKind() == TR_VRF)
               machine->coerceXMMRegisterAssignment(currentInstruction, virtReg, dependentRegNum);
            else
               machine->coerceGPRegisterAssignment(currentInstruction, virtReg, dependentRegNum);
            virtReg->block();
            changed = true;
            }
         }
      } while (changed);

   // Now all virtual registers which require a particular real register should
   // be in that register, so now find registers for which there is no required
   // real assignment, but which are not currently assigned to a real register
   // and make sure they are assigned.
   //
   // Assign byte register dependencies first.  Unblock any NoReg dependencies to not
   // constrain the byte register choices.
   //
   if (hasByteDeps)
      {
      if (hasNoRegDeps)
         {
         for (i=firstNoRegAssoc; i<=lastNoRegAssoc; i++)
            dependencies[i]->getRegister()->unblock();
         }

      do
         {
         changed = false;
         for (i = firstByteRegAssoc; i <= lastByteRegAssoc; i++)
            {
            virtReg = dependencies[i]->getRegister();
            if (toRealRegister(virtReg->getAssignedRealRegister()) == NULL)
               {
               machine->coerceGPRegisterAssignment(currentInstruction, virtReg, TR_ByteReg);
               virtReg->block();
               changed = true;
               }
            }
         } while (changed);

      if (hasNoRegDeps)
         {
         for (i=firstNoRegAssoc; i<=lastNoRegAssoc; i++)
            dependencies[i]->getRegister()->block();
         }
      }

   // Assign the remaining NoReg dependencies.
   //
   if (hasNoRegDeps)
      {
      do
         {
         changed = false;
         for (i = firstNoRegAssoc; i<=lastNoRegAssoc; i++)
            {
            virtReg = dependencies[i]->getRegister();
            if (toRealRegister(virtReg->getAssignedRealRegister()) == NULL)
               {
               if (virtReg->getKind() == TR_FPR || virtReg->getKind() == TR_VRF)
                  machine->coerceGPRegisterAssignment(currentInstruction, virtReg, TR_QuadWordReg);
               else
                  machine->coerceGPRegisterAssignment(currentInstruction, virtReg);
               virtReg->block();
               changed = true;
               }
            }
         } while (changed);
      }

   // Recommend the best available register without actually assigning it.
   //
   if (hasBestFreeRegDeps)
      {
      virtReg = dependencies[bestFreeRegAssoc]->getRegister();
      bestFreeRealReg = machine->findBestFreeGPRegister(currentInstruction, virtReg);
      bestFreeRealRegIndex = bestFreeRealReg ? bestFreeRealReg->getRegisterNumber() : TR::RealRegister::NoReg;
      }

   unblockRegisters(numberOfRegisters);

   for (i = 0; i < numDependencyRegisters; i++)
      {
      TR::Register *virtRegister = dependencies[i]->getRegister();
      assignedReg = toRealRegister(virtRegister->getAssignedRealRegister());

      // Document the registers to which NoReg registers get assigned for use
      // by snippets that need to look back and figure out which virtual
      // registers ended up assigned to which real registers.
      //
      if (dependencies[i]->isNoReg())
         dependencies[i]->setRealRegister(assignedReg->getRegisterNumber());

      if (dependencies[i]->isBestFreeReg())
         {
         dependencies[i]->setRealRegister(bestFreeRealRegIndex);
         virtRegister->decFutureUseCount();
         continue;
         }

      if (virtRegister->decFutureUseCount() == 0 &&
          assignedReg->getState() != TR::RealRegister::Locked)
         {
         virtRegister->setAssignedRegister(NULL);
         assignedReg->setAssignedRegister(NULL);
         assignedReg->setState(TR::RealRegister::Free);
         }
      }

   if (cg->getUseNonLinearRegisterAssigner())
      {
      for (i=0; i<numberOfRegisters; i++)
         {
         virtReg = _dependencies[i].getRegister();

         if (virtReg
             && (kindsToBeAssigned & virtReg->getKindAsMask())
             && _dependencies[i].isSpilledReg())
            {
            virtReg->decFutureUseCount();
            }
         }
      }

   }


void
OMR::X86::RegisterDependencyGroup::setDependencyInfo(
      uint32_t index,
      TR::Register *vr,
      TR::RealRegister::RegNum rr,
      TR::CodeGenerator *cg,
      uint8_t flag,
      bool isAssocRegDependency)
   {
   _dependencies[index].setRegister(vr);
   _dependencies[index].assignFlags(flag);
   _dependencies[index].setRealRegister(rr);

   if (vr &&
      vr->isLive() &&
      rr != TR::RealRegister::NoReg &&
      rr != TR::RealRegister::ByteReg)
      {
      TR::RealRegister *realReg = cg->machine()->getRealRegister(rr);
      if ((vr->getKind() == TR_GPR) && !isAssocRegDependency)
         {
         // Remember this association so that we can build interference info for
         // other live registers.
         //
         cg->getLiveRegisters(TR_GPR)->setAssociation(vr, realReg);
         }
      }
   }


void
OMR::X86::RegisterDependencyGroup::assignFPRegisters(
      TR::Instruction *prevInstruction,
      TR_RegisterKinds kindsToBeAssigned,
      uint32_t numberOfRegisters,
      TR::CodeGenerator *cg)
   {
   TR::Machine *machine = cg->machine();
   TR::Instruction *cursor  = prevInstruction;

   if (numberOfRegisters > 0)
      {
      TR::Register *reqdTopVirtReg = NULL;

      int32_t i;

#ifdef DEBUG
      int32_t numberOfFPRegisters = 0;
      for (i = 0; i < numberOfRegisters; i++)
         {
         TR::Register *virtReg = _dependencies[i].getRegister();
         if (virtReg && kindsToBeAssigned & virtReg->getKindAsMask())
            {
            if (_dependencies[i].getGlobalFPRegister())
               numberOfFPRegisters++;
            }
         }
#endif

      TR::X86LabelInstruction  *labelInstruction = NULL;

      if (prevInstruction->getNext())
         labelInstruction = prevInstruction->getNext()->getX86LabelInstruction();

      if (labelInstruction &&
          labelInstruction->getNeedToClearFPStack())
         {
         // Push the correct number of FP values in global registers
         // onto the stack; this would enable the stack height to be
         // correct at this point (start of extended basic block).
         //
         for (i = 0; i < numberOfRegisters; i++)
            {
            TR::Register *virtReg = _dependencies[i].getRegister();
            if (virtReg && kindsToBeAssigned & virtReg->getKindAsMask())
               {
               if (_dependencies[i].getGlobalFPRegister())
                  {
                  if ((virtReg->getFutureUseCount() == 0) ||
                      (virtReg->getTotalUseCount() == virtReg->getFutureUseCount()))
                     {
                     machine->fpStackPush(virtReg);
                     }
                  }
               }
            }
         }

      // Reverse all FP spills so that FP stack height
      // equals number of global registers as this point
      //
      for (i = 0; i < numberOfRegisters; i++)
         {
         TR::Register *virtReg = _dependencies[i].getRegister();
         if (virtReg && kindsToBeAssigned & virtReg->getKindAsMask())
            {
            if (((virtReg->getFutureUseCount() != 0) &&
                 (virtReg->getTotalUseCount() != virtReg->getFutureUseCount())) &&
                !virtReg->getAssignedRealRegister())
               {
               cursor = machine->reverseFPRSpillState(cursor, virtReg);
               }
            }
         }

      // Call the routine that places the global registers in correct order;
      // this routine tries to order them in as few exchanges as possible. This
      // routine could be improved slightly in the future.
      //
      List<TR::Register> popRegisters(cg->trMemory());
      orderGlobalRegsOnFPStack(cursor, kindsToBeAssigned, numberOfRegisters, &popRegisters, cg);

      for (i = 0; i < numberOfRegisters; i++)
         {
         TR::Register *virtReg = _dependencies[i].getRegister();
         if (virtReg && kindsToBeAssigned & virtReg->getKindAsMask())
            {
            if (virtReg->getTotalUseCount() != virtReg->getFutureUseCount())
               {
               // Original code that handles non global FP regs
               //
               if (!_dependencies[i].getGlobalFPRegister())
                  {
                  if (!machine->isFPRTopOfStack(virtReg))
                     {
                     cursor = machine->fpStackFXCH(cursor, virtReg);
                     }

                  if (virtReg->decFutureUseCount() == 0)
                     {
                     machine->fpStackPop();
                     }
                  }
               }
            else
               {
               // If this is the first reference of a register, then this must be the caller
               // side return value.  Assume it already exists on the FP stack if not a global
               // FP register; else coerce it to the right stack location. The required stack
               // must be available at this point.
               //
               if (_dependencies[i].getGlobalFPRegister())
                  {
                  int32_t reqdStackHeight = _dependencies[i].getRealRegister() - TR::RealRegister::FirstFPR;
                  machine->fpStackCoerce(virtReg, (machine->getFPTopOfStack() - reqdStackHeight));
                  virtReg->decFutureUseCount();
                  }
               else
                  {
                  if (virtReg->decFutureUseCount() != 0)
                     {
                     machine->fpStackPush(virtReg);
                     }
                  }
               }
            }
         else if (_dependencies[i].isAllFPRegisters())
            {
            // Spill the entire FP stack to memory.
            //
            cursor = machine->fpSpillStack(cursor);
            }
         }

      if (reqdTopVirtReg)
         {
         if (!machine->isFPRTopOfStack(reqdTopVirtReg))
            {
            cursor = machine->fpStackFXCH(cursor, reqdTopVirtReg);
            }
         }

#ifdef DEBUG
      bool onlyGlobalFPRA = true;
      bool onlySpillAllFP = true;
      bool globalFPRA = false;
      for (i = 0; i < numberOfRegisters; i++)
         {
         TR::Register *virtReg = _dependencies[i].getRegister();
         if (virtReg && kindsToBeAssigned & virtReg->getKindAsMask())
            {
            if (_dependencies[i].getGlobalFPRegister())
               {
               globalFPRA = true;
               onlySpillAllFP = false;
               TR_X86FPStackRegister *assignedRegister = toX86FPStackRegister(virtReg->getAssignedRealRegister());
               if (assignedRegister)
                  {
                  int32_t reqdStackHeight    = _dependencies[i].getRealRegister() - TR::RealRegister::FirstFPR;
                  int32_t currentStackHeight = machine->getFPTopOfStack() - assignedRegister->getFPStackRegisterNumber();

                  TR_ASSERT(reqdStackHeight == currentStackHeight,
                         "Stack height for global FP register is NOT correct\n");
                  }
               }
            }
         else
            {
            if (_dependencies[i].isAllFPRegisters())
               {
               // Spill the entire FP stack to memory.
               //
               cursor = machine->fpSpillStack(cursor);
               onlyGlobalFPRA = false;
               }
            }

         TR_ASSERT((!globalFPRA || ((machine->getFPTopOfStack()+1) == numberOfFPRegisters)),
                "Stack height is NOT correct\n");

         TR_ASSERT(onlyGlobalFPRA || onlySpillAllFP, "Illegal dependency\n");
         }
#endif

      // We may need to pop registers off the FP stack at this late stage in certain cases;
      // in particular if there are 2 global registers having the same value (same child C
      // in the GLRegDeps on a branch) and one of them is never used in the extended block after
      // the branch; then only one value will be popped off the stack when the last reference
      // to child C is seen in the extended basic block. This case cannot be detected during
      // instruction selection as the reference count on the child C is non-zero at the dependency
      // but in fact one of the registers must be popped off (late) in this case.
      //
      if (getMayNeedToPopFPRegisters() &&
          !popRegisters.isEmpty())
         {
         ListIterator<TR::Register> popRegsIt(&popRegisters);
         for (TR::Register *popRegister = popRegsIt.getFirst(); popRegister != NULL; popRegister = popRegsIt.getNext())
            {
            if (!machine->isFPRTopOfStack(popRegister))
               {
               cursor = machine->fpStackFXCH(cursor, popRegister);
               }

            TR::RealRegister *popRealRegister = machine->fpMapToStackRelativeRegister(popRegister);
            cursor = new (cg->trHeapMemory()) TR::X86FPRegInstruction(cursor, FSTPReg, popRealRegister, cg);
            machine->fpStackPop();
            }
         }
      }
   }


// Tries to coerce global FP registers into their required stack positions
// in fewest exchanges possible. Can be still improved; current worst case
// is 3N/2 exchanges where N is number of incorrect stack locations (this could happen
// if there are N/2 pairwise incorrect global registers. This could possibly be done in
// N+1 approx maybe (?)
//
void
OMR::X86::RegisterDependencyGroup::orderGlobalRegsOnFPStack(
      TR::Instruction *cursor,
      TR_RegisterKinds kindsToBeAssigned,
      uint32_t numberOfRegisters,
      List<TR::Register> *poppedRegisters,
      TR::CodeGenerator *cg)
   {
   TR::Machine *machine = cg->machine();
   int32_t *stackShape = machine->getFPStackShape();
   memset(stackShape, 0xff, TR_X86FPStackRegister::NumRegisters*sizeof(int32_t));

   // The data structure stackShape holds the required stack position j for each
   // value on the stack at a particular location i. This will be used in the
   // algorithm later.
   //
   int32_t topOfStack = machine->getFPTopOfStack();
   int32_t i;
   for (i = 0; i < numberOfRegisters; i++)
      {
      TR::Register *virtReg = _dependencies[i].getRegister();
      if (virtReg && kindsToBeAssigned & virtReg->getKindAsMask())
         {
         if (virtReg->getTotalUseCount() != virtReg->getFutureUseCount())
            {
            TR_X86FPStackRegister *assignedRegister = toX86FPStackRegister(virtReg->getAssignedRealRegister());
            TR_ASSERT(assignedRegister != NULL, "All spilled registers should have been reloaded by now\n");

            if (_dependencies[i].getGlobalFPRegister())
               {
               int32_t reqdStackHeight    = _dependencies[i].getRealRegister() - TR::RealRegister::FirstFPR;
               int32_t currentStackHeight = topOfStack - assignedRegister->getFPStackRegisterNumber();
               stackShape[currentStackHeight] = reqdStackHeight;
               }
            }
         }
      }

   TR::Register *reqdTopVirtReg = NULL;
   for (i = 0; i < numberOfRegisters; i++)
      {
      TR::Register *virtReg = _dependencies[i].getRegister();
      if (virtReg && kindsToBeAssigned & virtReg->getKindAsMask())
         {
         if (virtReg->getTotalUseCount() != virtReg->getFutureUseCount())
            {
            // If this is not the first reference of a register, then coerce it to the
            // top of stack.
            //
            if (_dependencies[i].getGlobalFPRegister())
               {
               int32_t reqdStackHeight = _dependencies[i].getRealRegister() - TR::RealRegister::FirstFPR;
               TR_X86FPStackRegister *assignedRegister = toX86FPStackRegister(virtReg->getAssignedRealRegister());
               int32_t currentStackHeight =  topOfStack - assignedRegister->getFPStackRegisterNumber();

               if (reqdStackHeight == 0)
                  {
                  reqdTopVirtReg = virtReg;
                  }

               TR::Register *origRegister = virtReg;
               while ((reqdStackHeight != currentStackHeight) &&
                      (reqdStackHeight >= 0))
                  {
                  if (!machine->isFPRTopOfStack(virtReg))
                     {
                     cursor = machine->fpStackFXCH(cursor, virtReg);
                     }

                  assignedRegister = toX86FPStackRegister(virtReg->getAssignedRealRegister());
                  int32_t tempCurrentStackHeight =  topOfStack - assignedRegister->getFPStackRegisterNumber();
                  if (reqdStackHeight != tempCurrentStackHeight)
                     {
                     cursor = machine->fpStackFXCH(cursor, reqdStackHeight);
                     }

#if DEBUG
                  if ((currentStackHeight < 0) ||
                      (currentStackHeight >= TR_X86FPStackRegister::NumRegisters))
                     TR_ASSERT(0, "Array out of bounds exception in array stackShape\n");
                  if ((reqdStackHeight < 0) ||
                      (reqdStackHeight >= TR_X86FPStackRegister::NumRegisters))
                     TR_ASSERT(0, "Array out of bounds exception in array stackShape\n");
#endif

                  // This is the new stack shape now after the exchanges done above
                  //
                  stackShape[currentStackHeight] = stackShape[0];
                  stackShape[0] = stackShape[reqdStackHeight];
                  stackShape[reqdStackHeight] = reqdStackHeight;

                  // Drive this loop by the value on top of the stack
                  //
                  reqdStackHeight = stackShape[0];
                  currentStackHeight = 0;
                  virtReg = machine->getFPStackLocationPtr(topOfStack)->getAssignedRegister();
                  }

               if (origRegister->decFutureUseCount() == 0)
                  {
                  // It is possible for a global register to go dead at a dependency;
                  // add it to the list of registers to be popped in this case (discussed
                  // in more detail above)
                  //
                  poppedRegisters->add(origRegister);
                  }
               }
            }
         }
      }

   if (reqdTopVirtReg)
      {
      if (!machine->isFPRTopOfStack(reqdTopVirtReg))
         {
         cursor = machine->fpStackFXCH(cursor, reqdTopVirtReg);
         }
      }
   }

