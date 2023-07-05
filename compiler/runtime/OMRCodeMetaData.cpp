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

#include "runtime/CodeMetaData.hpp"

#include "codegen/CodeGenerator.hpp"
#include "compile/Compilation.hpp"
#include "compile/ResolvedMethod.hpp"
#include "omrcomp.h"

static_assert(OMR_IS_TRIVIAL(TR::CodeMetaData), "TR::CodeMetaData must be a trivial class");
static_assert(OMR_IS_STANDARD_LAYOUT(TR::CodeMetaData), "TR::CodeMetaData must be a standard layout class");

TR::CodeMetaData *
OMR::CodeMetaData::self()
   {
   return static_cast<TR::CodeMetaData *>(this);
   }

void
OMR::CodeMetaData::initialize(uintptr_t id, TR::Compilation *comp)
   {
   TR::CodeGenerator *cg = comp->cg();

   _id = id;

   _codeAllocStart = reinterpret_cast<uintptr_t>(cg->getBinaryBufferStart());
   _codeAllocEnd = reinterpret_cast<uintptr_t>(cg->getCodeEnd());

   _interpreterEntryPC = reinterpret_cast<uintptr_t>(cg->getCodeStart());

   _compiledEntryPC = _interpreterEntryPC;
   _compiledEndPC = reinterpret_cast<uintptr_t>(cg->getCodeEnd());

   _hotness = comp->getMethodHotness();
   }

