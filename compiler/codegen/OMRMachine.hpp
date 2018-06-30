/*******************************************************************************
 * Copyright (c) 2000, 2018 IBM Corp. and others
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

#ifndef OMR_MACHINE_INCL
#define OMR_MACHINE_INCL

/*
 * The following #define and typedef must appear before any #includes in this file
 */
#ifndef OMR_MACHINE_CONNECTOR
#define OMR_MACHINE_CONNECTOR
namespace OMR { class Machine; }
namespace OMR { typedef OMR::Machine MachineConnector; }
#endif

#include <stddef.h>                            // for NULL
#include <stdint.h>                            // for uint8_t, uint32_t
#include "codegen/RealRegister.hpp"
#include "env/TRMemory.hpp"                    // for TR_Memory, etc
#include "infra/Annotations.hpp"               // for OMR_EXTENSIBLE

namespace TR { class CodeGenerator; }
namespace TR { class Machine; }

namespace OMR
{

class OMR_EXTENSIBLE Machine
   {

public:

   TR_ALLOC(TR_Memory::Machine)

   Machine(TR::CodeGenerator *cg) :
         _cg(cg)
      {
      }

   inline TR::Machine * self();

   /**
    * @return : the cached TR::CodeGenerator object
    */
   TR::CodeGenerator *cg() {return _cg;}

   /**
    * @brief Retrieve a pointer to the register file
    */
   TR::RealRegister **registerFile() { return _registerFile; }

   /**
    * @brief Retrieves the TR::RealRegister object for the given real register
    *
    * @param[in] rn : the desired real register
    *
    * @return : the desired TR::RealRegister object
    */
   TR::RealRegister *realRegister(TR::RealRegister::RegNum rn) { return _registerFile[rn]; }

private:

   TR::CodeGenerator *_cg;

protected:

   TR::RealRegister *_registerFile[TR::RealRegister::NumRegisters];

   };

}

#endif
