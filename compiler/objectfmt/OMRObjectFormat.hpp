/*******************************************************************************
 * Copyright (c) 2020, 2020 IBM Corp. and others
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

#ifndef OMR_OBJECTFORMAT_INCL
#define OMR_OBJECTFORMAT_INCL

/*
 * The following #define and typedef must appear before any #includes in this file
 */
#ifndef OMR_OBJECTFORMAT_CONNECTOR
#define OMR_OBJECTFORMAT_CONNECTOR
namespace OMR { class ObjectFormat; }
namespace OMR { typedef OMR::ObjectFormat ObjectFormatConnector; }
#endif

#include <stddef.h>
#include <stdint.h>
#include "env/TRMemory.hpp"
#include "infra/Annotations.hpp"

namespace TR { class FunctionCallData; }
namespace TR { class Instruction; }

namespace OMR
{

class OMR_EXTENSIBLE ObjectFormat
   {

public:

   TR_ALLOC(TR_Memory::ObjectFormat)

   /**
    * @brief Emit a sequence of \c TR::Instruction 's to call a global function (one that
    *        does not reside in the code cache, but in a shared library for example) using
    *        this object format.
    *
    * @param[in] data : a populated \c TR::FunctionCallData structure with
    *                   valid parameters for this call site.  Note that this function
    *                   may alter the contents of this structure.
    *
    * @return : the final \c TR::Instruction produced in the instruction stream to emit a
    *           call to a global function for this object format.
    */
   virtual TR::Instruction *emitGlobalFunctionCall(TR::FunctionCallData &data) = 0;

   /**
    * @brief Emit the binary code to call a global function (one that does not reside in
    *        the code cache, but in a shared library for example) using this object format.
    *        This function is suitable for calling during binary encoding and does not
    *        use \c TR::Instruction 's.
    *
    * @param[in] data : a populated \c TR::FunctionCallData structure with valid
    *               parameters for this call site.  Note that this function may alter
    *               the contents of this structure.
    *
    * @return : the next buffer address following the necessary instruction(s) to emit
    *           a global function call using this object format.
    */
   virtual uint8_t *encodeGlobalFunctionCall(TR::FunctionCallData &data) = 0;

   /**
    * @brief Return the length in bytes of the binary encoding sequence to generate a
    *        call to a global function using this object format.  If the exact length
    *        cannot be determined, this should return a conservatively correct estimate
    *        that is at least as large as the actual length.
    *
    * @return : the length in bytes of the encoding of a global function call
    */
   virtual int32_t globalFunctionCallBinaryLength() = 0;

   /**
    * @brief Emit a sequence of \c TR::Instruction 's to call another function that
    *        resides in the code cache using this object format.
    *
    * @param[in] data : a populated \c TR::FunctionCallData structure with
    *                   valid parameters for this call site.  Note that this function
    *                   may alter the contents of this structure.
    *
    * @return : the final \c TR::Instruction produced in the instruction stream to emit
    *           a call to a code cache function for this object format.
    */
   virtual TR::Instruction *emitCodeCacheFunctionCall(TR::FunctionCallData &data) = 0;

   /**
    * @brief Emit the binary code to call a function that resides in the code cache
    *        using this object format.  This function is suitable for calling during
    *        binary encoding and does not use \c TR::Instruction 's.
    *
    * @param[in] data : a populated \c TR::FunctionCallData structure with
    *               valid parameters for this call site.  Note that this function may
    *               alter the contents of this structure.
    *
    * @return : the next buffer address following the necessary instruction(s) to emit
    *           a call using this object format.
    */
   virtual uint8_t *encodeCodeCacheFunctionCall(TR::FunctionCallData &data) = 0;

   /**
    * @brief Return the length in bytes of the binary encoding sequence to generate a call
    *        to a function in the code cache using this object format.  If the exact length
    *        cannot be determined, this should return a conservatively correct estimate that
    *        is at least as large as the actual length.
    *
    * @return : the length in bytes of the encoding of a code cache function call
    */
   virtual int32_t codeCacheFunctionCallBinaryLength() = 0;

   };

}

#endif
