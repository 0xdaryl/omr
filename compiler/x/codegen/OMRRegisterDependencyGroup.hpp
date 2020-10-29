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

#ifndef OMR_X86_REGISTER_DEPENDENCY_GROUP_INCL
#define OMR_X86_REGISTER_DEPENDENCY_GROUP_INCL

/*
 * The following #define and typedef must appear before any #includes in this file
 */
#ifndef OMR_REGISTER_DEPENDENCY_GROUP_CONNECTOR
#define OMR_REGISTER_DEPENDENCY_GROUP_CONNECTOR
namespace OMR { namespace X86 { class RegisterDependencyGroup; } }
namespace OMR { typedef OMR::X86::RegisterDependencyGroup RegisterDependencyGroupConnector; }
#else
#error OMR::X86::RegisterDependencyGroup expected to be a primary connector, but an OMR connector is already defined
#endif

#include "compiler/codegen/OMRRegisterDependencyGroup.hpp"

#include <stdint.h>
#include "codegen/RealRegister.hpp"
#include "codegen/Register.hpp"
#include "codegen/RegisterConstants.hpp"
#include "infra/Annotations.hpp"

namespace TR { class Instruction; }
namespace TR { class CodeGenerator; }
template <typename ListKind> class List;


namespace OMR
{

namespace X86
{

class OMR_EXTENSIBLE RegisterDependencyGroup : public OMR::RegisterDependencyGroup
   {

public:

   void setDependencyInfo(uint32_t                        index,
                          TR::Register                   *vr,
                          TR::RealRegister::RegNum        rr,
                          TR::CodeGenerator              *cg,
                          uint8_t                         flag = UsesDependentRegister,
                          bool                            isAssocRegDependency = false);

   TR::RegisterDependency *findDependency(TR::Register *vr, uint32_t stop)
      {
      TR::RegisterDependency *result = NULL;
      for (uint32_t i=0; !result && (i < stop); i++)
         if (_dependencies[i].getRegister() == vr)
            result = _dependencies+i;
      return result;
      }

   TR::RegisterDependency *findDependency(TR::RealRegister::RegNum rr, uint32_t stop)
      {
      TR::RegisterDependency *result = NULL;
      for (uint32_t i=0; !result && (i < stop); i++)
         if (_dependencies[i].getRealRegister() == rr)
            result = _dependencies+i;
      return result;
      }

   void blockRealDependencyRegisters(uint32_t numberOfRegisters, TR::CodeGenerator *cg);
   void unblockRealDependencyRegisters(uint32_t numberOfRegisters, TR::CodeGenerator *cg);

   void assignRegisters(TR::Instruction   *currentInstruction,
                        TR_RegisterKinds  kindsToBeAssigned,
                        uint32_t          numberOfRegisters,
                        TR::CodeGenerator *cg);

   void assignFPRegisters(TR::Instruction   *currentInstruction,
                          TR_RegisterKinds  kindsToBeAssigned,
                          uint32_t          numberOfRegisters,
                          TR::CodeGenerator *cg);

   void orderGlobalRegsOnFPStack(TR::Instruction    *cursor,
                                 TR_RegisterKinds   kindsToBeAssigned,
                                 uint32_t           numberOfRegisters,
                                 List<TR::Register> *poppedRegisters,
                                 TR::CodeGenerator  *cg);

   void setMayNeedToPopFPRegisters(bool b) {_mayNeedToPopFPRegisters = b;}
   bool getMayNeedToPopFPRegisters() {return _mayNeedToPopFPRegisters;}

   void setNeedToClearFPStack(bool b) {_needToClearFPStack = b;}
   bool getNeedToClearFPStack() {return _needToClearFPStack;}

protected:

   RegisterDependencyGroup() :
      _mayNeedToPopFPRegisters(false),
      _needToClearFPStack(false)
      {}

private:

   bool _mayNeedToPopFPRegisters;
   bool _needToClearFPStack;

   };

}

}

#endif
