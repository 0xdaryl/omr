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
#include "codegen/BackingStore.hpp"
#include "codegen/CodeGenerator.hpp"
#include "codegen/GenerateInstructions.hpp"
#include "codegen/Machine.hpp"
#include "codegen/RealRegister.hpp"
#include "codegen/Register.hpp"
#include "codegen/RegisterDependency.hpp"
#include "codegen/RegisterDependencyGroup.hpp"
#include "codegen/RegisterPair.hpp"
#include "env/CompilerEnv.hpp"
#include "il/Node.hpp"
#include "il/Node_inlines.hpp"

OMR::ARM::RegisterDependencyConditions::RegisterDependencyConditions( TR::Node          *node,
                                                                       uint32_t           extranum,
                                                                       TR::Instruction   **cursorPtr,
                                                                       TR::CodeGenerator  *cg)
   {

   List<TR::Register>  regList(cg->trMemory());
   TR::Instruction    *cursor = (cursorPtr == NULL ? NULL : *cursorPtr);
   int32_t totalNum = node->getNumChildren() + extranum;
   int32_t i;


   cg->comp()->incVisitCount();

   int32_t numLongs = 0;
   //
   // Pre-compute how many longs are global register candidates
   //
   for (i = 0; i < node->getNumChildren(); ++i)
      {
      TR::Node     *child = node->getChild(i);
      TR::Register *reg   = child->getRegister();

      if (reg!=NULL /* && reg->getKind()==TR_GPR */)
         {
         if (child->getHighGlobalRegisterNumber() > -1)
            numLongs++;
         }
      }

   totalNum = totalNum + numLongs;

   _preConditions = TR::RegisterDependencyGroup::create(totalNum, cg->trMemory());
   _postConditions = TR::RegisterDependencyGroup::create(totalNum, cg->trMemory());
   _numPreConditions = totalNum;
   _addCursorForPre = 0;
   _numPostConditions = totalNum;
   _addCursorForPost = 0;

   // First, handle dependencies that match current association
   for (i=0; i<node->getNumChildren(); i++)
      {
      TR::Node *child = node->getChild(i);
      TR::Register *reg = child->getRegister();
      TR::Register *highReg = NULL;
      TR::RealRegister::RegNum regNum = (TR::RealRegister::RegNum)cg->getGlobalRegister(child->getGlobalRegisterNumber());

      TR::RealRegister::RegNum highRegNum;

      if (child->getHighGlobalRegisterNumber() > -1)
         {
         highRegNum = (TR::RealRegister::RegNum)cg->getGlobalRegister(child->getHighGlobalRegisterNumber());

         TR::RegisterPair *regPair = reg->getRegisterPair();
         TR_ASSERT(regPair, "assertion failure");
         highReg = regPair->getHighOrder();
         reg = regPair->getLowOrder();

         if (highReg->getAssociation() != highRegNum ||
            reg->getAssociation() != regNum)
            continue;
         }
      else if (reg->getAssociation() != regNum)
         continue;

      TR_ASSERT(!regList.find(reg) && (!highReg || !regList.find(highReg)), "Should not happen\n");

      addPreCondition(reg, regNum);
      addPostCondition(reg, regNum);
      regList.add(reg);

      if (highReg)
         {
         addPreCondition(highReg, highRegNum);
         addPostCondition(highReg, highRegNum);
         regList.add(highReg);
         }
      }


   // Second pass to handle dependencies for which association does not exist
   // or does not match
   for (i=0; i<node->getNumChildren(); i++)
      {
      TR::Node *child = node->getChild(i);
      TR::Register *reg = child->getRegister();
      TR::Register *highReg = NULL;
      TR::Register *copyReg = NULL;
      TR::Register *highCopyReg = NULL;
      TR::RealRegister::RegNum regNum = (TR::RealRegister::RegNum)cg->getGlobalRegister(child->getGlobalRegisterNumber());

      TR::RealRegister::RegNum highRegNum;

      if (child->getHighGlobalRegisterNumber() > -1)
         {
         highRegNum = (TR::RealRegister::RegNum)cg->getGlobalRegister(child->getHighGlobalRegisterNumber());
         TR::RegisterPair *regPair = reg->getRegisterPair();
         TR_ASSERT(regPair, "assertion failure");
         highReg = regPair->getHighOrder();
         reg = regPair->getLowOrder();

         if (highReg->getAssociation() == highRegNum &&
            reg->getAssociation() == regNum)
            continue;
         }
      else if (reg->getAssociation() == regNum)
         continue;

      if (regList.find(reg) || (highReg && regList.find(highReg)))
         {
         TR_ARMOpCodes    opCode;
         TR_RegisterKinds kind = reg->getKind();

         switch (kind)
            {
            case TR_GPR:
               opCode = ARMOp_mov;
               break;
            case TR_FPR:
               opCode = ARMOp_fmrs;
               break;
            default:
               TR_ASSERT(0, "Invalid register kind.");
            }

         if (regList.find(reg))
            {
            bool containsInternalPointer = false;
            if (reg->getPinningArrayPointer())
               containsInternalPointer = true;

            copyReg = (reg->containsCollectedReference() && !containsInternalPointer) ?
               cg->allocateCollectedReferenceRegister() : cg->allocateRegister(kind);

            if (containsInternalPointer)
               {
               copyReg->setContainsInternalPointer();
               copyReg->setPinningArrayPointer(reg->getPinningArrayPointer());
               }

            cursor = generateTrg1Src1Instruction(cg, opCode, node, copyReg, reg, cursor);
            reg = copyReg;
            }

         if (highReg && regList.find(highReg))
            {
            bool containsInternalPointer = false;
            if (highReg->getPinningArrayPointer())
               containsInternalPointer = true;

            highCopyReg = (highReg->containsCollectedReference() && !containsInternalPointer) ?
               cg->allocateCollectedReferenceRegister() : cg->allocateRegister(kind);

            if (containsInternalPointer)
               {
               highCopyReg->setContainsInternalPointer();
               highCopyReg->setPinningArrayPointer(highReg->getPinningArrayPointer());
               }

            cursor = generateTrg1Src1Instruction(cg, opCode, node, highCopyReg, highReg, cursor);
            highReg = highCopyReg;
            }
         }

      reg->setAssociation(regNum);
      addPreCondition(reg, regNum);
      addPostCondition(reg, regNum);
      if (copyReg != NULL)
         cg->stopUsingRegister(copyReg);
      else
         regList.add(reg);

      if (highReg)
         {
         highReg->setAssociation(highRegNum);
         addPreCondition(highReg, highRegNum);
         addPostCondition(highReg, highRegNum);
         if (highCopyReg != NULL)
            cg->stopUsingRegister(highCopyReg);
         else
            regList.add(highReg);
         }
      }

   if (cursor != NULL && cursorPtr != NULL)
      *cursorPtr = cursor;
   }

bool OMR::ARM::RegisterDependencyConditions::refsRegister(TR::Register *r)
   {
   for (int i = 0; i < _addCursorForPre; i++)
      {
      if (_preConditions->getRegisterDependency(i)->getRegister() == r &&
          _preConditions->getRegisterDependency(i)->getRefsRegister())
         {
         return true;
         }
      }
   for (int j = 0; j < _addCursorForPost; j++)
      {
      if (_postConditions->getRegisterDependency(j)->getRegister() == r &&
          _postConditions->getRegisterDependency(j)->getRefsRegister())
         {
         return true;
         }
      }
   return false;
   }

bool OMR::ARM::RegisterDependencyConditions::defsRegister(TR::Register *r)
   {
   for (int i = 0; i < _addCursorForPre; i++)
      {
      if (_preConditions->getRegisterDependency(i)->getRegister() == r &&
          _preConditions->getRegisterDependency(i)->getDefsRegister())
         {
         return true;
         }
      }
   for (int j = 0; j < _addCursorForPost; j++)
      {
      if (_postConditions->getRegisterDependency(j)->getRegister() == r &&
          _postConditions->getRegisterDependency(j)->getDefsRegister())
         {
         return true;
         }
      }
   return false;
   }

bool OMR::ARM::RegisterDependencyConditions::defsRealRegister(TR::Register *r)
   {
   for (int i = 0; i < _addCursorForPre; i++)
      {
      if (_preConditions->getRegisterDependency(i)->getRegister()->getAssignedRegister() == r &&
          _preConditions->getRegisterDependency(i)->getDefsRegister())
         {
         return true;
         }
      }
   for (int j = 0; j < _addCursorForPost; j++)
      {
      if (_postConditions->getRegisterDependency(j)->getRegister()->getAssignedRegister() == r &&
          _postConditions->getRegisterDependency(j)->getDefsRegister())
         {
         return true;
         }
      }
   return false;
   }


bool OMR::ARM::RegisterDependencyConditions::usesRegister(TR::Register *r)
   {
   for (int i = 0; i < _addCursorForPre; i++)
      {
      if (_preConditions->getRegisterDependency(i)->getRegister() == r &&
          _preConditions->getRegisterDependency(i)->getUsesRegister())
         {
         return true;
         }
      }
   for (int j = 0; j < _addCursorForPost; j++)
      {
      if (_postConditions->getRegisterDependency(j)->getRegister() == r &&
          _postConditions->getRegisterDependency(j)->getUsesRegister())
         {
         return true;
         }
      }
   return false;
   }

void OMR::ARM::RegisterDependencyConditions::incRegisterTotalUseCounts(TR::CodeGenerator * cg)
   {
   for (int i = 0; i < _addCursorForPre; i++)
      {
      _preConditions->getRegisterDependency(i)->getRegister()->incTotalUseCount();
      }
   for (int j = 0; j < _addCursorForPost; j++)
      {
      _postConditions->getRegisterDependency(j)->getRegister()->incTotalUseCount();
      }
   }


TR::RegisterDependencyConditions *OMR::ARM::RegisterDependencyConditions::clone(
   TR::CodeGenerator * cg, TR::RegisterDependencyConditions *added)
   {
   TR::RegisterDependencyConditions *result;
   TR::RegisterDependency *singlePair;
   int32_t      idx, preNum, postNum, addPre=0, addPost=0;
#if defined(DEBUG)
   int32_t      preGPR=0, postGPR=0;
#endif

   if (added != NULL)
      {
      addPre = added->getAddCursorForPre();
      addPost = added->getAddCursorForPost();
      }
   preNum = this->getAddCursorForPre();
   postNum = this->getAddCursorForPost();
   result = new (cg->trHeapMemory()) TR::RegisterDependencyConditions(preNum+addPre,
      postNum+addPost, cg->trMemory());

   for (idx=0; idx<postNum; idx++)
      {
      singlePair = this->getPostConditions()->getRegisterDependency(idx);
      result->addPostCondition(singlePair->getRegister(), singlePair->getRealRegister(),
                               singlePair->getFlags());
#if defined(DEBUG)
      if (singlePair->getRegister()->getKind() == TR_GPR)
         postGPR++;
#endif
      }

   for (idx=0; idx<preNum; idx++)
      {
      singlePair = this->getPreConditions()->getRegisterDependency(idx);
      result->addPreCondition(singlePair->getRegister(), singlePair->getRealRegister(),
                              singlePair->getFlags());
#if defined(DEBUG)
      if (singlePair->getRegister()->getKind() == TR_GPR)
         preGPR++;
#endif
      }

   for (idx=0; idx<addPost; idx++)
      {
      singlePair = added->getPostConditions()->getRegisterDependency(idx);
      result->addPostCondition(singlePair->getRegister(), singlePair->getRealRegister(),
                               singlePair->getFlags());
#if defined(DEBUG)
      if (singlePair->getRegister()->getKind() == TR_GPR)
         postGPR++;
#endif
      }

   for (idx=0; idx<addPre; idx++)
      {
      singlePair = added->getPreConditions()->getRegisterDependency(idx);
      result->addPreCondition(singlePair->getRegister(), singlePair->getRealRegister(),
                              singlePair->getFlags());
#if defined(DEBUG)
      if (singlePair->getRegister()->getKind() == TR_GPR)
         preGPR++;
#endif
      }

#if defined(DEBUG)
   int32_t max_gpr = cg->getProperties().getNumAllocatableIntegerRegisters();
   TR_ASSERT(preGPR<=max_gpr && postGPR<=max_gpr, "Over the limit of available global registers.");
#endif
   return result;
   }

TR::RegisterDependencyConditions *OMR::ARM::RegisterDependencyConditions::cloneAndFix(
   TR::CodeGenerator * cg, TR::RegisterDependencyConditions *added)
   {
   TR::RegisterDependencyConditions *result;
   TR::RegisterDependency *singlePair;
   int32_t      idx, preNum, postNum, addPre=0, addPost=0;
   TR::Register *postReg, *tempReg;
   TR::RealRegister::RegNum rnum;

   if (added != NULL)
      {
      addPre = added->getAddCursorForPre();
      addPost = added->getAddCursorForPost();
      }
   preNum = this->getAddCursorForPre();
   postNum = this->getAddCursorForPost();
   result = new (cg->trHeapMemory()) TR::RegisterDependencyConditions(preNum + addPre, postNum + addPost, cg->trMemory());

   for (idx=0; idx<postNum; idx++)
      {
      singlePair = this->getPostConditions()->getRegisterDependency(idx);
      rnum       = singlePair->getRealRegister();
      tempReg    = singlePair->getRegister();
      result->addPostCondition(tempReg, rnum, singlePair->getFlags());
      if (rnum == TR::RealRegister::gr0)
         {
         postReg = tempReg;
         }
      }

   for (idx=0; idx<preNum; idx++)
      {
      singlePair = this->getPreConditions()->getRegisterDependency(idx);
      rnum       = singlePair->getRealRegister();
      tempReg    = singlePair->getRegister();
      if (rnum == TR::RealRegister::gr0)
         tempReg = postReg;
      result->addPreCondition(tempReg, rnum, singlePair->getFlags());
      }

   for (idx=0; idx<addPost; idx++)
      {
      singlePair = added->getPostConditions()->getRegisterDependency(idx);
      result->addPostCondition(singlePair->getRegister(), singlePair->getRealRegister(),
                               singlePair->getFlags());
      }

   for (idx=0; idx<addPre; idx++)
      {
      singlePair = added->getPreConditions()->getRegisterDependency(idx);
      result->addPreCondition(singlePair->getRegister(), singlePair->getRealRegister(),
                              singlePair->getFlags());
      }
   return result;
   }

void OMR::ARM::RegisterDependencyConditions::unionNoRegPostCondition(TR::Register *reg, TR::CodeGenerator *cg)
   {
   addPostCondition(reg, TR::RealRegister::NoReg);
   }
