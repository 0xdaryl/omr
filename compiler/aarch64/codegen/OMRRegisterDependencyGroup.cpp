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
#include "codegen/Machine.hpp"
#include "codegen/MemoryReference.hpp"
#include "codegen/RealRegister.hpp"
#include "codegen/Register.hpp"
#include "codegen/RegisterConstants.hpp"
#include "codegen/RegisterDependency.hpp"
#include "codegen/RegisterDependencyStruct.hpp"
#include "compile/Compilation.hpp"
#include "control/Options.hpp"
#include "control/Options_inlines.hpp"
#include "il/Node.hpp"
#include "infra/Assert.hpp"
#include "ras/Debug.hpp"


template<class It>
void OMR::ARM64::RegisterDependencyGroup::stopUsingDepRegs(
      uint32_t numberOfRegisters,
      It skipRegsBegin,
      It skipRegsEnd,
      TR::CodeGenerator *cg)
   {
   for (uint32_t i = 0; i < numberOfRegisters; i++)
      {
      TR::Register *depReg = _dependencies[i].getRegister();
      if (depReg && (std::find(skipRegsBegin, skipRegsEnd, depReg) == skipRegsEnd))
         cg->stopUsingRegister(depReg);
      }
   }


/**
 * @brief Kills registers held by this dependency group
 * @param[in] numberOfRegisters : # of registers
 * @param[in] returnRegister    : register which is not killed
 * @param[in] cg                : CodeGenerator
 */
void
OMR::ARM64::RegisterDependencyGroup::stopUsingDepRegs(
      uint32_t numberOfRegisters,
      TR::Register *returnRegister,
      TR::CodeGenerator *cg)
   {
   for (uint32_t i = 0; i < numberOfRegisters; i++)
      {
      TR::Register *depReg = _dependencies[i].getRegister();
      if (depReg && (depReg != returnRegister))
         cg->stopUsingRegister(depReg);
      }
   }



void
OMR::ARM64::RegisterDependencyGroup::assignRegisters(
      TR::Instruction *currentInstruction,
      TR_RegisterKinds kindToBeAssigned,
      uint32_t numberOfRegisters,
      TR::CodeGenerator *cg)
   {
   TR::Compilation *comp = cg->comp();
   TR::Machine *machine = cg->machine();
   TR::Register  *virtReg;
   TR::RealRegister::RegNum dependentRegNum;
   TR::RealRegister *dependentRealReg, *assignedRegister;
   uint32_t i, j;
   bool changed;

   if (!comp->getOption(TR_DisableOOL))
      {
      for (i = 0; i< numberOfRegisters; i++)
         {
         virtReg = _dependencies[i].getRegister();
         if (_dependencies[i].isSpilledReg())
            {
            TR_ASSERT(virtReg->getBackingStorage(), "should have a backing store if SpilledReg");
            if (virtReg->getAssignedRealRegister())
               {
               // this happens when the register was first spilled in main line path then was reverse spilled
               // and assigned to a real register in OOL path. We protected the backing store when doing
               // the reverse spill so we could re-spill to the same slot now
               traceMsg (comp,"\nOOL: Found register spilled in main line and re-assigned inside OOL");
               TR::Node *currentNode = currentInstruction->getNode();
               TR::RealRegister *assignedReg = toRealRegister(virtReg->getAssignedRegister());
               TR::MemoryReference *tempMR = new (cg->trHeapMemory()) TR::MemoryReference(currentNode, (TR::SymbolReference*)virtReg->getBackingStorage()->getSymbolReference(), cg);
               TR_RegisterKinds rk = virtReg->getKind();
               TR::InstOpCode::Mnemonic opCode;
               switch (rk)
                  {
                  case TR_GPR:
                     opCode = TR::InstOpCode::ldrimmx;
                     break;
                  case TR_FPR:
                     opCode = TR::InstOpCode::vldrimmd;
                     break;
                  default:
                     TR_ASSERT(0, "\nRegister kind not supported in OOL spill\n");
                     break;
                  }

               TR::Instruction *inst = generateTrg1MemInstruction(cg, opCode, currentNode, assignedReg, tempMR, currentInstruction);

               assignedReg->setAssignedRegister(NULL);
               virtReg->setAssignedRegister(NULL);
               assignedReg->setState(TR::RealRegister::Free);

               if (comp->getDebug())
                  cg->traceRegisterAssignment("Generate reload of virt %s due to spillRegIndex dep at inst %p\n", cg->comp()->getDebug()->getName(virtReg), currentInstruction);
               cg->traceRAInstruction(inst);
               }

            if (!(std::find(cg->getSpilledRegisterList()->begin(), cg->getSpilledRegisterList()->end(), virtReg) != cg->getSpilledRegisterList()->end()))
               cg->getSpilledRegisterList()->push_front(virtReg);
            }
         // we also need to free up all locked backing storage if we are exiting the OOL during backwards RA assignment
         else if (currentInstruction->isLabel() && virtReg->getAssignedRealRegister())
            {
            TR::ARM64LabelInstruction *labelInstr = (TR::ARM64LabelInstruction *)currentInstruction;
            TR_BackingStore *location = virtReg->getBackingStorage();
            TR_RegisterKinds rk = virtReg->getKind();
            int32_t dataSize;
            if (labelInstr->getLabelSymbol()->isStartOfColdInstructionStream() && location)
               {
               traceMsg (comp,"\nOOL: Releasing backing storage (%p)\n", location);
               if (rk == TR_GPR)
                  dataSize = TR::Compiler->om.sizeofReferenceAddress();
               else
                  dataSize = 8;
               location->setMaxSpillDepth(0);
               cg->freeSpill(location, dataSize, 0);
               virtReg->setBackingStorage(NULL);
               }
            }
         }
      }

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
            dependentRegNum = toRealRegister(virtReg->getAssignedRealRegister())->getRegisterNumber();
            for (j = 0; j < numberOfRegisters; j++)
               {
               if (dependentRegNum == _dependencies[j].getRealRegister())
                  {
                  virtReg->block();
                  break;
                  }
               }
            }
         }
      }

   do
      {
      changed = false;
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
            machine->coerceRegisterAssignment(currentInstruction, virtReg, dependentRegNum);
            virtReg->block();
            changed = true;
            }
         }
      } while (changed == true);

   do
      {
      changed = false;
      for (i = 0; i < numberOfRegisters; i++)
         {
         TR::RegisterDependency &regDep = _dependencies[i];
         virtReg = regDep.getRegister();
         assignedRegister = NULL;
         if (virtReg->getAssignedRealRegister() != NULL)
            {
            assignedRegister = toRealRegister(virtReg->getAssignedRealRegister());
            }
         dependentRegNum = regDep.getRealRegister();
         dependentRealReg = machine->getRealRegister(dependentRegNum);
         if (!regDep.isNoReg() &&
             !regDep.isSpilledReg() &&
             dependentRealReg != assignedRegister)
            {
            machine->coerceRegisterAssignment(currentInstruction, virtReg, dependentRegNum);
            virtReg->block();
            changed = true;
            }
         }
      } while (changed == true);

   for (i = 0; i < numberOfRegisters; i++)
      {
      if (_dependencies[i].isNoReg())
         {
         TR::RealRegister *realOne;

         virtReg = _dependencies[i].getRegister();
         realOne = virtReg->getAssignedRealRegister();
         if (realOne == NULL)
            {
            if (virtReg->getTotalUseCount() == virtReg->getFutureUseCount())
               {
               if ((assignedRegister = machine->findBestFreeRegister(currentInstruction, virtReg->getKind(), true, virtReg)) == NULL)
                  {
                  assignedRegister = machine->freeBestRegister(currentInstruction, virtReg, NULL);
                  }
               }
            else
               {
               assignedRegister = machine->reverseSpillState(currentInstruction, virtReg, NULL);
               }
            virtReg->setAssignedRegister(assignedRegister);
            assignedRegister->setAssignedRegister(virtReg);
            assignedRegister->setState(TR::RealRegister::Assigned);
            virtReg->block();
            }
         }
      }

   self()->unblockRegisters(numberOfRegisters);
   for (i = 0; i < numberOfRegisters; i++)
      {
      TR::RegisterDependency *regDep = self()->getRegisterDependency(i);
      TR::Register *dependentRegister = regDep->getRegister();
      if (dependentRegister->getAssignedRegister())
         {
         TR::RealRegister *assignedRegister = dependentRegister->getAssignedRegister()->getRealRegister();

         if (regDep->isNoReg())
            regDep->setRealRegister(toRealRegister(assignedRegister)->getRegisterNumber());

         machine->decFutureUseCountAndUnlatch(currentInstruction, dependentRegister);
         }
      else if (regDep->isSpilledReg())
         {
         machine->decFutureUseCountAndUnlatch(currentInstruction, dependentRegister);
         }
      }
   }
