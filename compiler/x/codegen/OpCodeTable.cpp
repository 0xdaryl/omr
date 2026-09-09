/*******************************************************************************
 * Copyright IBM Corp. and others 2026
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

#include "codegen/InstOpCode.hpp"
#include "codegen/CodeGenerator.hpp"

const OMR::X86::InstOpCode::NewOpCode OMR::X86::InstOpCode::_opCodeTable[] = {
    // clang-format off

    // ADD8_Reg_Reg_NF
    // ADD8_Mem_Reg_NF
    // ADD8_Reg_Mem_NF
    // ADD8_Reg_Reg_Reg
    // ADD8_Reg_Mem_Reg
    // ADD8_Reg_Reg_Mem
    // ADD8_Reg_Reg_Reg_NF
    // ADD8_Reg_Mem_Reg_NF
    // ADD8_Reg_Reg_Mem_NF
    // ADD8_Reg_Imm8_NF
    // ADD8_Mem_Imm8_NF
    // ADD8_Reg_Reg_Imm8
    // ADD8_Reg_Mem_Imm8
    // ADD8_Reg_Reg_Imm8_NF
    // ADD8_Reg_Mem_Imm8_NF

    // ADD16_Reg_Imm8s
    // ADD16_Mem_Imm8s
    // ADD16_Reg_Imm8s_NF
    // ADD16_Mem_Imm8s_NF
    // ADD32_Reg_Imm8s
    // ADD32_Mem_Imm8s
    // ADD32_Reg_Imm8s_NF
    // ADD32_Mem_Imm8s_NF
    // ADD64_Reg_Imm8s
    // ADD64_Mem_Imm8s
    // ADD64_Reg_Imm8s_NF
    // ADD64_Mem_Imm8s_NF
    // ADD16_Reg_Reg_Imm8s
    // ADD16_Reg_Mem_Imm8s
    // ADD16_Reg_Reg_Imm8s_NF
    // ADD16_Reg_Mem_Imm8s_NF
    // ADD32_Reg_Reg_Imm8s
    // ADD32_Reg_Mem_Imm8s
    // ADD32_Reg_Reg_Imm8s_NF
    // ADD32_Reg_Mem_Imm8s_NF
    // ADD64_Reg_Reg_Imm8s
    // ADD64_Reg_Mem_Imm8s
    // ADD64_Reg_Reg_Imm8s_NF
    // ADD64_Reg_Mem_Imm8s_NF

    // ADD16_Reg_Imm16
    // ADD16_Mem_Imm16
    // ADD16_Reg_Reg
    // ADD16_Mem_Reg
    // ADD16_Reg_Mem

    // ADD32_EAX_Imm32
    // ADD32_Reg_Imm32
    // ADD32_Mem_Imm32
    // ADD32_Reg_Reg
    // ADD32_Mem_Reg
    // ADD32_Reg_Mem

    // ADD64_RAX_Imm32s
    // ADD64_Reg_Imm32s
    // ADD64_Mem_Imm32s
    // ADD64_Reg_Reg
    // ADD64_Mem_Reg
    // ADD64_Reg_Mem

    // ADD8_AL_Imm8
    {
        {
            0x04,
            opc_Ext_None,
            OPC_ENCODING_PREFIXES(opc_Legacy),
            OPC_FLAGS(opc_SetAllFlags),
            opc_WIG,
            opc_Kind_None,
            opc_Map_None,
            OPC_MISC(opc_SetStatusFlagsForTEST | opc_SetStatusFlagsForCMP),
        },
        {
            // Operand 1
            { opnd_Reg, opnd_Int, opnd_Scalar, opnd_8, opnd_RW, opnd_ImpliedReg, opnd_NotImm, opnd_Implicit, },

            // Operand 2
            { opnd_Imm, opnd_Int, opnd_Scalar, opnd_8, opnd_R, opnd_ImmEnc, opnd_NoExt, opnd_Explicit, },
        },

        "ADD8_AL_Imm8",
        "add8",
    },

    // ADD8_Reg_Imm8
    {
        {
            0x80,
            opc_Ext_0,
            OPC_ENCODING_PREFIXES(opc_Legacy | opc_REX | opc_REX2),
            OPC_FLAGS(opc_SetAllFlags),
            opc_WIG,
            opc_Kind_None,
            opc_Map_None,
            OPC_MISC(opc_SetStatusFlagsForTEST | opc_SetStatusFlagsForCMP),
        },
        {
            // Operand 1
            { opnd_Reg, opnd_Int, opnd_Scalar, opnd_8, opnd_RW, opnd_MR_rm_B3, opnd_NotImm, opnd_Explicit, },

            // Operand 2
            { opnd_Imm, opnd_Int, opnd_Scalar, opnd_8, opnd_R, opnd_ImmEnc, opnd_NoExt, opnd_Explicit, },
        },

        "ADD8_Reg_Imm8",
        "add8",
    },

    // ADD8_Mem_Imm8
    {
        {
            0x80,
            opc_Ext_0,
            OPC_ENCODING_PREFIXES(opc_Legacy | opc_REX | opc_REX2),
            OPC_FLAGS(opc_SetAllFlags),
            opc_WIG,
            opc_Kind_None,
            opc_Map_None,
            OPC_MISC(opc_SupportsLOCKPrefix | opc_SetStatusFlagsForTEST | opc_SetStatusFlagsForCMP),
        },
        {
            // Operand 1
            { opnd_Mem, opnd_Int, opnd_Scalar, opnd_8, opnd_RW, opnd_MR_rm_B3, opnd_NotImm, opnd_Explicit, },

            // Operand 2
            { opnd_Imm, opnd_Int, opnd_Scalar, opnd_8, opnd_R, opnd_ImmEnc, opnd_NoExt, opnd_Explicit, },
        },

        "ADD8_Mem_Imm8",
        "add8",
    },

    // ADD8_Reg_Reg
    {
        {
            0x00,
            opc_Ext_None,
            OPC_ENCODING_PREFIXES(opc_Legacy | opc_REX | opc_REX2),
            OPC_FLAGS(opc_SetAllFlags),
            opc_WIG,
            opc_Kind_None,
            opc_Map_None,
            OPC_MISC(opc_SetStatusFlagsForTEST | opc_SetStatusFlagsForCMP),
        },
        {
            // Operand 1
            { opnd_Reg, opnd_Int, opnd_Scalar, opnd_8, opnd_RW, opnd_MR_rm_B3, opnd_NotImm, opnd_Explicit, },

            // Operand 2
            { opnd_Reg, opnd_Int, opnd_Scalar, opnd_8, opnd_R, opnd_MR_reg_R3, opnd_NotImm, opnd_Explicit, },
        },

        "ADD8_Reg_Reg",
        "add8",
    },

    // ADD8_Mem_Reg
    {
        {
            0x00,
            opc_Ext_None,
            OPC_ENCODING_PREFIXES(opc_Legacy | opc_REX | opc_REX2),
            OPC_FLAGS(opc_SetAllFlags),
            opc_WIG,
            opc_Kind_None,
            opc_Map_None,
            OPC_MISC(opc_SupportsLOCKPrefix | opc_SetStatusFlagsForTEST | opc_SetStatusFlagsForCMP),
        },
        {
            // Operand 1
            { opnd_Mem, opnd_Int, opnd_Scalar, opnd_8, opnd_RW, opnd_MR_rm_B3, opnd_NotImm, opnd_Explicit, },

            // Operand 2
            { opnd_Reg, opnd_Int, opnd_Scalar, opnd_8, opnd_R, opnd_MR_reg_R3, opnd_NotImm, opnd_Explicit, },
        },

        "ADD8_Mem_Reg",
        "add8",
    },

    // ADD8_Reg_Mem
    {
        {
            0x02,
            opc_Ext_None,
            OPC_ENCODING_PREFIXES(opc_Legacy | opc_REX | opc_REX2),
            OPC_FLAGS(opc_SetAllFlags),
            opc_WIG,
            opc_Kind_None,
            opc_Map_None,
            OPC_MISC(opc_SetStatusFlagsForTEST | opc_SetStatusFlagsForCMP),
        },
        {
            // Operand 1
            { opnd_Reg, opnd_Int, opnd_Scalar, opnd_8, opnd_RW, opnd_MR_reg_R3, opnd_NotImm, opnd_Explicit, },

            // Operand 2
            { opnd_Mem, opnd_Int, opnd_Scalar, opnd_8, opnd_R, opnd_MR_rm_B3, opnd_NotImm, opnd_Explicit, },
        },

        "ADD8_Reg_Mem",
        "add8",
    },

    // ADD16_AX_Imm16
    {
        {
            0x05,
            opc_Ext_None,
            OPC_ENCODING_PREFIXES(opc_Legacy),
            OPC_FLAGS(opc_SetAllFlags),
            opc_WIG,
            opc_Kind_None,
            opc_Map_None,
            OPC_MISC(opc_SetStatusFlagsForTEST | opc_SetStatusFlagsForCMP),
            opc_Prefix_66,
        },
        {
            // Operand 1
            { opnd_Reg, opnd_Int, opnd_Scalar, opnd_16, opnd_RW, opnd_ImpliedReg, opnd_NotImm, opnd_Implicit, },

            // Operand 2
            { opnd_Imm, opnd_Int, opnd_Scalar, opnd_16, opnd_R, opnd_ImmEnc, opnd_NoExt, opnd_Explicit, },
        },

        "ADD16_AX_Imm16",
        "add16",
    },

    // ADD32_EAX_Imm32
    {
        {
            0x05,
            opc_Ext_None,
            OPC_ENCODING_PREFIXES(opc_Legacy),
            OPC_FLAGS(opc_SetAllFlags),
            opc_WIG,
            opc_Kind_None,
            opc_Map_None,
            OPC_MISC(opc_SetStatusFlagsForTEST | opc_SetStatusFlagsForCMP),
        },
        {
            // Operand 1
            { opnd_Reg, opnd_Int, opnd_Scalar, opnd_32, opnd_RW, opnd_ImpliedReg, opnd_NotImm, opnd_Implicit, },

            // Operand 2
            { opnd_Imm, opnd_Int, opnd_Scalar, opnd_32, opnd_R, opnd_ImmEnc, opnd_NoExt, opnd_Explicit, },
        },

        "ADD32_EAX_Imm32",
        "add32",
    },

    // ADD64_RAX_Imm32s
    {
        {
            0x05,
            opc_Ext_None,
            OPC_ENCODING_PREFIXES(opc_Legacy | opc_REX),
            OPC_FLAGS(opc_SetAllFlags),
            opc_W1,
            opc_Kind_None,
            opc_Map_None,
            OPC_MISC(opc_SetStatusFlagsForTEST | opc_SetStatusFlagsForCMP),
        },
        {
            // Operand 1
            { opnd_Reg, opnd_Int, opnd_Scalar, opnd_64, opnd_RW, opnd_ImpliedReg, opnd_NotImm, opnd_Implicit, },

            // Operand 2
            { opnd_Imm, opnd_Int, opnd_Scalar, opnd_32, opnd_R, opnd_ImmEnc, opnd_SignExt, opnd_Explicit, },
        },

        "ADD64_RAX_Imm32s",
        "add64",
    },



    // ------------------------------------------------------------------------

    // ADD8_RegImm8
    {
        {
            0x80,
            opc_Ext_0,
            OPC_ENCODING_PREFIXES(opc_VEX2 | opc_VEX3),
            OPC_FLAGS(opc_TestZF | opc_TestSF | opc_SetZF | opc_SetSF),
            opc_WIG,
            opc_Kind_None,
            opc_Map_0F3A,
            OPC_MISC(opc_SupportsLOCKPrefix | opc_SetStatusFlagsForCMP),
            OPC_PREFIX(opc_VEX_PP_None),
            opc_EEVEX_ND1,
            opc_EEVEX_NF_None,
        },
        {
            // Operand 1
            { opnd_Reg, opnd_Int, opnd_Scalar, opnd_16, opnd_RW, opnd_MR_rm_B3, opnd_NotImm, opnd_Explicit, },

            // Operand 2
            { opnd_Mem, opnd_Int, opnd_Vector, opnd_256, opnd_R, opnd_ImmEnc, opnd_NotImm, opnd_Implicit, },
        },

        "ADD8RegImm8",
        "add",
    },

    // clang-format on
};

