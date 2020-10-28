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
#include "codegen/RegisterDependencyGroup_inlines.hpp"

#include <stddef.h>
#include <stdint.h>
#include <algorithm>
#include "codegen/BackingStore.hpp"
#include "codegen/CodeGenerator.hpp"
#include "codegen/GenerateInstructions.hpp"
#include "codegen/LiveRegister.hpp"
#include "codegen/Machine.hpp"
#include "codegen/MemoryReference.hpp"
#include "codegen/RealRegister.hpp"
#include "codegen/Register.hpp"
#include "codegen/RegisterConstants.hpp"
#include "codegen/RegisterDependency.hpp"
#include "codegen/RegisterDependencyStruct.hpp"
#include "codegen/PPCInstruction.hpp"
#include "codegen/PPCOpsDefines.hpp"
#include "compile/Compilation.hpp"
#include "control/Options.hpp"
#include "control/Options_inlines.hpp"
#include "infra/Assert.hpp"
#include "ras/Debug.hpp"


static TR::RegisterDependency *
findDependencyChainHead(
      TR::RegisterDependency *dep,
      OMR::RegisterDependencyMap& map)
   {
   TR::RegisterDependency *cursor = map.getDependencyWithSourceAssigned(dep->getRealRegister());

   // Already at head of chain
   if (!cursor)
      return dep;

   // Follow the chain until we get to a dep who's target isn't assigned to another dep
   // or until we're back at the first dep
   while (cursor != dep)
      {
      TR::RegisterDependency *nextDep = map.getDependencyWithSourceAssigned(cursor->getRealRegister());
      if (!nextDep)
         break;
      cursor = nextDep;
      }

   return cursor;
   }


static void
assignFreeRegisters(
      TR::Instruction *currentInstruction,
      TR::RegisterDependency *dep,
      OMR::RegisterDependencyMap& map,
      TR::CodeGenerator *cg)
   {
   TR::Machine *machine = cg->machine();

   // Assign a chain of dependencies where the head of the chain depends on a free reg
   while (dep)
      {
      TR_ASSERT(machine->getRealRegister(dep->getRealRegister())->getState() == TR::RealRegister::Free, "Expecting free target register");
      TR::RealRegister *assignedReg = dep->getRegister()->getAssignedRealRegister() ?
         toRealRegister(dep->getRegister()->getAssignedRealRegister()) : NULL;
      machine->coerceRegisterAssignment(currentInstruction, dep->getRegister(), dep->getRealRegister());
      dep->getRegister()->block();
      dep = assignedReg ?
         map.getDependencyWithTarget(assignedReg->getRegisterNumber()) : NULL;
      }
   }


static void
assignContendedRegisters(
      TR::Instruction *currentInstruction,
      TR::RegisterDependency *dep,
      OMR::RegisterDependencyMap& map,
      bool depsBlocked,
      TR::CodeGenerator *cg)
   {
   TR::Machine *machine = cg->machine();

   dep = findDependencyChainHead(dep, map);

   TR::Register *virtReg = dep->getRegister();
   TR::RealRegister::RegNum targetRegNum = dep->getRealRegister();
   TR::RealRegister *targetReg = machine->getRealRegister(targetRegNum);
   TR::RealRegister *assignedReg = virtReg->getAssignedRealRegister() ?
      toRealRegister(virtReg->getAssignedRealRegister()) :  NULL;

   // Chain of length 1
   if (!assignedReg || !map.getDependencyWithTarget(assignedReg->getRegisterNumber()))
      {
      machine->coerceRegisterAssignment(currentInstruction, virtReg, targetRegNum);
      virtReg->block();
      return;
      }

   // Chain of length 2, handled here instead of below to get 3*xor exchange on GPRs
   if (map.getDependencyWithTarget(assignedReg->getRegisterNumber()) == map.getDependencyWithSourceAssigned(targetRegNum))
      {
      TR::Register *targetVirtReg = targetReg->getAssignedRegister();
      machine->coerceRegisterAssignment(currentInstruction, virtReg, targetRegNum);
      virtReg->block();
      targetVirtReg->block();
      return;
      }

   // Grab a spare reg in order to free the target of the first dep
   // At this point the first dep's target could be blocked, assigned, or NoReg
   // If it's blocked or assigned we allocate a spare and assign the target's virtual to it
   // If it's NoReg, the spare reg will be used as the first dep's actual target
   TR::RealRegister *spareReg = machine->findBestFreeRegister(currentInstruction, virtReg->getKind(),
                                                                targetRegNum == TR::RealRegister::NoReg ? dep->getExcludeGPR0() : false, false,
                                                                targetRegNum == TR::RealRegister::NoReg ? virtReg : targetReg->getAssignedRegister());
   bool haveFreeSpare = spareReg != NULL;
   if (!spareReg)
      {
      // If the regs in this dep group are not blocked we need to make sure we don't spill a reg that's in the middle of the chain
      if (!depsBlocked)
         {
         if (targetRegNum == TR::RealRegister::NoReg)
            spareReg = machine->freeBestRegister(currentInstruction,
                                                 map.getDependencyWithTarget(assignedReg->getRegisterNumber())->getRegister(),
                                                 assignedReg, false);
         else
            spareReg = machine->freeBestRegister(currentInstruction, virtReg, targetReg, false);
         }
      else
         {
         if (targetRegNum == TR::RealRegister::NoReg)
            spareReg = machine->freeBestRegister(currentInstruction, virtReg, NULL, dep->getExcludeGPR0());
         else
            spareReg = machine->freeBestRegister(currentInstruction, targetReg->getAssignedRegister(), NULL, false);
         }
      }

   if (targetRegNum != TR::RealRegister::NoReg && spareReg != targetReg)
      {
      machine->coerceRegisterAssignment(currentInstruction, targetReg->getAssignedRegister(), spareReg->getRegisterNumber());
      }

   TR_ASSERT(targetRegNum == TR::RealRegister::NoReg ||
          targetReg->getState() == TR::RealRegister::Free, "Expecting free target register");

   if (depsBlocked || targetRegNum != TR::RealRegister::NoReg || haveFreeSpare)
      {

      machine->coerceRegisterAssignment(currentInstruction, virtReg,
                                        targetRegNum == TR::RealRegister::NoReg ?
                                        spareReg->getRegisterNumber() : targetRegNum);
      virtReg->block();
      }

   dep = map.getDependencyWithTarget(assignedReg->getRegisterNumber());
   while (dep)
      {
      virtReg = dep->getRegister();
      targetRegNum = dep->getRealRegister();
      targetReg = machine->getRealRegister(targetRegNum);
      assignedReg = virtReg->getAssignedRealRegister() ?
         toRealRegister(virtReg->getAssignedRealRegister()) : NULL;

      TR_ASSERT(targetReg->getState() == TR::RealRegister::Free || targetReg == spareReg,
             "Expecting free target register or target to have been filled to free spare register");

      machine->coerceRegisterAssignment(currentInstruction, virtReg, targetRegNum);
      virtReg->block();
      dep = assignedReg ?
         map.getDependencyWithTarget(assignedReg->getRegisterNumber()) : NULL;
      }

   }


void
OMR::Power::RegisterDependencyGroup::assignRegisters(
      TR::Instruction *currentInstruction,
      TR_RegisterKinds kindToBeAssigned,
      uint32_t numberOfRegisters,
      TR::CodeGenerator *cg)
   {
   TR::Machine              *machine = cg->machine();
   TR::Register             *virtReg;
   TR::RealRegister::RegNum  dependentRegNum;
   TR::RealRegister         *dependentRealReg, *assignedRegister, *realReg;
   int                       i, j;
   TR::Compilation          *comp = cg->comp();

   if (!comp->getOption(TR_DisableOOL))
      {
      for (i = 0; i < numberOfRegisters; i++)
         {
         virtReg = _dependencies[i].getRegister();
         if (_dependencies[i].isSpilledReg())
            {
            TR_ASSERT(virtReg->getBackingStorage(), "Should have a backing store if SpilledReg");
            if (virtReg->getAssignedRealRegister())
               {
               // this happens when the register was first spilled in main line path then was reverse spilled
               // and assigned to a real register in OOL path. We protected the backing store when doing
               // the reverse spill so we could re-spill to the same slot now
               if (comp->getOption(TR_TraceCG))
                  traceMsg(comp,"\nOOL: Found register spilled in main line and re-assigned inside OOL");
               TR::Node            *currentNode = currentInstruction->getNode();
               TR::RealRegister    *assignedReg = toRealRegister(virtReg->getAssignedRegister());
               TR::MemoryReference *tempMR = TR::MemoryReference::createWithSymRef(cg, currentNode, (TR::SymbolReference*)virtReg->getBackingStorage()->getSymbolReference(), sizeof(uintptr_t));
               TR_RegisterKinds     rk = virtReg->getKind();

               TR::InstOpCode::Mnemonic opCode;
               switch (rk)
                  {
                  case TR_GPR:
                     opCode =TR::InstOpCode::Op_load;
                     break;
                  case TR_FPR:
                     opCode = virtReg->isSinglePrecision() ? TR::InstOpCode::lfs : TR::InstOpCode::lfd;
                     break;
                  default:
                     TR_ASSERT(0, "Register kind not supported in OOL spill");
                     break;
                  }

               TR::Instruction *inst = generateTrg1MemInstruction(cg, opCode, currentNode, assignedReg, tempMR, currentInstruction);

               assignedReg->setAssignedRegister(NULL);
               virtReg->setAssignedRegister(NULL);
               assignedReg->setState(TR::RealRegister::Free);
               if (comp->getDebug())
                  cg->traceRegisterAssignment("Generate reload of virt %s due to spillRegIndex dep at inst %p\n", comp->getDebug()->getName(virtReg),currentInstruction);
               cg->traceRAInstruction(inst);
               }

            if (!(std::find(cg->getSpilledRegisterList()->begin(), cg->getSpilledRegisterList()->end(), virtReg) != cg->getSpilledRegisterList()->end()))
               cg->getSpilledRegisterList()->push_front(virtReg);
            }
         }
      }

   uint32_t numGPRs = 0;
   uint32_t numFPRs = 0;
   uint32_t numVRFs = 0;
   bool haveSpilledCCRs = false;

   // Used to do lookups using real register numbers
   OMR::RegisterDependencyMap map(_dependencies, numberOfRegisters);

   for (i = 0; i < numberOfRegisters; i++)
      {
      map.addDependency(_dependencies[i], i);

      if (!_dependencies[i].isSpilledReg())
         {
         virtReg = _dependencies[i].getRegister();
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
            case TR_CCR:
               if (virtReg->getAssignedRegister() == NULL &&
                   virtReg->getBackingStorage() == NULL)
                  haveSpilledCCRs = true;
               break;
            default:
               break;
            }
         }
      }

#ifdef DEBUG
   int lockedGPRs = 0;
   int lockedFPRs = 0;
   int lockedVRFs = 0;

   // count up how many registers are locked for each type
   for(i = TR::RealRegister::FirstGPR; i <= TR::RealRegister::LastGPR; i++)
      {
        realReg = machine->getRealRegister((TR::RealRegister::RegNum)i);
        if (realReg->getState() == TR::RealRegister::Locked)
           lockedGPRs++;
      }
   for(i = TR::RealRegister::FirstFPR; i <= TR::RealRegister::LastFPR; i++)
      {
        realReg = machine->getRealRegister((TR::RealRegister::RegNum)i);
        if (realReg->getState() == TR::RealRegister::Locked)
           lockedFPRs++;
      }
   for(i = TR::RealRegister::FirstVRF; i <= TR::RealRegister::LastVRF; i++)
      {
        realReg = machine->getRealRegister((TR::RealRegister::RegNum)i);
        if (realReg->getState() == TR::RealRegister::Locked)
           lockedVRFs++;
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

   TR_ASSERT(numGPRs <= (TR::RealRegister::LastGPR - TR::RealRegister::FirstGPR + 1 - machine->getNumberOfLockedRegisters(TR_GPR)), "Too many GPR dependencies, unable to assign" );
   TR_ASSERT(numFPRs <= (TR::RealRegister::LastFPR - TR::RealRegister::FirstFPR + 1 - machine->getNumberOfLockedRegisters(TR_FPR)), "Too many FPR dependencies, unable to assign" );
   TR_ASSERT(numVRFs <= (TR::RealRegister::LastVRF - TR::RealRegister::FirstVRF + 1 - machine->getNumberOfLockedRegisters(TR_VRF)), "Too many VRF dependencies, unable to assign" );

   if (numGPRs == (TR::RealRegister::LastGPR - TR::RealRegister::FirstGPR + 1 - machine->getNumberOfLockedRegisters(TR_GPR)))
      haveSpareGPRs = false;
   if (numFPRs == (TR::RealRegister::LastFPR - TR::RealRegister::FirstFPR + 1 - machine->getNumberOfLockedRegisters(TR_FPR)))
      haveSpareFPRs = false;
   if (numVRFs == (TR::RealRegister::LastVRF - TR::RealRegister::FirstVRF + 1 - machine->getNumberOfLockedRegisters(TR_VRF)))
      haveSpareVRFs = false;

   for (i = 0; i < numberOfRegisters; i++)
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
      }

   // If spilled CCRs are present in the dependencies and we are not blocking GPRs
   // (because we may not have any spare ones) we will assign the CCRs first.
   // This is because in order to assign a spilled virtual to a physical CCR
   // we need to reverse spill it to a GPR first and then assign it to the
   // CCR, so we do that first to avoid having all the GPRs become assigned and blocked.
   // Note: It is safe to assign CCRs before GPRs in any circumstance, but the extra
   // checks are cheap and let us avoid an extra loop through the dependencies for
   // what should be a relatively infrequent situation anyway.
   if (haveSpilledCCRs && !haveSpareGPRs)
      {
      for (i = 0; i < numberOfRegisters; i++)
         {
         TR::RegisterDependency &regDep = _dependencies[i];
         virtReg = regDep.getRegister();
         if (virtReg->getKind() != TR_CCR)
            continue;
         dependentRegNum = regDep.getRealRegister();
         dependentRealReg = machine->getRealRegister(dependentRegNum);

         if (!regDep.isNoReg() &&
             !regDep.isSpilledReg() &&
             dependentRealReg->getState() == TR::RealRegister::Free)
            {
            assignFreeRegisters(currentInstruction, &regDep, map, cg);
            }
         }
      }

   // Assign all virtual regs that depend on a specific real reg that is free
   for (i = 0; i < numberOfRegisters; i++)
      {
      TR::RegisterDependency &regDep = _dependencies[i];
      virtReg = regDep.getRegister();
      dependentRegNum = regDep.getRealRegister();
      dependentRealReg = machine->getRealRegister(dependentRegNum);

      if (!regDep.isNoReg() &&
          !regDep.isSpilledReg() &&
          dependentRealReg->getState() == TR::RealRegister::Free)
         {
         assignFreeRegisters(currentInstruction, &regDep, map, cg);
         }
      }

   // Assign all virtual regs that depend on a specific real reg that is not free
   for (i = 0; i < numberOfRegisters; i++)
      {
      virtReg          = _dependencies[i].getRegister();
      assignedRegister = NULL;
      if (virtReg->getAssignedRealRegister() != NULL)
         {
         assignedRegister = toRealRegister(virtReg->getAssignedRealRegister());
         }
      dependentRegNum  = _dependencies[i].getRealRegister();
      dependentRealReg = machine->getRealRegister(dependentRegNum);
      if (!_dependencies[i].isNoReg() &&
          !_dependencies[i].isSpilledReg() &&
          dependentRealReg != assignedRegister)
         {
         bool depsBlocked = false;
         switch (_dependencies[i].getRegister()->getKind())
            {
            case TR_GPR:
               depsBlocked = haveSpareGPRs;
               break;
            case TR_FPR:
               depsBlocked = haveSpareFPRs;
               break;
            case TR_VRF:
               depsBlocked = haveSpareVRFs;
               break;
            }
         assignContendedRegisters(currentInstruction, &_dependencies[i], map, depsBlocked, cg);
         }
      }

   // Assign all virtual regs that depend on NoReg but exclude gr0
   for (i = 0; i < numberOfRegisters; i++)
      {
      if (_dependencies[i].isNoReg() && _dependencies[i].getExcludeGPR0())
         {
         TR::RealRegister *realOne;

         virtReg = _dependencies[i].getRegister();
         realOne = virtReg->getAssignedRealRegister();
         if (realOne != NULL && toRealRegister(realOne)->getRegisterNumber() == TR::RealRegister::gr0)
            {
            if ((assignedRegister = machine->findBestFreeRegister(currentInstruction, virtReg->getKind(), true, false, virtReg)) == NULL)
               {
               assignedRegister = machine->freeBestRegister(currentInstruction, virtReg, NULL, true);
               }
            machine->coerceRegisterAssignment(currentInstruction, virtReg, assignedRegister->getRegisterNumber());
            }
         else if (realOne == NULL)
            {
            machine->assignOneRegister(currentInstruction, virtReg, true);
            }
         virtReg->block();
         }
      }

   // Assign all virtual regs that depend on NoReg
   for (i = 0; i < numberOfRegisters; i++)
      {
      if (_dependencies[i].isNoReg() && !_dependencies[i].getExcludeGPR0())
         {
         TR::RealRegister *realOne;

         virtReg = _dependencies[i].getRegister();
         realOne = virtReg->getAssignedRealRegister();
         if (!realOne)
            {
            machine->assignOneRegister(currentInstruction, virtReg, false);
            }
         virtReg->block();
         }
      }

   self()->unblockRegisters(numberOfRegisters);
   for (i = 0; i < numberOfRegisters; i++)
      {
      TR::Register *dependentRegister = self()->getRegisterDependency(i)->getRegister();
      // dependentRegister->getAssignedRegister() is NULL if the reg has already been spilled due to a spilledReg dep
      if (comp->getOption(TR_DisableOOL) || (!(cg->isOutOfLineColdPath()) && !(cg->isOutOfLineHotPath())))
         {
         TR_ASSERT(dependentRegister->getAssignedRegister(), "Assigned register can not be NULL");
         }
      if (dependentRegister->getAssignedRegister())
         {
         TR::RealRegister *assignedRegister = dependentRegister->getAssignedRegister()->getRealRegister();

         if (self()->getRegisterDependency(i)->isNoReg())
            self()->getRegisterDependency(i)->setRealRegister(toRealRegister(assignedRegister)->getRegisterNumber());

         machine->decFutureUseCountAndUnlatch(dependentRegister);
         }
      }
   }


void
OMR::Power::RegisterDependencyGroup::stopUsingDepRegs(
      uint32_t numberOfRegisters,
      int numRetReg,
      TR::Register **retReg,
      TR::CodeGenerator *cg)
   {
   for (uint32_t i = 0; i < numberOfRegisters; i++)
      {
      TR::Register *depReg = _dependencies[i].getRegister();
      bool found = false;
      for (int j = 0; j < numRetReg; j++)
         if (depReg == retReg[j])
            found = true;
      if (!found)
         cg->stopUsingRegister(depReg);
      }
   }


void
OMR::Power::RegisterDependencyGroup::stopUsingDepRegs(
      uint32_t numberOfRegisters,
      TR::Register *ret1,
      TR::Register *ret2,
      TR::CodeGenerator *cg)
   {
   TR::Register* retRegs[2] = {ret1, ret2};
   self()->stopUsingDepRegs(numberOfRegisters, 2, retRegs, cg);
   }


