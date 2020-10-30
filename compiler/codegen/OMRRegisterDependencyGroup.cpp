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

// On zOS, the XLC linker can't handle files with same name at link time
// This workaround with pragma is needed. What this does is essentially
// give a different name to the codesection (csect) for this file. So it
// doesn't conflict with another file with same name.

#pragma csect(CODE,"OMRRegDepGroup#C")
#pragma csect(STATIC,"OMRRegDepGroup#S")
#pragma csect(TEST,"OMRRegDepGroup#T")

#include "codegen/RegisterDependencyGroup.hpp"

#include <stdint.h>
#include "env/TRMemory.hpp"


TR::RegisterDependencyGroup *
OMR::RegisterDependencyGroup::create(int32_t numDependencies, TR_Memory *m)
   {
   return numDependencies ? new (numDependencies, m) TR::RegisterDependencyGroup : 0;
   }
