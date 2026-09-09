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

    // ADD_r8_rm8_imm8  TBD
    // ADD_r8_rm8_imm8_NF  TBD

    // ADD16_Reg_Imm8s  TBD
    // ADD16_Mem_Imm8s  TBD
    // ADD16_Reg_Imm8s_NF  TBD
    // ADD16_Mem_Imm8s_NF  TBD
    // ADD32_Reg_Imm8s  TBD
    // ADD32_Mem_Imm8s  TBD
    // ADD32_Reg_Imm8s_NF  TBD
    // ADD32_Mem_Imm8s_NF  TBD
    // ADD64_Reg_Imm8s  TBD
    // ADD64_Mem_Imm8s  TBD
    // ADD64_Reg_Imm8s_NF  TBD
    // ADD64_Mem_Imm8s_NF  TBD
    // ADD16_Reg_Reg_Imm8s  TBD
    // ADD16_Reg_Mem_Imm8s  TBD
    // ADD16_Reg_Reg_Imm8s_NF  TBD
    // ADD16_Reg_Mem_Imm8s_NF  TBD
    // ADD32_Reg_Reg_Imm8s  TBD
    // ADD32_Reg_Mem_Imm8s  TBD
    // ADD32_Reg_Reg_Imm8s_NF  TBD
    // ADD32_Reg_Mem_Imm8s_NF  TBD
    // ADD64_Reg_Reg_Imm8s  TBD
    // ADD64_Reg_Mem_Imm8s  TBD
    // ADD64_Reg_Reg_Imm8s_NF  TBD
    // ADD64_Reg_Mem_Imm8s_NF  TBD

    // ADD16_Reg_Imm16  TBD
    // ADD16_Mem_Imm16  TBD
    // ADD16_Reg_Reg  TBD
    // ADD16_Mem_Reg  TBD
    // ADD16_Reg_Mem  TBD

    // ADD32_EAX_Imm32  TBD
    // ADD32_Reg_Imm32  TBD
    // ADD32_Mem_Imm32  TBD
    // ADD32_Reg_Reg  TBD
    // ADD32_Mem_Reg  TBD
    // ADD32_Reg_Mem  TBD

    // ADD64_RAX_Imm32s  TBD
    // ADD64_Reg_Imm32s  TBD
    // ADD64_Mem_Imm32s  TBD
    // ADD64_Reg_Reg  TBD
    // ADD64_Mem_Reg  TBD
    // ADD64_Reg_Mem  TBD

    // ADD_AL_imm8
    {
        {
        0x04, opc_Ext_None, opc_Map_0, opc_Prefix_NP, opc_WIG,
        OPC_ENCODING_PREFIXES(opc_Legacy),
        OPC_RFLAGS(opc_SetAllFlags),
        },
        {
        /* 1 */ { opnd_Reg, opnd_Int, opnd_Scalar, opnd_8, opnd_RW, opnd_ImpliedReg, opnd_NotImm, opnd_Implicit, },
        /* 2 */ { opnd_Imm, opnd_Int, opnd_Scalar, opnd_8, opnd_R, opnd_ImmEnc, opnd_NoExt, opnd_Explicit, },
        },
        "ADD_AL_imm8", "add",
    },

    // ADD_r8_imm8
    {
        {
        0x80, opc_Ext_0, opc_Map_0, opc_Prefix_NP, opc_WIG,
        OPC_ENCODING_PREFIXES(opc_Legacy | opc_REX | opc_REX2),
        OPC_RFLAGS(opc_SetAllFlags),
        },
        {
        /* 1 */ { opnd_Reg, opnd_Int, opnd_Scalar, opnd_8, opnd_RW, opnd_MR_rm_B3, opnd_NotImm, opnd_Explicit, },
        /* 2 */ { opnd_Imm, opnd_Int, opnd_Scalar, opnd_8, opnd_R, opnd_ImmEnc, opnd_NoExt, opnd_Explicit, },
        },
        "ADD_r8_imm8", "add",
    },

    // ADD_r8_imm8_NF
    {
        {
        0x80, opc_Ext_0, opc_Map_4, opc_VEX_PP_NP, opc_WIG,
        OPC_ENCODING_PREFIXES(opc_Legacy2EVEX),
        opc_NoRFlags,
        opc_NoFlags,
        opc_LLZ, opc_TupleType_NoScale, opc_EEVEX_ND0, opc_EEVEX_NF1, opc_PP_NP, 0x80,
        },
        {
        /* 1 */ { opnd_Reg, opnd_Int, opnd_Scalar, opnd_8, opnd_RW, opnd_MR_rm_B3, opnd_NotImm, opnd_Explicit, },
        /* 2 */ { opnd_Imm, opnd_Int, opnd_Scalar, opnd_8, opnd_R, opnd_ImmEnc, opnd_NoExt, opnd_Explicit, },
        },
        "ADD_r8_imm8_NF", "add{NF}",
    },

    // ADD_m8_imm8
    {
        {
        0x80, opc_Ext_0, opc_Map_0, opc_Prefix_NP, opc_WIG,
        OPC_ENCODING_PREFIXES(opc_Legacy | opc_REX | opc_REX2),
        OPC_RFLAGS(opc_SetAllFlags),
        },
        {
        /* 1 */ { opnd_Mem, opnd_Int, opnd_Scalar, opnd_8, opnd_RW, opnd_MR_rm_B3, opnd_NotImm, opnd_Explicit, },
        /* 2 */ { opnd_Imm, opnd_Int, opnd_Scalar, opnd_8, opnd_R, opnd_ImmEnc, opnd_NoExt, opnd_Explicit, },
        },
        "ADD_m8_imm8", "add",
    },

    // ADD_m8_imm8_NF
    {
        {
        0x80, opc_Ext_0, opc_Map_4, opc_VEX_PP_NP, opc_WIG,
        OPC_ENCODING_PREFIXES(opc_Legacy2EVEX),
        opc_NoRFlags,
        opc_NoFlags,
        opc_LLZ, opc_TupleType_NoScale, opc_EEVEX_ND0, opc_EEVEX_NF1, opc_PP_NP, 0x80,
        },
        {
        /* 1 */ { opnd_Mem, opnd_Int, opnd_Scalar, opnd_8, opnd_RW, opnd_MR_rm_B3, opnd_NotImm, opnd_Explicit, },
        /* 2 */ { opnd_Imm, opnd_Int, opnd_Scalar, opnd_8, opnd_R, opnd_ImmEnc, opnd_NoExt, opnd_Explicit, },
        },
        "ADD_m8_imm8_NF", "add{NF}",
    },

    // ADD_m8_r8
    {
        {
        0x00, opc_Ext_None, opc_Map_0, opc_Prefix_NP, opc_WIG,
        OPC_ENCODING_PREFIXES(opc_Legacy | opc_REX | opc_REX2),
        OPC_RFLAGS(opc_SetAllFlags),
        },
        {
        /* 1 */ { opnd_Mem, opnd_Int, opnd_Scalar, opnd_8, opnd_RW, opnd_MR_rm_B3, opnd_NotImm, opnd_Explicit, },
        /* 2 */ { opnd_Reg, opnd_Int, opnd_Scalar, opnd_8, opnd_R, opnd_MR_reg_R3, opnd_NotImm, opnd_Explicit, },
        },
        "ADD_m8_r8", "add",
    },

    // ADD_r8_rm8
    {
        {
        0x02, opc_Ext_None, opc_Map_0, opc_Prefix_NP, opc_WIG,
        OPC_ENCODING_PREFIXES(opc_Legacy | opc_REX | opc_REX2),
        OPC_RFLAGS(opc_SetAllFlags),
        },
        {
        /* 1 */ { opnd_Reg, opnd_Int, opnd_Scalar, opnd_8, opnd_RW, opnd_MR_reg_R3, opnd_NotImm, opnd_Explicit, },
        /* 2 */ { opnd_RM, opnd_Int, opnd_Scalar, opnd_8, opnd_R, opnd_MR_rm_B3, opnd_NotImm, opnd_Explicit, },
        },
        "ADD_r8_rm8", "add",
    },

    // ADD_m8_r8_NF
    {
        {
        0x00, opc_Ext_None, opc_Map_4, opc_VEX_PP_NP, opc_WIG,
        OPC_ENCODING_PREFIXES(opc_Legacy2EVEX),
        opc_NoRFlags,
        opc_NoFlags,
        opc_LLZ, opc_TupleType_NoScale, opc_EEVEX_ND0, opc_EEVEX_NF1, opc_PP_NP, 0x00,
        },
        {
        /* 1 */ { opnd_Mem, opnd_Int, opnd_Scalar, opnd_8, opnd_RW, opnd_MR_rm_B3, opnd_NotImm, opnd_Explicit, },
        /* 2 */ { opnd_Reg, opnd_Int, opnd_Scalar, opnd_8, opnd_R, opnd_MR_reg_R3, opnd_NotImm, opnd_Explicit, },
        },
        "ADD_m8_r8_NF", "add{NF}",
    },

    // ADD_r8_rm8_NF
    {
        {
        0x02, opc_Ext_None, opc_Map_4, opc_VEX_PP_NP, opc_WIG,
        OPC_ENCODING_PREFIXES(opc_Legacy2EVEX),
        opc_NoRFlags,
        opc_NoFlags,
        opc_LLZ, opc_TupleType_NoScale, opc_EEVEX_ND0, opc_EEVEX_NF1, opc_PP_NP, 0x02,
        },
        {
        /* 1 */ { opnd_Reg, opnd_Int, opnd_Scalar, opnd_8, opnd_RW, opnd_MR_reg_R3, opnd_NotImm, opnd_Explicit, },
        /* 2 */ { opnd_RM, opnd_Int, opnd_Scalar, opnd_8, opnd_R, opnd_MR_rm_B3, opnd_NotImm, opnd_Explicit, },
        },
        "ADD_r8_rm8_NF", "add{NF}",
    },

    // ADD_r8_rm8_r8
    {
        {
        0x00, opc_Ext_None, opc_Map_4, opc_VEX_PP_NP, opc_WIG,
        opc_Legacy2EVEX,
        OPC_RFLAGS(opc_SetAllFlags),
        opc_NoFlags,
        opc_LLZ, opc_TupleType_NoScale, opc_EEVEX_ND1, opc_EEVEX_NF0, opc_PP_NP, 0x00,
        },
        {
        /* 1 */ { opnd_Reg, opnd_Int, opnd_Scalar, opnd_8, opnd_W, opnd_vvvv, opnd_NotImm, opnd_Explicit, },
        /* 2 */ { opnd_RM, opnd_Int, opnd_Scalar, opnd_8, opnd_R, opnd_MR_rm_B3, opnd_NotImm, opnd_Explicit, },
        /* 3 */ { opnd_Reg, opnd_Int, opnd_Scalar, opnd_8, opnd_R, opnd_MR_reg_R3, opnd_NotImm, opnd_Explicit, },
        },
        "ADD_r8_rm8_r8", "add",
    },

    // ADD_r8_rm8_r8_NF  TBD

    // ADD_r8_r8_rm8
    {
        {
        0x02, opc_Ext_None, opc_Map_4, opc_VEX_PP_NP, opc_WIG,
        opc_Legacy2EVEX,
        OPC_RFLAGS(opc_SetAllFlags),
        opc_NoFlags,
        opc_LLZ, opc_TupleType_NoScale, opc_EEVEX_ND1, opc_EEVEX_NF0, opc_PP_NP, 0x02,
        },
        {
        /* 1 */ { opnd_Reg, opnd_Int, opnd_Scalar, opnd_8, opnd_W, opnd_vvvv, opnd_NotImm, opnd_Explicit, },
        /* 2 */ { opnd_Reg, opnd_Int, opnd_Scalar, opnd_8, opnd_R, opnd_MR_reg_R3, opnd_NotImm, opnd_Explicit, },
        /* 3 */ { opnd_RM, opnd_Int, opnd_Scalar, opnd_8, opnd_R, opnd_MR_rm_B3, opnd_NotImm, opnd_Explicit, },
        },
        "ADD_r8_r8_rm8", "add",
    },

    // ADD_r8_r8_rm8_NF  TBD

    // ADD_AX_imm16
    {
        {
        0x05, opc_Ext_None, opc_Map_0, opc_Prefix_66, opc_WIG,
        OPC_ENCODING_PREFIXES(opc_Legacy),
        OPC_RFLAGS(opc_SetAllFlags),
        },
        {
        /* 1 */ { opnd_Reg, opnd_Int, opnd_Scalar, opnd_16, opnd_RW, opnd_ImpliedReg, opnd_NotImm, opnd_Implicit, },
        /* 2 */ { opnd_Imm, opnd_Int, opnd_Scalar, opnd_16, opnd_R, opnd_ImmEnc, opnd_NoExt, opnd_Explicit, },
        },
        "ADD_AX_imm16", "add",
    },

    // ADD_EAX_imm32
    {
        {
        0x05, opc_Ext_None, opc_Map_0, opc_Prefix_NP, opc_WIG,
        OPC_ENCODING_PREFIXES(opc_Legacy),
        OPC_RFLAGS(opc_SetAllFlags),
        },
        {
        /* 1 */ { opnd_Reg, opnd_Int, opnd_Scalar, opnd_32, opnd_RW, opnd_ImpliedReg, opnd_NotImm, opnd_Implicit, },
        /* 2 */ { opnd_Imm, opnd_Int, opnd_Scalar, opnd_32, opnd_R, opnd_ImmEnc, opnd_NoExt, opnd_Explicit, },
        },
        "ADD_EAX_imm32", "add",
    },

    // ADD_RAX_imm32s
    {
        {
        0x05, opc_Ext_None, opc_Map_0, opc_Prefix_NP, opc_W1,
        OPC_ENCODING_PREFIXES(opc_REX),
        OPC_RFLAGS(opc_SetAllFlags),
        },
        {
        /* 1 */ { opnd_Reg, opnd_Int, opnd_Scalar, opnd_64, opnd_RW, opnd_ImpliedReg, opnd_NotImm, opnd_Implicit, },
        /* 2 */ { opnd_Imm, opnd_Int, opnd_Scalar, opnd_32, opnd_R, opnd_ImmEnc, opnd_SignExt, opnd_Explicit, },
        },
        "ADD_RAX_imm32s", "add",
    },

    // CRC32_r32_rm16  TBD
    // CRC32_r32_rm32  TBD
    // CRC32_r64_rm8  TBD
    // CRC32_r64_rm64  TBD

    // CRC32_r32_rm8
    {
        {
        0xF0, opc_Ext_None, opc_Map_2_0F38, opc_Prefix_F2, opc_WIG,
        OPC_ENCODING_PREFIXES(opc_Legacy | opc_REX | opc_Legacy2EVEX),
        opc_NoRFlags,
        opc_NoFlags,
        opc_LL_Scalar, opc_TupleType_NoScale, opc_EEVEX_ND0, opc_EEVEX_NF0, opc_PP_NP, 0xF0,
        },
        {
        /* 1 */ { opnd_Reg, opnd_Int, opnd_Scalar, opnd_32, opnd_RW, opnd_MR_reg_R3, opnd_NotImm, opnd_Explicit, },
        /* 2 */ { opnd_RM, opnd_Int, opnd_Scalar, opnd_8, opnd_R, opnd_MR_rm_B3, opnd_NotImm, opnd_Explicit, },
        },
        "CRC32_r32_rm8", "crc32",
    },

    // JMPABS_imm64
    {
        {
        0xA1, opc_Ext_None, opc_Map_0, opc_Prefix_NP, opc_WIG,
        OPC_ENCODING_PREFIXES(opc_REX2),
        opc_NoRFlags,
        OPC_FLAGS(opc_Kind_Branch),
        },
        {
        /* 1 */ { opnd_Imm, opnd_Int, opnd_Scalar, opnd_64, opnd_R, opnd_ImmEnc, opnd_NoExt, opnd_Explicit, },
        },
        "JMPABS_imm64", "jmpabs",
    },

    // KMOVB_k8_km8
    {
        {
        0x90, opc_Ext_None, opc_Map_1_0F, opc_VEX_PP_66, opc_W0,
        OPC_ENCODING_PREFIXES(opc_VEX2 | opc_VEX3 | opc_VEX2EVEX),
        opc_NoRFlags,
        opc_NoFlags,
        opc_LL_Scalar, opc_TupleType_NoScale, opc_EEVEX_ND0, opc_EEVEX_NF0, opc_PP_66, 0x90,
        },
        {
        /* 1 */ { opnd_Reg, opnd_Mask, opnd_Scalar, opnd_8, opnd_W, opnd_MR_reg_R3, opnd_NotImm, opnd_Explicit, },
        /* 2 */ { opnd_RM, opnd_Mask, opnd_Scalar, opnd_8, opnd_R, opnd_MR_rm_B3, opnd_NotImm, opnd_Explicit, },
        },
        "KMOVB_k8_km8", "kmovb",
    },

    // KMOVB_m8_k8
    {
        {
        0x91, opc_Ext_None, opc_Map_1_0F, opc_VEX_PP_66, opc_W0,
        OPC_ENCODING_PREFIXES(opc_VEX2 | opc_VEX3 | opc_VEX2EVEX),
        opc_NoRFlags,
        opc_NoFlags,
        opc_LL_Scalar, opc_TupleType_NoScale, opc_EEVEX_ND0, opc_EEVEX_NF0, opc_PP_66, 0x91,
        },
        {
        /* 1 */ { opnd_Mem, opnd_Mask, opnd_Scalar, opnd_8, opnd_W, opnd_MR_rm_B3, opnd_NotImm, opnd_Explicit, },
        /* 2 */ { opnd_Reg, opnd_Mask, opnd_Scalar, opnd_8, opnd_R, opnd_MR_reg_R3, opnd_NotImm, opnd_Explicit, },
        },
        "KMOVB_m8_k8", "kmovb",
    },

    // KMOVB_k8_r32
    {
        {
        0x92, opc_Ext_None, opc_Map_1_0F, opc_VEX_PP_66, opc_W0,
        OPC_ENCODING_PREFIXES(opc_VEX2 | opc_VEX3 | opc_VEX2EVEX),
        opc_NoRFlags,
        opc_NoFlags,
        opc_LL_Scalar, opc_TupleType_NoScale, opc_EEVEX_ND0, opc_EEVEX_NF0, opc_PP_66, 0x92,
        },
        {
        /* 1 */ { opnd_Reg, opnd_Mask, opnd_Scalar, opnd_8, opnd_W, opnd_MR_reg_R3, opnd_NotImm, opnd_Explicit, },
        /* 2 */ { opnd_Reg, opnd_Int, opnd_Scalar, opnd_32, opnd_R, opnd_MR_rm_B3, opnd_NotImm, opnd_Explicit, },
        },
        "KMOVB_k8_r32", "kmovb",
    },

    // KMOVB_r32_k8
    {
        {
        0x93, opc_Ext_None, opc_Map_1_0F, opc_VEX_PP_66, opc_W0,
        OPC_ENCODING_PREFIXES(opc_VEX2 | opc_VEX3 | opc_VEX2EVEX),
        opc_NoRFlags,
        opc_NoFlags,
        opc_LL_Scalar, opc_TupleType_NoScale, opc_EEVEX_ND0, opc_EEVEX_NF0, opc_PP_66, 0x93,
        },
        {
        /* 1 */ { opnd_Reg, opnd_Int, opnd_Scalar, opnd_32, opnd_W, opnd_MR_reg_R3, opnd_NotImm, opnd_Explicit, },
        /* 2 */ { opnd_Reg, opnd_Mask, opnd_Scalar, opnd_8, opnd_R, opnd_MR_rm_B3, opnd_NotImm, opnd_Explicit, },
        },
        "KMOVB_r32_k8", "kmovb",
    },

    // LZCNT_r16_rm16
    {
        {
        0xBD, opc_Ext_None, opc_Map_1_0F, OPC_PREFIX(opc_Prefix_F3 | opc_Prefix_66), opc_W0,
        OPC_ENCODING_PREFIXES(opc_Legacy | opc_REX | opc_REX2),
        OPC_RFLAGS(opc_SetZF | opc_SetCF),
        },
        {
        /* 1 */ { opnd_Reg, opnd_Int, opnd_Scalar, opnd_16, opnd_W, opnd_MR_reg_R3, opnd_NotImm, opnd_Explicit, },
        /* 2 */ { opnd_RM, opnd_Int, opnd_Scalar, opnd_16, opnd_R, opnd_MR_rm_B3, opnd_NotImm, opnd_Explicit, },
        },
        "LZCNT_r16_rm16", "lzcnt",
    },

    // LZCNT_r16_rm16_NF
    {
        {
        0xF5, opc_Ext_None, opc_Map_4, opc_VEX_PP_66, opc_W0,
        OPC_ENCODING_PREFIXES(opc_Legacy2EVEX),
        opc_NoRFlags,
        opc_NoFlags,
        opc_LLZ, opc_TupleType_NoScale, opc_EEVEX_ND0, opc_EEVEX_NF1, opc_PP_66, 0xF5,
        },
        {
        /* 1 */ { opnd_Reg, opnd_Int, opnd_Scalar, opnd_16, opnd_W, opnd_MR_reg_R3, opnd_NotImm, opnd_Explicit, },
        /* 2 */ { opnd_RM, opnd_Int, opnd_Scalar, opnd_16, opnd_R, opnd_MR_rm_B3, opnd_NotImm, opnd_Explicit, },
        },
        "LZCNT_r16_rm16_NF", "lzcnt{nf}",
    },

    // ADDPS_x128_xm128
    {
        {
        0x58, opc_Ext_None, opc_Map_1_0F, opc_Prefix_NP, opc_WIG,
        OPC_ENCODING_PREFIXES(opc_Legacy | opc_REX | opc_REX2),
        },
        {
        /* 1 */ { opnd_Reg, opnd_Float, opnd_Vector, opnd_128, opnd_RW, opnd_MR_reg_R3, opnd_NotImm, opnd_Explicit, },
        /* 2 */ { opnd_RM, opnd_Float, opnd_Vector, opnd_128, opnd_R, opnd_MR_rm_B3, opnd_NotImm, opnd_Explicit, },
        },
        "ADDPS_x128_xm128", "addps",
    },

    // VADDPS_x128_x128_xm128
    {
        {
        0x58, opc_Ext_None, opc_Map_1_0F, opc_Prefix_NP, opc_W0,
        OPC_ENCODING_PREFIXES(opc_VEX2 | opc_VEX3 | opc_EVEX),
        opc_NoRFlags,
        opc_NoFlags,
        opc_LL_128, opc_TupleType_FullMem, opc_EEVEX_ND0, opc_EEVEX_NF0, opc_PP_NP, 0x58,
        },
        {
        /* 1 */ { opnd_Reg, opnd_Float, opnd_Vector, opnd_128, opnd_W, opnd_MR_reg_R3, opnd_NotImm, opnd_Explicit, },
        /* 2 */ { opnd_Reg, opnd_Float, opnd_Vector, opnd_128, opnd_R, opnd_vvvv, opnd_NotImm, opnd_Explicit, },
        /* 3 */ { opnd_RM, opnd_Float, opnd_Vector, opnd_128, opnd_R, opnd_MR_rm_B3, opnd_NotImm, opnd_Explicit, },
        },
        "VADDPS_x128_x128_xm128", "vaddps",
    },

    // VADDPS_y256_y256_ym256
    {
        {
        0x58, opc_Ext_None, opc_Map_1_0F, opc_Prefix_NP, opc_W0,
        OPC_ENCODING_PREFIXES(opc_VEX2 | opc_VEX3 | opc_EVEX),
        opc_NoRFlags,
        opc_NoFlags,
        opc_LL_256, opc_TupleType_FullMem, opc_EEVEX_ND0, opc_EEVEX_NF0, opc_PP_NP, 0x58,
        },
        {
        /* 1 */ { opnd_Reg, opnd_Float, opnd_Vector, opnd_256, opnd_W, opnd_MR_reg_R3, opnd_NotImm, opnd_Explicit, },
        /* 2 */ { opnd_Reg, opnd_Float, opnd_Vector, opnd_256, opnd_R, opnd_vvvv, opnd_NotImm, opnd_Explicit, },
        /* 3 */ { opnd_RM, opnd_Float, opnd_Vector, opnd_256, opnd_R, opnd_MR_rm_B3, opnd_NotImm, opnd_Explicit, },
        },
        "VADDPS_y256_y256_ym256", "vaddps",
    },

    // VADDPS_z512_z512_zm512
    {
        {
        0x58, opc_Ext_None, opc_Map_1_0F, opc_Prefix_NP, opc_W0,
        OPC_ENCODING_PREFIXES(opc_EVEX),
        opc_NoRFlags,
        opc_NoFlags,
        opc_LL_512, opc_TupleType_FullMem, opc_EEVEX_ND0, opc_EEVEX_NF0, opc_PP_NP, 0x58,
        },
        {
        /* 1 */ { opnd_Reg, opnd_Float, opnd_Vector, opnd_512, opnd_W, opnd_MR_reg_R3, opnd_NotImm, opnd_Explicit, },
        /* 2 */ { opnd_Reg, opnd_Float, opnd_Vector, opnd_512, opnd_R, opnd_vvvv, opnd_NotImm, opnd_Explicit, },
        /* 3 */ { opnd_RM, opnd_Float, opnd_Vector, opnd_512, opnd_R, opnd_MR_rm_B3, opnd_NotImm, opnd_Explicit, },
        },
        "VADDPS_z512_z512_zm512", "vaddps",
    },

    // clang-format on
};

const OMR::X86::OpCodeAuxProperties OMR::X86::InstOpCode::_opCodeAuxProperties[] = {
    // clang-format off

    // ADD_AL_imm8
    OPC_AUXPROP(opc_SetStatusFlagsForTEST | opc_SetStatusFlagsForCMP),

    // ADD_r8_imm8
    OPC_AUXPROP(opc_SetStatusFlagsForTEST | opc_SetStatusFlagsForCMP),

    // ADD_r8_imm8_NF
    opc_AuxNone,

    // ADD_m8_imm8
    OPC_AUXPROP(opc_SupportsLOCKPrefix | opc_SetStatusFlagsForTEST | opc_SetStatusFlagsForCMP),

    // ADD_m8_imm8_NF
    opc_SupportsLOCKPrefix,

    // ADD_m8_r8
    OPC_AUXPROP(opc_SupportsLOCKPrefix | opc_SetStatusFlagsForTEST | opc_SetStatusFlagsForCMP),

    // ADD_r8_rm8
    OPC_AUXPROP(opc_SetStatusFlagsForTEST | opc_SetStatusFlagsForCMP),

    // ADD_m8_r8_NF
    opc_SupportsLOCKPrefix,

    // ADD_r8_rm8_NF
    opc_AuxNone,

    // ADD_r8_rm8_r8
    OPC_AUXPROP(opc_SetStatusFlagsForTEST | opc_SetStatusFlagsForCMP),

    // ADD_r8_r8_rm8
    OPC_AUXPROP(opc_SetStatusFlagsForTEST | opc_SetStatusFlagsForCMP),

    // ADD_AX_imm16
    OPC_AUXPROP(opc_SetStatusFlagsForTEST | opc_SetStatusFlagsForCMP),

    // ADD_EAX_imm32
    OPC_AUXPROP(opc_SetStatusFlagsForTEST | opc_SetStatusFlagsForCMP),

    // ADD_RAX_imm32s
    OPC_AUXPROP(opc_SetStatusFlagsForTEST | opc_SetStatusFlagsForCMP),

    // CRC32_r32_rm8
    opc_AuxNone,

    // JMPABS_imm64
    opc_AuxNone,

    // KMOVB_k8_km8
    opc_AuxNone,

    // KMOVB_m8_k8
    opc_AuxNone,

    // KMOVB_k8_r32
    opc_AuxNone,

    // KMOVB_r32_k8
    opc_AuxNone,

    // LZCNT_r16_rm16
    opc_AuxNone,

    // LZCNT_r16_rm16_NF
    opc_AuxNone,

    // ADDPS_x128_xm128
    opc_AuxNone,

    // VADDPS_x128_x128_xm128
    opc_AuxNone,

    // VADDPS_y256_y256_ym256
    opc_AuxNone,

    // VADDPS_z512_z512_zm512
    opc_AuxNone,

    // clang-format on
};

static_assert(sizeof(OMR::X86::InstOpCode::_opCodeTable) / sizeof(OMR::X86::InstOpCode::_opCodeTable[0])
        == sizeof(OMR::X86::InstOpCode::_opCodeAuxProperties) / sizeof(OMR::X86::InstOpCode::_opCodeAuxProperties[0]),
    "_opCodeTable and _opCodeAuxProperties must have the same number of elements");
