/*******************************************************************************
 * Copyright IBM Corp. and others 2000
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License 2.0 which accompanies this
 * distribution and is available at https://www.eclipse.org/legal/epl-2.0/
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

#include "codegen/CodeGenerator.hpp"
#include "codegen/Instruction.hpp"
#include "codegen/InstOpCode.hpp"
#include "codegen/Machine.hpp"
#include "codegen/RealRegister.hpp"

TR_RegisterMask OMR::X86::RealRegister::getAvailableRegistersMask(TR_RegisterKinds rk)
{
    if (rk == TR_GPR)
        return TR::RealRegister::AvailableGPRMask;
    else if (rk == TR_FPR || rk == TR_VRF)
        return TR::RealRegister::AvailableXMMRMask;
    else // MMX: not used
        return 0;
}

TR::RealRegister::RegMask OMR::X86::RealRegister::getRealRegisterMask(TR_RegisterKinds rk, TR::RealRegister::RegNum idx)
{
    if (rk == TR_GPR)
        return TR::RealRegister::gprMask(idx);
    else if (rk == TR_FPR || rk == TR_VRF)
        return TR::RealRegister::xmmrMask(idx);
    else if (rk == TR_VMR)
        return TR::RealRegister::vmrMask(idx);
    else
        TR_ASSERT_FATAL(false, "Unknown register kind");
}

void OMR::X86::RealRegister::analyzeOperand(OMR::X86::OperandProperties &opndProps,
    OMR::X86::InstructionEncodingBits &encBits, TR::CodeGenerator *cg)
{
    TR_ASSERT_FATAL(opndProps.opnd_kind == opnd_Reg, "Operand expected to be a register");

    uint16_t id = self()->getId();
    uint16_t needsRXBV3 = self()->needsRXBV3();
    uint16_t needsRXBV4 = self()->needsRXBV4();

    switch (opndProps.opnd_regEncoding) {
        case opnd_MR_reg_R3:
            encBits.ModRM.Reg = id;
            encBits.R3 = needsRXBV3;
            encBits.R4 = needsRXBV4;
            encBits.needsModRM = 1;
            break;
        case opnd_MR_rm_B3:
            encBits.ModRM.RM = id;
            encBits.B3 = needsRXBV3;
            encBits.B4 = needsRXBV4;
            encBits.needsModRM = 1;
            break;
        case opnd_OPC_reg_B3:
            encBits.opcodeReg = id; // opcodeReg needs to be just the 3 bits in the bitfield
            encBits.B3 = needsRXBV3;
            encBits.B4 = needsRXBV4;
            break;
        case opnd_vvvv:
            encBits.vvvv = self()->getVVVV(); // inverted bits
            encBits.V4 = needsRXBV4;
            break;
        default:
            TR_ASSERT_FATAL(false, "Unexpected operand encoding");
    }
}
