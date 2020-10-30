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

#ifndef OMR_ARM64_REGISTER_DEPENDENCY_GROUP_INCL
#define OMR_ARM64_REGISTER_DEPENDENCY_GROUP_INCL

/*
 * The following #define and typedef must appear before any #includes in this file
 */
#ifndef OMR_REGISTER_DEPENDENCY_GROUP_CONNECTOR
#define OMR_REGISTER_DEPENDENCY_GROUP_CONNECTOR
namespace OMR { namespace ARM64 { class RegisterDependencyGroup; } }
namespace OMR { typedef OMR::ARM64::RegisterDependencyGroup RegisterDependencyGroupConnector; }
#else
#error OMR::ARM64::RegisterDependencyGroup expected to be a primary connector, but an OMR connector is already defined
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

namespace ARM64
{

class OMR_EXTENSIBLE RegisterDependencyGroup : public OMR::RegisterDependencyGroup
   {

public:

   /**
    * @brief Searches for a register
    * @param[in] rr : real register number
    * @param[in] numberOfRegisters : # of registers
    * @return register when found, NULL when not found
    */
   TR::Register *searchForRegister(TR::RealRegister::RegNum rr, uint32_t numberOfRegisters)
      {
      for (uint32_t i=0; i<numberOfRegisters; i++)
         {
         if (_dependencies[i].getRealRegister() == rr)
            return _dependencies[i].getRegister();
         }
      return NULL;
      }

   /**
    * @brief Assigns registers
    * @param[in] currentInstruction : current instruction
    * @param[in] kindToBeAssigned : kind of register to be assigned
    * @param[in] numberOfRegisters : # of registers
    * @param[in] cg : code generator
    */
   void assignRegisters(TR::Instruction *currentInstruction,
                        TR_RegisterKinds kindToBeAssigned,
                        uint32_t numberOfRegisters,
                        TR::CodeGenerator *cg);

   template<class It>
   void stopUsingDepRegs(uint32_t numberOfRegisters, It skipRegsBegin, It skipRegsEnd, TR::CodeGenerator *cg);

   /**
    * @brief Kills registers held by this dependency group
    * @param[in] numberOfRegisters : # of registers
    * @param[in] returnRegister    : register which is not killed
    * @param[in] cg                : CodeGenerator
    */
   void stopUsingDepRegs(uint32_t numberOfRegisters, TR::Register *returnRegister, TR::CodeGenerator *cg);

protected:

   RegisterDependencyGroup() {}

   };

}

}

#endif
