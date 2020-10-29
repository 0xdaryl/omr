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

#ifndef OMR_REGISTER_DEPENDENCY_GROUP_INCL
#define OMR_REGISTER_DEPENDENCY_GROUP_INCL

/*
 * The following #define and typedef must appear before any #includes in this file
 */
#ifndef OMR_REGISTER_DEPENDENCY_GROUP_CONNECTOR
#define OMR_REGISTER_DEPENDENCY_GROUP_CONNECTOR
namespace OMR { class RegisterDependencyGroup; }
namespace OMR { typedef OMR::RegisterDependencyGroup RegisterDependencyGroupConnector; }
#endif

#include "env/TRMemory.hpp"
#include "codegen/RealRegister.hpp"
#include "codegen/Register.hpp"
#include "codegen/RegisterDependencyStruct.hpp"
#include "infra/Annotations.hpp"
#include "infra/Assert.hpp"

namespace TR { class RegisterDependencyGroup; }

#define NUM_DEFAULT_DEPENDENCIES 1

namespace OMR
{

class OMR_EXTENSIBLE RegisterDependencyGroup
   {

public:

   static TR::RegisterDependencyGroup *create(int32_t numDependencies, TR_Memory *m);

   /**
    * @param[in] \p index : zero-based index of \c a TR::RegisterDependency object in this group
    *
    * @return Returns a pointer to the \c TR::RegisterDependency object at \p index
    */
   TR::RegisterDependency *getRegisterDependency(uint32_t index)
      {
      return &_dependencies[index];
      }


   // DMDM: X86 to override
   void setDependencyInfo(
         uint32_t index,
         TR::Register *vr,
         TR::RealRegister::RegNum rr,
         uint8_t flag)
      {
      _dependencies[index].setRegister(vr);
      _dependencies[index].assignFlags(flag);
      _dependencies[index].setRealRegister(rr);
      }


   void blockRegisters(uint32_t numberOfRegisters)
      {
      for (uint32_t i = 0; i < numberOfRegisters; i++)
         {
         // DMDM: REMOVE:
         TR_ASSERT_FATAL(_dependencies[i].getRegister(), "blockRegisters: virtual register is null for i=%d", i);
         _dependencies[i].getRegister()->block();
         }
      }


   void unblockRegisters(uint32_t numberOfRegisters)
      {
      for (uint32_t i = 0; i < numberOfRegisters; i++)
         {
         // DMDM: REMOVE:
         TR_ASSERT_FATAL(_dependencies[i].getRegister(), "unblockRegisters: virtual register is null for i=%d", i);
         _dependencies[i].getRegister()->unblock();
         }
      }

protected:

   TR::RegisterDependency _dependencies[NUM_DEFAULT_DEPENDENCIES];

   TR::RegisterDependencyGroup *self();

private:

   TR_ALLOC_WITHOUT_NEW(TR_Memory::RegisterDependencyGroup)

   void * operator new(size_t s, int32_t numDependencies, TR_Memory *m)
      {
      TR_ASSERT_FATAL(numDependencies > 0, "operator new called with numDependencies == 0");
      if (numDependencies > NUM_DEFAULT_DEPENDENCIES)
         {
         s += (numDependencies-NUM_DEFAULT_DEPENDENCIES)*sizeof(TR::RegisterDependency);
         }
      return m->allocateHeapMemory(s);
      }

/*
   // DMDM: not needed?
   // Power, AArch64, RISCV
   void * operator new(size_t s, TR_Memory * m)
      {
      return m->allocateHeapMemory(s);
      }
*/

   // DMDM: X86 : REMOVE?
   void operator delete(void *p, int32_t numDependencies, TR_Memory *m)
      {

      TR_ASSERT_FATAL(0, "operator delete RegisterDependencyGroup");

      m->freeMemory(p, TR_AllocationKind::heapAlloc);
      }



   };

}

#endif
