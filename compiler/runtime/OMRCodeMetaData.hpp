/*******************************************************************************
 * Copyright IBM Corp. and others 2000
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
 * [2] https://openjdk.org/legal/assembly-exception.html
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0-only WITH Classpath-exception-2.0 OR GPL-2.0-only WITH OpenJDK-assembly-exception-1.0
 *******************************************************************************/

#ifndef OMR_CODE_METADATA_INCL
#define OMR_CODE_METADATA_INCL

/*
 * The following #define(s) and typedef(s) must appear before any #includes in this file
 */
#ifndef OMR_CODE_METADATA_CONNECTOR
#define OMR_CODE_METADATA_CONNECTOR
namespace OMR { struct CodeMetaData; }
namespace OMR { typedef OMR::CodeMetaData CodeMetaDataConnector; }
#endif

#include "env/jittypes.h"
#include "compile/CompilationTypes.hpp"
#include "infra/Annotations.hpp"

namespace TR { class CodeMetaData; }
namespace TR { class Compilation; }

/**
 * CodeMetaData contains metadata information about a single method.
 */

namespace OMR
{

class OMR_EXTENSIBLE CodeMetaData
   {
   public:

   TR::CodeMetaData *self();

   /**
    * @brief Initialize CodeMetaData for a given identifier from \c TR::Compilation object
    *
    * @param id : An environment-specific identifier for this code.  It could be a
    *    numeric identifier or the address of some object that uniquely identifies this
    *    code in the context of some language environment.
    * @param comp : \c TR::Compilation object
    */
   void initialize(uintptr_t id, TR::Compilation *comp);

   /**
    * @brief Returns the environment-specific identifier for this code
    */
   uintptr_t getIdentifier() { return _id; }

   /**
    * @brief Returns the first address of code cache allocated memory for a method
    */
   uintptr_t getCodeAllocStart() { return _codeAllocStart; }

   /**
    * @brief Returns the last address of code cache allocated memory for a method
    */
   uintptr_t getCodeAllocEnd() { return _codeAllocEnd; }

   /**
    * @brief Returns the total size of code memory allocated for a method
    * within a code cache.
    */
   uint32_t getCodeAllocSize() { return _codeAllocEnd - _codeAllocStart + 1; }

   /**
    * @brief Returns the starting address of compiled code for a
    * method when invoked from an interpreter.
    *
    * Interpreter entry PC may preceed compiled entry PC and may point
    * to code necessary for proper execution of this method if invoked
    * from an interpreter. For example, it may need to marshall method
    * arguments from an interpreter linkage to a compiled method linkage.
    *
    * By default, the interpreter entry PC and compiled entry PC point
    * to the same address.
    */
   uintptr_t getInterpreterEntryPC() { return _interpreterEntryPC; }

   /**
    * @brief Returns the starting address of compiled code for a
    * method when invoked from compiled code.
    *
    * By default, the interpreter entry PC and compiled entry PC point
    * to the same address.
    */
   uintptr_t getCompiledEntryPC() { return _compiledEntryPC; }

   /**
    * @brief Returns the end address of compiled code for a method.
    */
   uintptr_t getCompiledEndPC() { return _compiledEndPC; }

   /**
    * @brief Returns the compilation hotness level of a compiled method.
    */
   TR_Hotness getCodeHotness() { return _hotness; }

   protected:

   uintptr_t _id;

   uintptr_t _codeAllocStart;
   uintptr_t _codeAllocEnd;

   uintptr_t _interpreterEntryPC;
   uintptr_t _compiledEntryPC;
   uintptr_t _compiledEndPC;

   TR_Hotness _hotness;
   };

}

#endif
