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

#ifndef OMR_X86_INSTRUCTION_INCL
#define OMR_X86_INSTRUCTION_INCL

/*
 * The following #define and typedef must appear before any #includes in this file
 */
#ifndef OMR_INSTRUCTION_CONNECTOR
#define OMR_INSTRUCTION_CONNECTOR

namespace OMR {
namespace X86 {
class Instruction;
}

typedef OMR::X86::Instruction InstructionConnector;
} // namespace OMR
#else
#error OMR::X86::Instruction expected to be a primary connector, but an OMR connector is already defined
#endif

#include "compiler/codegen/OMRInstruction.hpp"

#include <stddef.h>
#include <stdint.h>
#include "codegen/InstOpCode.hpp"
#include "codegen/RegisterConstants.hpp"
#include "codegen/InstOpCode.hpp"

namespace TR {
class X86ImmInstruction;
class X86LabelInstruction;
class X86RegInstruction;
class CodeGenerator;
class Instruction;
class MemoryReference;
class Node;
class Register;
class RegisterDependencyConditions;
} // namespace TR
struct TR_VFPState;

namespace OMR { namespace X86 {
class EnlargementResult {
    int32_t _patchGrowth; // the minimum guaratneed size of increase - we know we will grow this much
    int32_t _encodingGrowth; // the maximum size of increase - we have to accomodate growth of up to this much
public:
    EnlargementResult(int32_t patchGrowth, int32_t encodingGrowth)
        : _patchGrowth(patchGrowth)
        , _encodingGrowth(encodingGrowth)
    {}

    EnlargementResult(const EnlargementResult &other)
        : _patchGrowth(other._patchGrowth)
        , _encodingGrowth(other._encodingGrowth)
    {}

    int32_t getPatchGrowth() { return _patchGrowth; }

    int32_t getEncodingGrowth() { return _encodingGrowth; }
};

class OMR_EXTENSIBLE Instruction : public OMR::Instruction {
private:
    uint8_t _rexRepeatCount;
    OMR::X86::Encoding _encodingMethod;

protected:
#if defined(TR_TARGET_64BIT)
    void setRexRepeatCount(uint8_t count) { _rexRepeatCount = count; }
#else
    void setRexRepeatCount(uint8_t count)
    {
        TR_ASSERT(0, "Forcing a rex repetition does not make sense in IA32 mode!");
    }
#endif

protected:
    Instruction(TR::CodeGenerator *cg, OP::Mnemonic op, TR::Node *node, OMR::X86::Encoding encoding = Default);
    Instruction(TR::CodeGenerator *cg, TR::Instruction *precedingInstruction, OP::Mnemonic op, TR::Node *node = 0,
        OMR::X86::Encoding encoding = Default);
    void initialize(TR::CodeGenerator *cg = NULL, TR::RegisterDependencyConditions *cond = NULL,
        OP::Mnemonic op = OP::bad, bool flag = false);

public:
    virtual const char *description() { return "X86"; }

    virtual Kind getKind() { return IsNotExtended; }

    OMR_FINAL OMR::X86::Encoding getEncodingMethod() { return _encodingMethod; }

    void setEncodingMethod(OMR::X86::Encoding method) { _encodingMethod = method; }

    virtual bool isBranchOp() { return _opcode.isBranchOp() != 0; }

    virtual bool isLabel();
    virtual bool isRegRegMove();
    virtual bool isPatchBarrier(TR::CodeGenerator *cg);

    OMR_FINAL TR::RegisterDependencyConditions *getDependencyConditions() { return _conditions; }

    OMR_FINAL TR::RegisterDependencyConditions *setDependencyConditions(TR::RegisterDependencyConditions *cond)
    {
        return (_conditions = cond);
    }

    /**
     * @brief Queries whether instruction has been finalized prior to binary encoding
     *
     * @returns true if finalized; false otherwise
     */
    OMR_FINAL bool isFinalized() { return (_index & TO_MASK(IsFinalized)) != 0; }

    /**
     * @brief Indicates instruction has been finalized prior to binary encoding
     */
    OMR_FINAL void setIsFinalized() { _index |= TO_MASK(IsFinalized); }

    /**
     * @brief Indicates instruction has not been finalized prior to binary encoding
     */
    OMR_FINAL void resetIsFinalized() { _index &= ~TO_MASK(IsFinalized); }

    struct EncodingBits {
        union {
            uint32_t raw; // Direct access to all 32 bits

            struct {
                // ----------------------------------------------------------------
                // Byte 0: opcode or modRM (bits 0-7)
                // ----------------------------------------------------------------

                union {
                    union {
                        uint8_t opcode;

                        struct {
                            uint8_t opcodeReg: 3; // bits 0-2
                            uint8_t padding: 5; // bits 3-7
                        };
                    };

                    union {
                        uint8_t modRM;

                        struct {
                            uint8_t rm: 3; // bits 0-2
                            uint8_t reg: 3; // bits 3-5
                            uint8_t mod: 2; // bits 6-7
                        };
                    };
                };

                // ----------------------------------------------------------------
                // Byte 1: SIB (bits 8-15)
                // ----------------------------------------------------------------

                union {
                    uint8_t SIB;

                    struct {
                        uint8_t base: 3; // bits 8-10
                        uint8_t index: 3; // bits 11-13
                        uint8_t ss: 2; // bits 14-15
                    };
                };

                // ----------------------------------------------------------------
                // Byte 2: vvvv and EGPR bits (bits 16-23)
                // ----------------------------------------------------------------

                uint8_t vvvv: 4; // bits 16-19
                uint8_t V4: 1; // bit 20
                uint8_t R4: 1; // bit 21
                uint8_t X4: 1; // bit 22
                uint8_t B4: 1; // bit 23

                // ----------------------------------------------------------------
                // Byte 3: R3/X3/B3, W, and control flags (bits 24-31)
                // ----------------------------------------------------------------

                union {
                    struct {
                        uint8_t R3: 1; // bit 24
                        uint8_t X3: 1; // bit 25
                        uint8_t B3: 1; // bit 26
                        uint8_t W: 1; // bit 27
                        uint8_t needModRM: 1; // bit 28
                        uint8_t needSIBByte: 1; // bit 29
                        uint8_t requiresEGPR: 1; // bit 30
                        uint8_t reserved: 2; // bits 30-31 (unused)
                    };

                    struct {
                        uint8_t RXB: 3; // bits 24-26
                        uint8_t padding: 5; // bits 27-31
                    };
                };
            };
        };

        EncodingBits()
            : raw(0)
        {}

        // Constructor from raw value
        explicit EncodingBits(uint32_t value)
            : raw(value)
        {}

        // Reset all bits to zero
        inline void clear() { raw = 0; }

#if 0
        // Helper methods for grouped access

        inline uint8_t getRXB() const { return RXB; }

        inline void setRXB(uint8_t value) { RXB = value; }

        // Set individual R3, x3, B3 bits
        inline void setR3(bool value) { R3 = value ? 1 : 0; }

        inline void setX3(bool value) { x3 = value ? 1 : 0; }

        inline void setB3(bool value) { B3 = value ? 1 : 0; }

        // Get individual R3, x3, B3 bits
        inline bool getR3() const { return R3 != 0; }

        inline bool getX3() const { return x3 != 0; }

        inline bool getB3() const { return B3 != 0; }

        // Set modRM as a complete byte
        inline void setModRM(uint8_t value) { modRM = value; }

        // Get modRM as a complete byte
        inline uint8_t getModRM() const { return modRM; }

        // Set modRM using individual fields
        inline void setModRM(uint8_t mod_val, uint8_t reg_val, uint8_t rm_val)
        {
            mod = mod_val & 0x3; // 2 bits
            reg = reg_val & 0x7; // 3 bits
            rm = rm_val & 0x7; // 3 bits
        }

        // Set SIB as a complete byte
        inline void setSIB(uint8_t value) { SIB = value; }

        // Get SIB as a complete byte
        inline uint8_t getSIB() const { return SIB; }

        // Set SIB using individual fields
        inline void setSIB(uint8_t ss_val, uint8_t index_val, uint8_t base_val)
        {
            ss = ss_val & 0x3; // 2 bits
            index = index_val & 0x7; // 3 bits
            base = base_val & 0x7; // 3 bits
        }

        // Set vvvv as a 4-bit unit
        inline void setVVVV(uint8_t value)
        {
            vvvv = value & 0xF; // Mask to 4 bits
        }

        // Get vvvv as a 4-bit unit
        inline uint8_t getVVVV() const { return vvvv; }
#endif
    };

    static_assert(sizeof(EncodingBits) == 4, "EncodingBits must be exactly 4 bytes");

    /**
     * @brief
     *     Perform any final processing of a TR::Instruction prior to generating
     *     its binary encoding. In practice, an instruction should be finalized
     *     before binary length estimation, which is considered a necessary step
     *     of binary encoding.
     *
     *     Every instruction must be finalized prior to generating its binary
     *     encoding. Downstream projects may specialize this function.
     */
    void finalizeBeforeBinaryEncoding();

    /**
     * @brief
     *     Perform any final processing of a TR::Instruction's operands prior
     *     to generating its binary encoding.
     */
    virtual void finalizeOperands() {}

    virtual void assignRegisters(TR_RegisterKinds kindsToBeAssigned);
    virtual bool refsRegister(TR::Register *reg);
    virtual bool defsRegister(TR::Register *reg);
    virtual bool usesRegister(TR::Register *reg);
    virtual bool dependencyRefsRegister(TR::Register *reg);

    virtual uint8_t *generateBinaryEncoding();
    virtual int32_t estimateBinaryLength(int32_t currentEstimate);

    virtual uint8_t *generateOperand(uint8_t *cursor) { return cursor; }

    virtual bool needsRepPrefix();
    virtual bool needsLockPrefix();

    virtual EnlargementResult enlarge(int32_t requestedEnlargementSize, int32_t maxEnlargementSize,
        bool allowPartialEnlargement)
    {
        return EnlargementResult(0, 0);
    }

    virtual TR::X86RegInstruction *getX86RegInstruction() { return NULL; }

    virtual TR::X86LabelInstruction *getX86LabelInstruction() { return NULL; }

#if defined(DEBUG) || defined(PROD_WITH_ASSUMES)
    // The following safe virtual downcast method is used under debug only
    // for assertion checking.
    //
    virtual TR::X86ImmInstruction *getX86ImmInstruction() { return NULL; }

    virtual uint32_t getNumOperandReferencedGPRegisters() { return 0; }

    uint32_t totalReferencedGPRegisters(TR::CodeGenerator *);
#endif

    // AMD64-specific REX prefix calculations
    // Note the polymorphism
    //

    // Number of unnecessary REX bytes for instruction expansion and/or padding
    uint8_t rexRepeatCount();
    // Only repeated REX bytes for expansion/padding are generated
    OMR_FINAL uint8_t *generateRepeatedRexPrefix(uint8_t *cursor);

#if defined(TR_TARGET_64BIT)
    uint8_t operandSizeRexBits();
    // Each subclass should override this as necessary
    virtual uint8_t rexBits();
#else
    OMR_FINAL uint8_t rexBits() { return 0; }
#endif

    // Adjust the VFP state to reflect the execution of this instruction.
    //
    virtual void adjustVFPState(TR_VFPState *state, TR::CodeGenerator *cg);

    virtual TR::Register *getTargetRegister() { return NULL; }

    virtual TR::Register *getSourceRegister() { return NULL; }

    virtual TR::Register *getSource2ndRegister() { return NULL; }

    virtual TR::Register *getMaskRegister() { return NULL; }

    virtual TR::MemoryReference *getMemoryReference() { return NULL; }

    virtual bool hasZeroMask() { return false; }

    int32_t getMaxPatchableInstructionLength() { return 10; }

protected:
    // Facility for subclasses to compute effect of a call on the VFP state.
    // If _opcode->isCallOp(), adds vfpAdjustmentForCall; else, calls TR::Instruction ::adjustVFPState.
    //
    void adjustVFPStateForCall(TR_VFPState *state, int32_t vfpAdjustmentForCall, TR::CodeGenerator *cg);
    void clobberRegsForRematerialisation();

private:
    TR::RegisterDependencyConditions *_conditions;
    void assumeValidInstruction();

public:
    struct ModRM {
        uint8_t rm: 3;
        uint8_t reg: 3;
        uint8_t mod: 2;

        inline ModRM() {}

        inline ModRM(uint8_t opcode)
        {
            rm = 0;
            reg = opcode;
            mod = 0x3;
        }

        inline ModRM(const ModRM &other)
        {
            rm = other.rm;
            reg = other.reg;
            mod = other.mod;
        }

        inline operator uint8_t() const { return *((uint8_t *)this); }

        inline uint8_t Reg(uint8_t R = 0) const { return (R << 3) | (0x7 & reg); }

        inline uint8_t RM(uint8_t B = 0) const
        {
            TR_ASSERT(mod == 0x3, "ModRM is not in register mode");
            return (B << 3) | (0x7 & rm);
        }

        inline ModRM *setMod(uint8_t mod = 0x03) // 0b11
        {
            this->mod = mod;
            return this;
        }

        inline ModRM *setBase()
        {
            return setMod(0x00); // 0b00
        }

        inline ModRM *setBaseDisp8()
        {
            return setMod(0x01); // 0b01
        }

        inline ModRM *setBaseDisp32()
        {
            return setMod(0x02); // 0b10
        }

        inline ModRM *setIndexOnlyDisp32()
        {
            rm = 0x05; // 0b101
            return setMod(0x00); // 0b00
        }

        inline ModRM *setHasSIB()
        {
            rm = 0x04; // 0b100
            return this;
        }
    };

    struct SIB {
        uint8_t base: 3;
        uint8_t index: 3;
        uint8_t scale: 2;

        inline operator uint8_t() const { return *((uint8_t *)this); }

        inline SIB *setScale(uint8_t scale = 0)
        {
            this->scale = scale;
            return this;
        }

        inline SIB *setNoIndex()
        {
            index = 0x04; // 0b100
            return this;
        }

        inline SIB *setIndexDisp32()
        {
            base = 0x05; // 0b101
            return this;
        }
    };

    struct REX {
        uint8_t B: 1;
        uint8_t X: 1;
        uint8_t R: 1;
        uint8_t W: 1;
        uint8_t _padding: 4;

        inline REX(uint8_t val = 0)
        {
            *((uint8_t *)this) = val;
            _padding = 0x4;
        }

        inline uint8_t value() const { return 0x0f & *((uint8_t *)this); }
    };

    template<size_t VEX_SIZE> struct VEX {
        VEX() { TR_ASSERT_FATAL(false, "INVALID VEX PREFIX"); }
    };

    struct EVEX {
        // 0x62 P0 P1 P2
        // Byte 0: 0x62
        uint8_t escape;
        // Byte 1 : P0
        uint8_t mm: 2; // P0[0] : m0, m1
        uint8_t zero: 2; // P0[2] : 0b00
        uint8_t r: 1; // P0[4] : R'
        uint8_t B: 1; // P0[5] : B
        uint8_t X: 1; // P0[6] : X
        uint8_t R: 1; // P0[7] : R
        // Byte 2 : P1
        uint8_t p: 2; // P1[0] : p1, p0
        uint8_t one: 1; // P1[2] : 0b1
        uint8_t v: 4; // P1[3] : v0, v1, v2, v3
        uint8_t W: 1; // P1[7] : W

        // Byte 3 : P2
        uint8_t a: 3; // P2[0] : a0, a1, a2 -- write mask {k1-k7}
        uint8_t V: 1; // P2[3] : V'
        uint8_t b: 1; // P2[4] : b
        uint8_t L: 2; // P2[5] : L, L'
        uint8_t Z: 1; // P2[7] : z
        // Byte 4: opcode
        uint8_t opcode;

        inline EVEX() {}

        inline EVEX(const REX &rex, uint8_t ModRMOpCode)
        {
            escape = '\x62';
            // reserved bits
            one = 1;
            zero = 0;
            Z = 0;
            b = 0;

            R = ~rex.R;
            X = ~rex.X;
            B = ~rex.B;

            ModRM modrm(ModRMOpCode);
            r = ~(rex.R & modrm.reg);

            W = rex.W;
            v = 0xf; // 0b1111
            V = v >> 3;
            a = 0;
        }

        inline uint8_t Reg(const ModRM modrm) const { return modrm.Reg(~R); }

        inline uint8_t RM(const ModRM modrm) const { return modrm.RM(~B); }
    };
};

template<> struct Instruction::VEX<3> {
    // Byte 0: C4
    uint8_t escape;
    // Byte 1
    uint8_t m: 5;
    uint8_t B: 1;
    uint8_t X: 1;
    uint8_t R: 1;
    // Byte 2
    uint8_t p: 2;
    uint8_t L: 1;
    uint8_t v: 4;
    uint8_t W: 1;
    // Byte 3: opcode
    uint8_t opcode;

    inline VEX() {}

    inline VEX(const REX &rex)
    {
        escape = '\xC4';
        R = ~rex.R;
        X = ~rex.X;
        B = ~rex.B;
        W = rex.W;
        v = 0xf; // 0b1111
    }

    inline bool CanBeShortened() const { return X && B && !W && (m == 1); }

    inline uint8_t Reg(const ModRM modrm) const { return modrm.Reg(~R); }

    inline uint8_t RM(const ModRM modrm) const { return modrm.RM(~B); }
};

template<> struct Instruction::VEX<2> {
    // Byte 0: C5
    uint8_t escape;
    // Byte 1
    uint8_t p: 2;
    uint8_t L: 1;
    uint8_t v: 4;
    uint8_t R: 1;
    // Byte 2: opcode
    uint8_t opcode;

    inline VEX() {}

    inline VEX(const VEX<3> &other)
    {
        escape = '\xC5';
        p = other.p;
        L = other.L;
        v = other.v;
        R = other.R;
        opcode = other.opcode;
    }

    inline uint8_t Reg(const ModRM modrm) const { return modrm.Reg(~R); }

    inline uint8_t RM(const ModRM modrm) const { return modrm.RM(); }
};
}} // namespace OMR::X86

#endif
