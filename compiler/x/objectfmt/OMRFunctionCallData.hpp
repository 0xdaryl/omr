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

#ifndef OMR_X86_FUNCTIONCALLDATA_INCL
#define OMR_X86_FUNCTIONCALLDATA_INCL

/*
 * The following #define and typedef must appear before any #includes in this file
 */
#ifndef OMR_FUNCTIONCALLDATA_CONNECTOR
#define OMR_FUNCTIONCALLDATA_CONNECTOR
namespace OMR { namespace X86 { class FunctionCallData; } }
namespace OMR { typedef OMR::X86::FunctionCallData FunctionCallDataConnector; }
#endif

#include "compiler/objectfmt/OMRFunctionCallData.hpp"

namespace TR { class SymbolReference; }
namespace TR { class Node; }
namespace TR { class CodeGenerator; }

namespace OMR
{

namespace X86
{

class OMR_EXTENSIBLE FunctionCallData : public OMR::FunctionCallData
   {
public:

   FunctionCallData(
         TR::SymbolReference *parm_methodSymRef,
         TR::Node *parm_callNode,
         uint8_t *parm_bufferAddress,
         TR::CodeGenerator *parm_cg) :
      OMR::FunctionCallData(parm_methodSymRef, parm_callNode, parm_bufferAddress, parm_cg) {}

   };

}

}

#endif
