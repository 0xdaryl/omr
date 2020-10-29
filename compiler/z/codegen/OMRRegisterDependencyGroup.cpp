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

// On zOS XLC linker can't handle files with same name at link time
// This workaround with pragma is needed. What this does is essentially
// give a different name to the codesection (csect) for this file. So it
// doesn't conflict with another file with same name.

#pragma csect(CODE,"OMRZRegisterDependencyGroup#C")
#pragma csect(STATIC,"OMRZRegisterDependencyGroup#S")
#pragma csect(TEST,"OMRZRegisterDependencyGroup#T")

#include <stddef.h>
#include <stdint.h>

#include "codegen/RegisterDependencyGroup.hpp"
#include "codegen/RegisterDependencyGroup_inlines.hpp"

#include "codegen/BackingStore.hpp"
#include "codegen/CodeGenerator.hpp"
#include "codegen/Instruction.hpp"
#include "codegen/LiveRegister.hpp"
#include "codegen/Machine.hpp"
#include "codegen/RealRegister.hpp"
#include "codegen/Register.hpp"
#include "codegen/RegisterDependency.hpp"
#include "codegen/S390GenerateInstructions.hpp"
#include "codegen/S390Instruction.hpp"
#include "compile/Compilation.hpp"
#include "control/Options.hpp"
#include "infra/Assert.hpp"
#include "il/LabelSymbol.hpp"
#include "il/Node.hpp"
#include "ras/Debug.hpp"


void
OMR::Z::RegisterDependencyGroup::setDependencyInfo(
      uint32_t index,
      TR::Register *vr,
      TR::RealRegister::RegDep rd,
      uint8_t flag)
   {
   self()->setDependencyInfo(index, vr, static_cast<TR::RealRegister::RegNum>(rd), flag);
   }


uint32_t
OMR::Z::RegisterDependencyGroup::genBitMapOfAssignableGPRs(
      TR::CodeGenerator * cg,
      uint32_t numberOfRegisters)
   {
   TR::Machine* machine = cg->machine();

   // TODO: This looks like it can be cached as it should be the same value every time in a single compilation. Should
   // also investigate whether we even need this? Don't we already block registers which are part of a dependency? I
   // suppose with a mask we can avoid shuffling things into GPRs which are part of the dependency and may need to get
   // other virtual registers shuffled into such GPRs. Need to think about this. Are other codegens doing this?
   uint32_t availRegMap = machine->genBitMapOfAssignableGPRs();

   for (uint32_t i = 0; i < numberOfRegisters; i++)
      {
      TR::RealRegister::RegNum realReg = _dependencies[i].getRealRegister();
      if (TR::RealRegister::isGPR(realReg))
         {
         availRegMap &= ~machine->getRealRegister(realReg)->getRealRegisterMask();
         }
      }

   return availRegMap;
   }


void
OMR::Z::RegisterDependencyGroup::assignRegisters(
      TR::Instruction *currentInstruction,
      TR_RegisterKinds kindToBeAssigned,
      uint32_t numOfDependencies,
      TR::CodeGenerator *cg)
   {
   TR::Compilation *comp = cg->comp();
   TR::Machine *machine = cg->machine();
   TR::Register * virtReg;
   TR::RealRegister::RegNum dependentRegNum;
   TR::RealRegister * dependentRealReg, * assignedRegister;
   int32_t i, j;
   bool changed;
   uint32_t availRegMask = self()->genBitMapOfAssignableGPRs(cg, numOfDependencies);

   if (!comp->getOption(TR_DisableOOL))
      {
      for (i = 0; i< numOfDependencies; i++)
         {
         TR::RegisterDependency &regDep = _dependencies[i];
         virtReg = regDep.getRegister();
         if (regDep.isSpilledReg() && !virtReg->getRealRegister())
            {
            TR_ASSERT_FATAL(virtReg->getBackingStorage() != NULL, "Spilled virtual register on dependency list does not have backing storage");

            if (virtReg->getAssignedRealRegister())
               {
               // this happens when the register was first spilled in main line path then was reverse spilled
               // and assigned to a real register in OOL path. We protected the backing store when doing
               // the reverse spill so we could re-spill to the same slot now
               TR::Node *currentNode = currentInstruction->getNode();
               TR::RealRegister *assignedReg = toRealRegister(virtReg->getAssignedRegister());
               cg->traceRegisterAssignment ("\nOOL: Found %R spilled in main line and reused inside OOL", virtReg);
               TR::MemoryReference * tempMR = generateS390MemoryReference(currentNode, virtReg->getBackingStorage()->getSymbolReference(), cg);
               TR::InstOpCode::Mnemonic opCode;
               TR_RegisterKinds rk = virtReg->getKind();
               switch (rk)
                  {
                  case TR_GPR:
                     if (virtReg->is64BitReg())
                        {
                        opCode = TR::InstOpCode::LG;
                        }
                     else
                        {
                        opCode = TR::InstOpCode::L;
                        }
                     break;
                  case TR_FPR:
                     opCode = TR::InstOpCode::LD;
                     break;
                  case TR_VRF:
                     opCode = TR::InstOpCode::VL;
                     break;
                  default:
                     TR_ASSERT( 0, "\nRegister kind not supported in OOL spill\n");
                     break;
                  }

               bool isVector = (rk == TR_VRF);

               TR::Instruction *inst = isVector ? generateVRXInstruction(cg, opCode, currentNode, assignedReg, tempMR, 0, currentInstruction) :
                                                     generateRXInstruction (cg, opCode, currentNode, assignedReg, tempMR, currentInstruction);
               cg->traceRAInstruction(inst);

               assignedReg->setAssignedRegister(NULL);
               virtReg->setAssignedRegister(NULL);
               assignedReg->setState(TR::RealRegister::Free);
               }

            // now we are leaving the OOL sequence, anything that was previously spilled in OOL hot path or main line
            // should be considered as mainline spill for the next OOL sequence.
            virtReg->getBackingStorage()->setMaxSpillDepth(1);
            }
         // we also need to free up all locked backing storage if we are exiting the OOL during backwards RA assignment
         else if (currentInstruction->isLabel() && virtReg->getAssignedRealRegister())
            {
            TR_BackingStore * location = virtReg->getBackingStorage();
            TR_RegisterKinds rk = virtReg->getKind();
            int32_t dataSize;
            if (toS390LabelInstruction(currentInstruction)->getLabelSymbol()->isStartOfColdInstructionStream() &&
                location)
               {
               traceMsg (comp,"\nOOL: Releasing backing storage (%p)\n", location);
               if (rk == TR_GPR)
                  dataSize = TR::Compiler->om.sizeofReferenceAddress();
               else if (rk == TR_VRF)
                  dataSize = 16; // Will change to 32 in a few years..
               else
                  dataSize = 8;

               location->setMaxSpillDepth(0);
               cg->freeSpill(location,dataSize,0);
               virtReg->setBackingStorage(NULL);
               }
            }
         }
      }

   uint32_t numGPRs = 0;
   uint32_t numFPRs = 0;
   uint32_t numVRFs = 0;

   // Used to do lookups using real register numbers
   OMR::RegisterDependencyMap map(_dependencies, numOfDependencies);

   for (i = 0; i < numOfDependencies; i++)
      {
      TR::RegisterDependency &regDep = _dependencies[i];

      map.addDependency(regDep, i);

      if (!regDep.isSpilledReg() &&
          !regDep.isKillVolHighRegs() &&
          !regDep.isNoReg())
         {
         virtReg = regDep.getRegister();
         switch (virtReg->getKind())
            {
            case TR_GPR:
               ++numGPRs;
               break;
            case TR_FPR:
               ++numFPRs;
               break;
            case TR_VRF:
               ++numVRFs;
               break;
            default:
               break;
            }
         }
      }

#ifdef DEBUG
   int32_t lockedGPRs = 0;
   int32_t lockedFPRs = 0;
   int32_t lockedVRFs = 0;

   // count up how many registers are locked for each type
   for (int32_t i = TR::RealRegister::FirstGPR; i <= TR::RealRegister::LastGPR; ++i)
      {
      TR::RealRegister* realReg = machine->getRealRegister((TR::RealRegister::RegNum)i);
      if (realReg->getState() == TR::RealRegister::Locked)
         ++lockedGPRs;
      }

   for (int32_t i = TR::RealRegister::FirstFPR; i <= TR::RealRegister::LastFPR; ++i)
      {
      TR::RealRegister* realReg = machine->getRealRegister((TR::RealRegister::RegNum)i);
      if (realReg->getState() == TR::RealRegister::Locked)
         ++lockedFPRs;
      }

   for (int32_t i = TR::RealRegister::FirstVRF; i <= TR::RealRegister::LastVRF; ++i)
      {
      TR::RealRegister* realReg = machine->getRealRegister((TR::RealRegister::RegNum)i);
      if (realReg->getState() == TR::RealRegister::Locked)
         ++lockedVRFs;
      }

   TR_ASSERT(lockedGPRs == machine->getNumberOfLockedRegisters(TR_GPR), "Inconsistent number of locked GPRs");
   TR_ASSERT(lockedFPRs == machine->getNumberOfLockedRegisters(TR_FPR), "Inconsistent number of locked FPRs");
   TR_ASSERT(lockedVRFs == machine->getNumberOfLockedRegisters(TR_VRF), "Inconsistent number of locked VRFs");
#endif

   // To handle circular dependencies, we block a real register if (1) it is already assigned to a correct
   // virtual register and (2) if it is assigned to one register in the list but is required by another.
   // However, if all available registers are requested, we do not block in case (2) to avoid all registers
   // being blocked.

   bool haveSpareGPRs = true;
   bool haveSpareFPRs = true;
   bool haveSpareVRFs = true;

   TR_ASSERT(numGPRs <= (TR::RealRegister::LastGPR - TR::RealRegister::FirstGPR + 1 - machine->getNumberOfLockedRegisters(TR_GPR)), "Too many GPR dependencies, unable to assign");
   TR_ASSERT(numFPRs <= (TR::RealRegister::LastFPR - TR::RealRegister::FirstFPR + 1 - machine->getNumberOfLockedRegisters(TR_FPR)), "Too many FPR dependencies, unable to assign");
   TR_ASSERT(numVRFs <= (TR::RealRegister::LastVRF - TR::RealRegister::FirstVRF + 1 - machine->getNumberOfLockedRegisters(TR_VRF)), "Too many VRF dependencies, unable to assign");

   if (numGPRs == (TR::RealRegister::LastGPR - TR::RealRegister::FirstGPR + 1 - machine->getNumberOfLockedRegisters(TR_GPR)))
      haveSpareGPRs = false;
   if (numFPRs == (TR::RealRegister::LastFPR - TR::RealRegister::FirstFPR + 1 - machine->getNumberOfLockedRegisters(TR_FPR)))
      haveSpareFPRs = false;
   if (numVRFs == (TR::RealRegister::LastVRF - TR::RealRegister::FirstVRF + 1 - machine->getNumberOfLockedRegisters(TR_VRF)))
      haveSpareVRFs = false;

   for (i = 0; i < numOfDependencies; i++)
      {
      virtReg = _dependencies[i].getRegister();

      if (virtReg->getAssignedRealRegister() != NULL)
         {
         if (_dependencies[i].isNoReg())
            {
            virtReg->block();
            }
         else
            {
            TR::RealRegister::RegNum assignedRegNum = toRealRegister(virtReg->getAssignedRealRegister())->getRegisterNumber();

            // Always block if the required register and assigned register match or if the assigned register is
            // required by another dependency but only if there are any spare registers left so as to avoid blocking
            // all existing registers
            if (_dependencies[i].getRealRegister() == assignedRegNum ||
                (map.getDependencyWithTarget(assignedRegNum) &&
                 ((virtReg->getKind() != TR_GPR || haveSpareGPRs) &&
                  (virtReg->getKind() != TR_FPR || haveSpareFPRs) &&
                  (virtReg->getKind() != TR_VRF || haveSpareVRFs))))
               {
               virtReg->block();
               }
            }
         }

      // Check for directive to spill high registers. Used on callouts
      if ( _dependencies[i].isKillVolHighRegs())
         {
         machine->spillAllVolatileHighRegisters(currentInstruction);
         _dependencies[i].setRealRegister(TR::RealRegister::NoReg);  // ignore from now on
         }
      }

   /////    REGISTER PAIRS

   // We handle register pairs by passing regpair dependencies stubs into the register dependency
   // list.  The following pieces of code are responsible for replacing these stubs with actual
   // real register values, using register allocation routines in the S390Machine, and hence,
   // allow the dependency routines to complete the allocation.
   //
   // We need to run through the list of dependencies looking for register pairs.
   // We assign a register pair to any such pairs that are found and then look for the low
   // and high order regs in the list of deps.  We update the low and high order pair deps with
   // the real reg assigned by to the pair, and then let the these deps be handled as any other
   // single reg dep.
   //
   // If the virt registers have already been assigned, the low and high order
   // regs obtained from regPair will not match the ones in the dependency list.
   // Hence, will fall through the dep entries for the associated virt high
   // and low regs as the real high and low will not match the dep virt reg entries.
   //
   for (i = 0; i < numOfDependencies; i++)
      {
      TR::RegisterDependency &regDep = _dependencies[i];

      // Get reg pair pointer
      //
      TR::RegisterPair * virtRegPair = regDep.getRegister()->getRegisterPair();

      // If it is a register pair
      //
      if (virtRegPair == NULL)
         {
         continue;
         }

      // Is this an Even-Odd pair assignment
      //
      if (regDep.isEvenOddPair()
          || regDep.isFPPair())
         {
         TR::Register * virtRegHigh = virtRegPair->getHighOrder();
         TR::Register * virtRegLow = virtRegPair->getLowOrder();
         TR::RegisterPair * retPair = NULL;

         #if 0
         fprintf(stderr, "DEP: PRE REG PAIR ASS: %p(%p,%p)->(%p,%p)\n", virtRegPair, virtRegPair->getHighOrder(),
            virtRegPair->getLowOrder(), virtRegPair->getHighOrder()->getAssignedRegister(),
            virtRegPair->getLowOrder()->getAssignedRegister());
         #endif

         // Assign a real register pair
         // This call returns a pair placeholder that points to the assigned real registers
         //     retPair:
         //                 high -> realRegHigh -> virtRegHigh
         //                 low  -> realRegLow  -> virtRegLow
         //
         // No bookkeeping on assignment call as we do bookkeeping at end of this method
         //
         retPair = (TR::RegisterPair *) machine->assignBestRegister((TR::Register *) virtRegPair,
                                                                   currentInstruction,
                                                                   NOBOOKKEEPING,
                                                                   availRegMask);

         // Grab real regs that are returned as high/low in retPair
         //
         TR::Register * realRegHigh = retPair->getHighOrder();
         TR::Register * realRegLow  = retPair->getLowOrder();

         // We need to update the virtRegPair assignedRegister pointers
         // The real regs should already be pointing at the virtuals after
         // returning from the assignment call.
         //
         virtRegPair->getHighOrder()->setAssignedRegister(realRegHigh);
         virtRegPair->getLowOrder()->setAssignedRegister(realRegLow);

         #if 0
         fprintf(stderr, "DEP: POST REG PAIR ASS: %p(%p,%p)->(%p,%p)=(%d,%d)\n", virtRegPair, virtRegPair->getHighOrder(),
            virtRegPair->getLowOrder(), virtRegPair->getHighOrder()->getAssignedRegister(),
            virtRegPair->getLowOrder()->getAssignedRegister(), toRealRegister(realRegHigh)->getRegisterNumber() - 1,
            toRealRegister(realRegLow)->getRegisterNumber() - 1);
         #endif

         // We are done with the reg pair structure.  No assignment later here.
         //
         regDep.setRealRegister(TR::RealRegister::NoReg);

         // See if there are any LegalEvenOfPair/LegalOddOfPair deps in list that correspond
         // to the assigned pair
         //
         for (j = 0; j < numOfDependencies; j++)
            {
            TR::RegisterDependency &innerRegDep = _dependencies[j];

            // Check to ensure correct assignment of even/odd pair.
            //
            if (((innerRegDep.isLegalEvenOfPair()
                 || innerRegDep.isLegalFirstOfFPPair()) &&
                 innerRegDep.getRegister() == virtRegLow) ||
                ((innerRegDep.isLegalOddOfPair()
                 || innerRegDep.isLegalSecondOfFPPair())  &&
                 innerRegDep.getRegister() == virtRegHigh))
               {
               TR_ASSERT( 0, "Register pair odd and even assigned wrong\n");
               }

            // Look for the Even reg in deps
            if ((innerRegDep.isLegalEvenOfPair()
                || innerRegDep.isLegalFirstOfFPPair()) &&
                innerRegDep.getRegister() == virtRegHigh)
               {
               innerRegDep.setRealRegister(TR::RealRegister::NoReg);
               toRealRegister(realRegLow)->block();
               }

            // Look for the Odd reg in deps
            if ((innerRegDep.isLegalOddOfPair()
                || innerRegDep.isLegalSecondOfFPPair()) &&
                innerRegDep.getRegister() == virtRegLow)
               {
               innerRegDep.setRealRegister(TR::RealRegister::NoReg);
               toRealRegister(realRegHigh)->block();
               }
            }

         changed = true;
         }
      }

   // If we have an [Even|Odd]OfLegal of that has not been associated
   // to a reg pair in this list (in case we only need to ensure that the
   // reg can be assigned to a pair in the future), we need to handle such
   // cases here.
   //
   // It is possible that left over legal even/odd regs are actually real regs
   // that were already assigned in a previous instruction.  We check for this
   // case before assigning a new reg by checking for assigned real regs.  If
   // real reg is assigned, we need to make sure it is a legal assignment.
   //
   for (i = 0; i < numOfDependencies; i++)
      {
      TR::RegisterDependency &regDep = _dependencies[i];
      TR::Register * virtReg = regDep.getRegister();
      TR::Register * assRealReg = virtReg->getAssignedRealRegister();

      if (regDep.isLegalEvenOfPair())
         {
         // if not assigned, or assigned but not legal.
         //
         if (assRealReg == NULL || (!machine->isLegalEvenRegister(toRealRegister(assRealReg), ALLOWBLOCKED)))
            {
            TR::RealRegister * bestEven = machine->findBestLegalEvenRegister();
            regDep.setRealRegister(TR::RealRegister::NoReg);
            toRealRegister(bestEven)->block();

            #ifdef DEBUG_MMIT
            fprintf(stderr, "DEP ASS Best Legal Even %p (NOT pair)\n", toRealRegister(bestEven)->getRegisterNumber() - 1);
            #endif
            }
         else
            {
            regDep.setRealRegister(TR::RealRegister::NoReg);
            }
         }

      if (regDep.isLegalOddOfPair())
         {
         // if not assigned, or assigned but not legal.
         //
         if (assRealReg == NULL || (!machine->isLegalOddRegister(toRealRegister(assRealReg), ALLOWBLOCKED)))
            {
            TR::RealRegister * bestOdd = machine->findBestLegalOddRegister();
            regDep.setRealRegister(TR::RealRegister::NoReg);
            toRealRegister(bestOdd)->block();

            #ifdef DEBUG_MMIT
            fprintf(stderr, "DEP ASS Best Legal Odd %p (NOT pair)\n", toRealRegister(bestOdd)->getRegisterNumber() - 1);
            #endif
            }
         else
            {
            regDep.setRealRegister(TR::RealRegister::NoReg);
            }
         }

      if (regDep.isLegalFirstOfFPPair())
         {
         // if not assigned, or assigned but not legal.
         //
         if (assRealReg == NULL || (!machine->isLegalFirstOfFPRegister(toRealRegister(assRealReg), ALLOWBLOCKED)))
            {
            TR::RealRegister * bestFirstFP = machine->findBestLegalSiblingFPRegister(true);
            regDep.setRealRegister(TR::RealRegister::NoReg);
            toRealRegister(bestFirstFP)->block();

            #ifdef DEBUG_FPPAIR
            fprintf(stderr, "DEP ASS Best Legal FirstOfFPPair %p (NOT pair)\n", toRealRegister(bestFirstFP)->getRegisterNumber() - 1);
            #endif
            }
         else
            {
            regDep.setRealRegister(TR::RealRegister::NoReg);
            }
         }

      if (regDep.isLegalSecondOfFPPair())
         {
         // if not assigned, or assigned but not legal.
         //
         if (assRealReg == NULL || (!machine->isLegalSecondOfFPRegister(toRealRegister(assRealReg), ALLOWBLOCKED)))
            {
            TR::RealRegister * bestSecondFP = machine->findBestLegalSiblingFPRegister(false);
            regDep.setRealRegister(TR::RealRegister::NoReg);
            toRealRegister(bestSecondFP)->block();

            #ifdef DEBUG_FPPAIR
            fprintf(stderr, "DEP ASS Best Legal SecondOfFPPair  %p (NOT pair)\n", toRealRegister(bestSecondFP)->getRegisterNumber() - 1);
            #endif
            }
         else
            {
            regDep.setRealRegister(TR::RealRegister::NoReg);
            }
         }

      }// for

   #if 0
    {
      fprintf(stdout,"PART II ---------------------------------------\n");
      fprintf(stdout,"DEPENDS for INST [%p]\n",currentInstruction);
      printDeps(stdout, numberOfRegisters);
    }
   #endif


   /////    COERCE

   // These routines are responsible for assigning registers and using necessary
   // ops to ensure that any defined dependency [virt reg, real reg] (where real reg
   // is not NoReg), is assigned such that virtreg->realreg.
   //
   do
      {
      changed = false;

      // For each dependency
      //
      for (i = 0; i < numOfDependencies; i++)
         {
         TR::RegisterDependency &regDep = _dependencies[i];

         virtReg = regDep.getRegister();

         TR_ASSERT( virtReg != NULL, "null virtual register during coercion");
         dependentRegNum = regDep.getRealRegister();

         dependentRealReg = machine->getRealRegister(dependentRegNum);

         // If dep requires a specific real reg, and the real reg is free
         if (!regDep.isNoReg() &&
             !regDep.isAssignAny() &&
             !regDep.isSpilledReg() &&
             dependentRealReg->getState() == TR::RealRegister::Free)
            {
            #if 0
               fprintf(stdout, "COERCE1 %i (%s, %d)\n", i, cg->getDebug()->getName(virtReg), dependentRegNum-1);
            #endif

            // Assign the real reg to the virt reg
            machine->coerceRegisterAssignment(currentInstruction, virtReg, dependentRegNum);
            virtReg->block();
            changed = true;
            }
         }
      }
   while (changed == true);


   do
      {
      changed = false;
      // For each dependency
      for (i = 0; i < numOfDependencies; i++)
         {
         TR::RegisterDependency &regDep = _dependencies[i];
         virtReg = regDep.getRegister();
         assignedRegister = NULL;

         // Is a real reg assigned to the virt reg
         if (virtReg->getAssignedRealRegister() != NULL)
            {
            assignedRegister = toRealRegister(virtReg->getAssignedRealRegister());
            }

         dependentRegNum = regDep.getRealRegister();
         dependentRealReg = machine->getRealRegister(dependentRegNum);

         // If the dependency requires a real reg
         // and the assigned real reg is not equal to the required one
         if (!regDep.isNoReg() &&
             !regDep.isAssignAny() &&
             !regDep.isSpilledReg())
            {
            if(dependentRealReg != assignedRegister)
              {
              // Coerce the assignment
              #if 0
               fprintf(stdout, "COERCE2 %i (%s, %d)\n", i, cg->getDebug()->getName(virtReg), dependentRegNum-1);
              #endif

               machine->coerceRegisterAssignment(currentInstruction, virtReg, dependentRegNum);
               virtReg->block();
               changed = true;

               // If there was a wrongly assigned real register
               if (assignedRegister != NULL && assignedRegister->getState() == TR::RealRegister::Free)
                 {
                 // For each dependency
                 for (int32_t lcount = 0; lcount < numOfDependencies; lcount++)
                   {
                   TR::RegisterDependency &regDep = _dependencies[lcount];
                   // get the dependencies required real reg
                   TR::RealRegister::RegNum aRealNum = regDep.getRealRegister();
                   // If the real reg is the same as the one just assigned
                   if (!regDep.isNoReg() &&
                       !regDep.isAssignAny() &&
                       !regDep.isSpilledReg() &&
                       assignedRegister == machine->getRealRegister(aRealNum))
                     {
                       #if 0
                        fprintf(stdout, "COERCE3 %i (%s, %d)\n", i, cg->getDebug()->getName(virtReg), aRealNum-1);
                       #endif

                        // Coerce the assignment
                        TR::Register *depReg = _dependencies[lcount].getRegister();
                        machine->coerceRegisterAssignment(currentInstruction, depReg, aRealNum);
                        _dependencies[lcount].getRegister()->block();
                        break;
                     }
                   }
                 }
              }
            }
         }
      }
   while (changed == true);



   /////    ASSIGN ANY

   // AssignAny Support
   // This code allows any register to be assigned to the dependency.
   // This is generally used when we wish to force an early assignment to a virt reg
   // (before the first inst that actually uses the reg).
   //
   // assign those used in memref first
   for (i = 0; i < numOfDependencies; i++)
      {
      if (_dependencies[i].isAssignAny())
         {
         virtReg = _dependencies[i].getRegister();

         if(virtReg->isUsedInMemRef())
           {
           uint32_t availRegMask =~TR::RealRegister::GPR0Mask;

            // No bookkeeping on assignment call as we do bookkeeping at end of this method
            TR::Register * targetReg = machine->assignBestRegister(virtReg, currentInstruction, NOBOOKKEEPING, availRegMask);

            // Disable this dependency
            _dependencies[i].setRealRegister(toRealRegister(targetReg)->getRegisterNumber());
            virtReg->block();
           }
         }
      }

   // assign those which are not used in memref
   for (i = 0; i < numOfDependencies; i++)
      {
      if (_dependencies[i].isAssignAny())
         {
         virtReg = _dependencies[i].getRegister();

         if(!virtReg->isUsedInMemRef())
            {
            virtReg = _dependencies[i].getRegister();
            // No bookkeeping on assignment call as we do bookkeeping at end of this method
            TR::Register * targetReg = machine->assignBestRegister(virtReg, currentInstruction, NOBOOKKEEPING, 0xffffffff);

            // Disable this dependency
            _dependencies[i].setRealRegister(toRealRegister(targetReg)->getRegisterNumber());

            virtReg->block();
            }
          }
      }

   /////    UNBLOCK


   // Unblock all other regs
   //
   self()->unblockRegisters(numOfDependencies);



   /////    BOOKKEEPING

   // Check to see if we can release any registers that are not used in the future
   // This is why we omit BOOKKEEPING in the assign calls above
   //
   for (i = 0; i < numOfDependencies; i++)
      {
      TR::Register * dependentRegister = self()->getRegisterDependency(i)->getRegister();

      // We decrement the use count, and kill if there are no further uses
      // We pay special attention to pairs, as it is not the parent placeholder
      // that must have its count decremented, but rather the children.
      if (dependentRegister->getRegisterPair() == NULL &&
          dependentRegister->getAssignedRegister() != NULL &&
          !self()->getRegisterDependency(i)->isSpilledReg()
         )
         {
         TR::Register * assignedRegister = dependentRegister->getAssignedRegister();
         TR::RealRegister * assignedRealRegister = assignedRegister != NULL ? assignedRegister->getRealRegister() : NULL;
         dependentRegister->decFutureUseCount();
         dependentRegister->setIsLive();
         if (assignedRegister && assignedRealRegister->getState() != TR::RealRegister::Locked)
            {
            TR_ASSERT(dependentRegister->getFutureUseCount() >=0,
                    "\nReg dep assignment: futureUseCount should not be negative\n");
            }
         if (dependentRegister->getFutureUseCount() == 0)
            {
            dependentRegister->setAssignedRegister(NULL);
            dependentRegister->resetIsLive();
            if(assignedRealRegister)
              {
              assignedRealRegister->setState(TR::RealRegister::Unlatched);
              if (assignedRealRegister->getState() == TR::RealRegister::Locked)
                 assignedRealRegister->setAssignedRegister(NULL);
              cg->traceRegFreed(dependentRegister, assignedRealRegister);
              }
            }
         }
      else if (dependentRegister->getRegisterPair() != NULL)
         {
         TR::Register * dependentRegisterHigh = dependentRegister->getHighOrder();

         dependentRegisterHigh->decFutureUseCount();
         dependentRegisterHigh->setIsLive();
         TR::Register * highAssignedRegister = dependentRegisterHigh->getAssignedRegister();

         if (highAssignedRegister &&
             highAssignedRegister->getRealRegister()->getState() != TR::RealRegister::Locked)
            {
            TR_ASSERT(dependentRegisterHigh->getFutureUseCount() >=0,
                    "\nReg dep assignment: futureUseCount should not be negative\n");
            }
         if (highAssignedRegister &&
             dependentRegisterHigh->getFutureUseCount() == 0)
            {
            TR::RealRegister * assignedRegister = highAssignedRegister->getRealRegister();
            dependentRegisterHigh->setAssignedRegister(NULL);
            assignedRegister->setState(TR::RealRegister::Unlatched);
            if (assignedRegister->getState() == TR::RealRegister::Locked)
               assignedRegister->setAssignedRegister(NULL);

            cg->traceRegFreed(dependentRegisterHigh, assignedRegister);
            }

         TR::Register * dependentRegisterLow = dependentRegister->getLowOrder();

         dependentRegisterLow->decFutureUseCount();
         dependentRegisterLow->setIsLive();

         TR::Register * lowAssignedRegister = dependentRegisterLow->getAssignedRegister();


         if (lowAssignedRegister &&
             lowAssignedRegister->getRealRegister()->getState() != TR::RealRegister::Locked)
            {
            TR_ASSERT(dependentRegisterLow->getFutureUseCount() >=0,
                    "\nReg dep assignment: futureUseCount should not be negative\n");
            }
         if (lowAssignedRegister &&
             dependentRegisterLow->getFutureUseCount() == 0)
            {
            TR::RealRegister * assignedRegister = lowAssignedRegister->getRealRegister();
            dependentRegisterLow->setAssignedRegister(NULL);
            assignedRegister->setState(TR::RealRegister::Unlatched);
            if (assignedRegister->getState() == TR::RealRegister::Locked)
               assignedRegister->setAssignedRegister(NULL);
            cg->traceRegFreed(dependentRegisterLow, assignedRegister);
            }
         }
      }

   if (!comp->getOption(TR_DisableOOL))
      {
      // TODO: Is this HPR related? Do we need this code?
      for (i = 0; i < numOfDependencies; i++)
         {
         if (_dependencies[i].getRegister()->getRealRegister())
            {
            TR::RealRegister* realReg = toRealRegister(_dependencies[i].getRegister());
            if (_dependencies[i].isSpilledReg())
               {
               virtReg = realReg->getAssignedRegister();

               traceMsg(comp,"\nOOL HPR Spill: %s", cg->getDebug()->getName(realReg));
               traceMsg(comp,":%s\n", cg->getDebug()->getName(virtReg));

               virtReg->setAssignedRegister(NULL);
               }
            }
         }
      }
   }

