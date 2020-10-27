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

#ifndef OMR_X86_REGISTER_DEPENDENCY_INCL
#define OMR_X86_REGISTER_DEPENDENCY_INCL

/*
 * The following #define and typedef must appear before any #includes in this file
 */
#ifndef OMR_REGISTER_DEPENDENCY_CONNECTOR
#define OMR_REGISTER_DEPENDENCY_CONNECTOR
   namespace OMR { namespace X86 { class RegisterDependencyConditions; } }
   namespace OMR { typedef OMR::X86::RegisterDependencyConditions RegisterDependencyConditionsConnector; }
#else
   #error OMR::X86::RegisterDependencyConditions expected to be a primary connector, but a OMR connector is already defined
#endif

#include "compiler/codegen/OMRRegisterDependency.hpp"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "codegen/CodeGenerator.hpp"
#include "codegen/RealRegister.hpp"
#include "codegen/Register.hpp"
#include "codegen/RegisterConstants.hpp"
#include "codegen/RegisterDependencyGroup.hpp"
#include "codegen/RegisterDependencyStruct.hpp"
#include "env/TRMemory.hpp"
#include "infra/Assert.hpp"

namespace TR { class Instruction; }
namespace TR { class Node; }
namespace TR { class RegisterDependencyConditions; }
template <typename ListKind> class List;

#define UsesGlobalDependentFPRegister (UsesDependentRegister | GlobalRegisterFPDependency)
#define NUM_DEFAULT_DEPENDENCIES 1

typedef uint16_t depsize_t;

typedef TR::RegisterDependencyGroup TR_X86RegisterDependencyGroup;


namespace OMR
{
namespace X86
{
class RegisterDependencyConditions: public OMR::RegisterDependencyConditions
   {
   TR::RegisterDependencyGroup *_preConditions;
   TR::RegisterDependencyGroup *_postConditions;
   uint32_t  _numPreConditions;
   uint32_t  _addCursorForPre;
   uint32_t  _numPostConditions;
   uint32_t  _addCursorForPost;

   uint32_t unionDependencies(TR::RegisterDependencyGroup *deps, uint32_t cursor, TR::Register *vr, TR::RealRegister::RegNum rr, TR::CodeGenerator *cg, uint8_t flag, bool isAssocRegDependency);
   uint32_t unionRealDependencies(TR::RegisterDependencyGroup *deps, uint32_t cursor, TR::Register *vr, TR::RealRegister::RegNum rr, TR::CodeGenerator *cg, uint8_t flag, bool isAssocRegDependency);

   public:

   TR_ALLOC(TR_Memory::RegisterDependencyConditions)

   RegisterDependencyConditions()
      : _preConditions(NULL),
        _postConditions(NULL),
        _numPreConditions(0),
        _addCursorForPre(0),
        _numPostConditions(0),
        _addCursorForPost(0)
      {}

   RegisterDependencyConditions(uint32_t numPreConds, uint32_t numPostConds, TR_Memory * m)
      : _preConditions(TR::RegisterDependencyGroup::create(numPreConds, m)),
        _postConditions(TR::RegisterDependencyGroup::create(numPostConds, m)),
        _numPreConditions(numPreConds),
        _addCursorForPre(0),
        _numPostConditions(numPostConds),
        _addCursorForPost(0)
      {}

   RegisterDependencyConditions(TR::Node *node, TR::CodeGenerator *cg, uint32_t additionalRegDeps = 0, List<TR::Register> * = 0);

   void unionNoRegPostCondition(TR::Register *reg, TR::CodeGenerator *cg);

   TR::RegisterDependencyConditions  *clone(TR::CodeGenerator *cg, uint32_t additionalRegDeps = 0);

   TR::RegisterDependencyGroup *getPreConditions()  {return _preConditions;}

   void setMayNeedToPopFPRegisters(bool b)
      {
      if (_preConditions)
         _preConditions->setMayNeedToPopFPRegisters(b);
      if (_postConditions)
         _postConditions->setMayNeedToPopFPRegisters(b);
      }

   void setNeedToClearFPStack(bool b)
      {
      if (_preConditions)
         _preConditions->setNeedToClearFPStack(b);
      else
         {
         if (_postConditions)
            _postConditions->setNeedToClearFPStack(b);
         }
      }

   uint32_t getNumPreConditions() {return _numPreConditions;}

   uint32_t setNumPreConditions(uint32_t n, TR_Memory * m)
      {
      if (_preConditions == NULL)
         {
         _preConditions = TR::RegisterDependencyGroup::create(n, m);
         }
      return (_numPreConditions = n);
      }

   uint32_t getNumPostConditions() {return _numPostConditions;}
   uint32_t setNumPostConditions(uint32_t n, TR_Memory * m)
      {
      if (_postConditions == NULL)
         {
         _postConditions = TR::RegisterDependencyGroup::create(n, m);
         }
      return (_numPostConditions = n);
      }

   uint32_t getAddCursorForPre() {return _addCursorForPre;}
   uint32_t setAddCursorForPre(uint32_t a) {return (_addCursorForPre = a);}

   uint32_t getAddCursorForPost() {return _addCursorForPost;}
   uint32_t setAddCursorForPost(uint32_t a) {return (_addCursorForPost = a);}

   void addPreCondition(TR::Register              *vr,
                        TR::RealRegister::RegNum   rr,
                        TR::CodeGenerator         *cg,
                        uint8_t                    flag = UsesDependentRegister,
                        bool                       isAssocRegDependency = false)
      {
      uint32_t newCursor = unionRealDependencies(_preConditions, _addCursorForPre, vr, rr, cg, flag, isAssocRegDependency);
      TR_ASSERT(newCursor <= _numPreConditions, "Too many dependencies");
      if (newCursor == _addCursorForPre)
         _numPreConditions--; // A vmThread/ebp dependency was displaced, so there is now one less condition.
      else
         _addCursorForPre = newCursor;
      }

   void unionPreCondition(TR::Register              *vr,
                          TR::RealRegister::RegNum   rr,
                          TR::CodeGenerator         *cg,
                          uint8_t                    flag = UsesDependentRegister,
                          bool                       isAssocRegDependency = false)
      {
      uint32_t newCursor = unionDependencies(_preConditions, _addCursorForPre, vr, rr, cg, flag, isAssocRegDependency);
      TR_ASSERT(newCursor <= _numPreConditions, "Too many dependencies");
      if (newCursor == _addCursorForPre)
         _numPreConditions--; // A union occurred, so there is now one less condition
      else
         _addCursorForPre = newCursor;
      }

   TR::RegisterDependencyGroup *getPostConditions() {return _postConditions;}

   void addPostCondition(TR::Register              *vr,
                         TR::RealRegister::RegNum   rr,
                         TR::CodeGenerator         *cg,
                         uint8_t                    flag = UsesDependentRegister,
                         bool                       isAssocRegDependency = false)
      {
      uint32_t newCursor = unionRealDependencies(_postConditions, _addCursorForPost, vr, rr, cg, flag, isAssocRegDependency);
      TR_ASSERT(newCursor <= _numPostConditions, "Too many dependencies");
      if (newCursor == _addCursorForPost)
         _numPostConditions--; // A vmThread/ebp dependency was displaced, so there is now one less condition.
      else
         _addCursorForPost = newCursor;
      }

   void unionPostCondition(TR::Register              *vr,
                           TR::RealRegister::RegNum   rr,
                           TR::CodeGenerator         *cg,
                           uint8_t                    flag = UsesDependentRegister,
                           bool                       isAssocRegDependency = false)
      {
      uint32_t newCursor = unionDependencies(_postConditions, _addCursorForPost, vr, rr, cg, flag, isAssocRegDependency);
      TR_ASSERT(newCursor <= _numPostConditions, "Too many dependencies");
      if (newCursor == _addCursorForPost)
         _numPostConditions--; // A union occurred, so there is now one less condition
      else
         _addCursorForPost = newCursor;
      }

   TR::RegisterDependency *findPreCondition (TR::Register *vr){ return _preConditions ->findDependency(vr, _addCursorForPre ); }
   TR::RegisterDependency *findPostCondition(TR::Register *vr){ return _postConditions->findDependency(vr, _addCursorForPost); }
   TR::RegisterDependency *findPreCondition (TR::RealRegister::RegNum rr){ return _preConditions ->findDependency(rr, _addCursorForPre ); }
   TR::RegisterDependency *findPostCondition(TR::RealRegister::RegNum rr){ return _postConditions->findDependency(rr, _addCursorForPost); }

   void assignPreConditionRegisters(TR::Instruction *currentInstruction, TR_RegisterKinds kindsToBeAssigned, TR::CodeGenerator *cg)
      {
      if (_preConditions != NULL)
         {
         if ((kindsToBeAssigned & TR_X87_Mask))
            {
            _preConditions->assignFPRegisters(currentInstruction, kindsToBeAssigned, _numPreConditions, cg);
            }
         else
            {
            cg->clearRegisterAssignmentFlags();
            cg->setRegisterAssignmentFlag(TR_PreDependencyCoercion);
            _preConditions->assignRegisters(currentInstruction, kindsToBeAssigned, _numPreConditions, cg);
            }
         }
      }

   void assignPostConditionRegisters(TR::Instruction *currentInstruction, TR_RegisterKinds kindsToBeAssigned, TR::CodeGenerator *cg)
      {
      if (_postConditions != NULL)
         {
         if ((kindsToBeAssigned & TR_X87_Mask))
            {
            _postConditions->assignFPRegisters(currentInstruction, kindsToBeAssigned, _numPostConditions, cg);
            }
         else
            {
            cg->clearRegisterAssignmentFlags();
            cg->setRegisterAssignmentFlag(TR_PostDependencyCoercion);
            _postConditions->assignRegisters(currentInstruction, kindsToBeAssigned, _numPostConditions, cg);
            }
         }
      }

   void blockPreConditionRegisters()
      {
      _preConditions->blockRegisters(_numPreConditions);
      }

   void unblockPreConditionRegisters()
      {
      _preConditions->unblockRegisters(_numPreConditions);
      }

   void blockPostConditionRegisters()
      {
      _postConditions->blockRegisters(_numPostConditions);
      }

   void unblockPostConditionRegisters()
      {
      _postConditions->unblockRegisters(_numPostConditions);
      }

   void blockPostConditionRealDependencyRegisters(TR::CodeGenerator *cg)
      {
      _postConditions->blockRealDependencyRegisters(_numPostConditions, cg);
      }

   void unblockPostConditionRealDependencyRegisters(TR::CodeGenerator *cg)
      {
      _postConditions->unblockRealDependencyRegisters(_numPostConditions, cg);
      }

   void blockPreConditionRealDependencyRegisters(TR::CodeGenerator *cg)
      {
      _preConditions->blockRealDependencyRegisters(_numPreConditions, cg);
      }

   void unblockPreConditionRealDependencyRegisters(TR::CodeGenerator *cg)
      {
      _preConditions->unblockRealDependencyRegisters(_numPreConditions, cg);
      }

   // All conditions are added - set the number of conditions to be the number
   // currently added
   //
   void stopAddingPreConditions()
      {
      _numPreConditions  = _addCursorForPre;
      }

   void stopAddingPostConditions()
      {
      _numPostConditions  = _addCursorForPost;
      }

   void stopAddingConditions()
      {
      stopAddingPreConditions();
      stopAddingPostConditions();
      }

   void createRegisterAssociationDirective(TR::Instruction *instruction, TR::CodeGenerator *cg);

   TR::RealRegister *getRealRegisterFromVirtual(TR::Register *virtReg, TR::CodeGenerator *cg);

   bool refsRegister(TR::Register *r);
   bool defsRegister(TR::Register *r);
   bool usesRegister(TR::Register *r);

   void useRegisters(TR::Instruction  *instr, TR::CodeGenerator *cg);

#if defined(DEBUG) || defined(PROD_WITH_ASSUMES)
   uint32_t numReferencedGPRegisters(TR::CodeGenerator *);
   uint32_t numReferencedFPRegisters(TR::CodeGenerator *);
   void printFullRegisterDependencyInfo(FILE *pOutFile);
   void printDependencyConditions(TR::RegisterDependencyGroup *conditions,
                                  uint32_t                     numConditions,
                                  char                        *prefix,
                                  FILE                        *pOutFile);
#endif

   };
}
}

////////////////////////////////////////////////////
// Generate Routines
////////////////////////////////////////////////////

TR::RegisterDependencyConditions  * generateRegisterDependencyConditions(TR::Node *, TR::CodeGenerator *, uint32_t = 0, List<TR::Register> * = 0);
TR::RegisterDependencyConditions  * generateRegisterDependencyConditions(uint32_t, uint32_t, TR::CodeGenerator *);

#endif
