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

#ifndef OMR_Z_REGISTER_DEPENDENCY_GROUP_INCL
#define OMR_Z_REGISTER_DEPENDENCY_GROUP_INCL

/*
 * The following #define and typedef must appear before any #includes in this file
 */
#ifndef OMR_REGISTER_DEPENDENCY_GROUP_CONNECTOR
#define OMR_REGISTER_DEPENDENCY_GROUP_CONNECTOR
namespace OMR { namespace Z { class RegisterDependencyGroup; } }
namespace OMR { typedef OMR::Z::RegisterDependencyGroup RegisterDependencyGroupConnector; }
#else
#error OMR::Z::RegisterDependencyGroup expected to be a primary connector, but an OMR connector is already defined
#endif

#include "compiler/codegen/OMRRegisterDependencyGroup.hpp"

#include <stdint.h>
#include "codegen/RealRegister.hpp"
#include "codegen/Register.hpp"
#include "codegen/RegisterConstants.hpp"
#include "infra/Annotations.hpp"

namespace TR { class Instruction; }
namespace TR { class CodeGenerator; }


namespace OMR
{

namespace Z
{

class OMR_EXTENSIBLE RegisterDependencyGroup : public OMR::RegisterDependencyGroup
   {

public:

   void clearDependencyInfo(uint32_t index)
      {
      _dependencies[index].setRegister(NULL);
      _dependencies[index].assignFlags(0);
      _dependencies[index].setRealRegister(TR::RealRegister::NoReg);
      }

   using OMR::RegisterDependencyGroup::setDependencyInfo;

   void setDependencyInfo(uint32_t index,
                          TR::Register *vr,
                          TR::RealRegister::RegDep rd,
                          uint8_t flag);

   TR::Register *searchForRegister(TR::Register* vr, uint8_t flag, uint32_t numberOfRegisters, TR::CodeGenerator *cg)
      {
      for (int32_t i=0; i<numberOfRegisters; i++)
         {
         if (_dependencies[i].getRegister() == vr && (_dependencies[i].getFlags() & flag))
            return _dependencies[i].getRegister();
         }
      return NULL;
      }

   TR::Register *searchForRegister(TR::RealRegister::RegNum rr, uint8_t flag, uint32_t numberOfRegisters, TR::CodeGenerator *cg)
      {
      for (int32_t i=0; i<numberOfRegisters; i++)
         {
         if (_dependencies[i].getRealRegister() == rr && (_dependencies[i].getFlags() & flag))
            return _dependencies[i].getRegister();
         }
      return NULL;
      }

   int32_t searchForRegisterPos(TR::Register* vr, uint8_t flag, uint32_t numberOfRegisters, TR::CodeGenerator *cg)
      {
      for (int32_t i=0; i<numberOfRegisters; i++)
         {
         if (_dependencies[i].getRegister() == vr && (_dependencies[i].getFlags() & flag))
            return i;
         }
      return -1;
      }

   uint32_t genBitMapOfAssignableGPRs(TR::CodeGenerator *cg, uint32_t numberOfRegisters);

   void setRealRegisterForDependency(int32_t index, TR::RealRegister::RegNum regNum)
      {
      _dependencies[index].setRealRegister(regNum);
      }

   void assignRegisters(TR::Instruction  *currentInstruction,
                        TR_RegisterKinds kindToBeAssigned,
                        uint32_t         numberOfRegisters,
                        TR::CodeGenerator *cg);

   void stopUsingDepRegs(uint32_t numberOfRegisters, TR::Register *ret1, TR::Register *ret2, TR::CodeGenerator *cg)
      {
      for (uint32_t i = 0; i < numberOfRegisters; i++)
         {
         TR::Register *depReg = _dependencies[i].getRegister();
         if (depReg != ret1 && depReg != ret2)
            cg->stopUsingRegister(depReg);
         }
      }

   int8_t getNumUses() { return _numUses; }

   void incNumUses(int8_t n=1) { _numUses+=n; }

protected:

   RegisterDependencyGroup() : _numUses(0) {}

private:

   int8_t _numUses;

   };

}

}

#endif
