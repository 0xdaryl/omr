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
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0 WITH Classpath-exception-2.0 OR LicenseRef-GPL-2.0 WITH Assembly-exception
 *******************************************************************************/

/*  __   ___  __   __   ___  __       ___  ___  __
 * |  \ |__  |__) |__) |__  /  `  /\   |  |__  |  \
 * |__/ |___ |    |  \ |___ \__, /~~\  |  |___ |__/
 *
 * This file is now deprecated and its contents are slowly
 * being moved back to codegen and other directories. Please do not
 * add more interfaces here.
 */

#ifndef DEBUG_INCL
#define DEBUG_INCL

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "codegen/Machine.hpp"
#include "codegen/RegisterConstants.hpp"
#include "compile/Method.hpp"
#include "compile/VirtualGuard.hpp"
#include "cs2/hashtab.h"
#include "env/RawAllocator.hpp"
#include "env/TRMemory.hpp"
#include "env/jittypes.h"
#include "il/DataTypes.hpp"
#include "il/ILOpCodes.hpp"
#include "il/ILOps.hpp"
#include "infra/Assert.hpp"
#include "infra/BitVector.hpp"
#include "infra/TRlist.hpp"
#include "optimizer/Optimizations.hpp"
#include "runtime/Runtime.hpp"
#include "infra/CfgNode.hpp"

#include "codegen/RegisterRematerializationInfo.hpp"

class TR_Debug;
class TR_BlockStructure;
class TR_CHTable;
namespace TR { class CompilationFilters; }
class TR_FilterBST;
class TR_FrontEnd;
class TR_GCStackMap;
class TR_InductionVariable;
class TR_PrettyPrinterString;
class TR_PseudoRandomNumbersListElement;
class TR_RegionAnalysis;
class TR_RegionStructure;
class TR_RematerializationInfo;
class TR_ResolvedMethod;
class TR_Structure;
class TR_StructureSubGraphNode;
namespace TR { class VPConstraint; }
namespace TR { class GCStackAtlas; }
namespace TR { class AutomaticSymbol; }
namespace TR { class Block; }
namespace TR { class CFG; }
namespace TR { class CFGEdge; }
namespace TR { class CFGNode; }
namespace TR { class CodeGenerator; }
namespace TR { class Compilation; }
namespace TR { class DebugCounterGroup; }
namespace TR { class GCRegisterMap; }
namespace TR { class InstOpCode; }
namespace TR { class Instruction; }
namespace TR { class LabelSymbol; }
namespace TR { class Logger; }
namespace TR { class Machine; }
namespace TR { class MemoryReference; }
namespace TR { class Node; }
namespace TR { class OptionSet; }
namespace TR { class Options; }
namespace TR { class RealRegister; }
namespace TR { class Register; }
namespace TR { class RegisterDependencyGroup; }
namespace TR { class RegisterDependency; }
namespace TR { class RegisterDependencyConditions; }
namespace TR { class RegisterMappedSymbol; }
namespace TR { class ResolvedMethodSymbol; }
namespace TR { class SimpleRegex; }
namespace TR { class Snippet; }
namespace TR { class Symbol; }
namespace TR { class SymbolReference; }
namespace TR { class SymbolReferenceTable; }
namespace TR { class TreeTop; }
namespace TR { struct OptionTable; }
template <class T> class List;
template <class T> class ListIterator;

struct J9JITExceptionTable;
struct J9AnnotationInfo;
struct J9AnnotationInfoEntry;
struct J9PortLibrary;

namespace TR { class X86LabelInstruction;                  }
namespace TR { class X86PaddingInstruction;                }
namespace TR { class X86AlignmentInstruction;              }
namespace TR { class X86BoundaryAvoidanceInstruction;      }
namespace TR { class X86PatchableCodeAlignmentInstruction; }
namespace TR { class X86FenceInstruction;                  }
namespace TR { class X86VirtualGuardNOPInstruction;        }
namespace TR { class X86ImmInstruction;                    }
namespace TR { class X86ImmSnippetInstruction;             }
namespace TR { class X86ImmSymInstruction;                 }
namespace TR { class X86RegInstruction;                    }
namespace TR { class X86RegRegInstruction;                 }
namespace TR { class X86RegImmInstruction;                 }
namespace TR { class X86RegRegImmInstruction;              }
namespace TR { class X86RegRegRegInstruction;              }
namespace TR { class X86RegMaskRegRegInstruction;          }
namespace TR { class X86RegMaskRegRegImmInstruction;       }
namespace TR { class X86RegMaskRegInstruction;             }
namespace TR { class X86RegMaskMemInstruction;             }
namespace TR { class X86MemInstruction;                    }
namespace TR { class X86MemImmInstruction;                 }
namespace TR { class X86MemRegInstruction;                 }
namespace TR { class X86MemMaskRegInstruction;             }
namespace TR { class X86MemRegImmInstruction;              }
namespace TR { class X86RegMemInstruction;                 }
namespace TR { class X86RegMemImmInstruction;              }
namespace TR { class X86RegRegMemInstruction;              }
namespace TR { class X86FPRegInstruction;                  }
namespace TR { class X86FPRegRegInstruction;               }
namespace TR { class X86FPMemRegInstruction;               }
namespace TR { class X86FPRegMemInstruction;               }
namespace TR { class X86RestartSnippet; }
namespace TR { class X86PicDataSnippet; }
namespace TR { class X86DivideCheckSnippet; }
namespace TR { class X86FPConvertToIntSnippet; }
namespace TR { class X86FPConvertToLongSnippet; }
namespace TR { class X86GuardedDevirtualSnippet; }
namespace TR { class X86HelperCallSnippet; }
namespace TR { class UnresolvedDataSnippet; }
namespace TR { class AMD64Imm64Instruction;    }
namespace TR { class AMD64Imm64SymInstruction; }
namespace TR { class AMD64RegImm64Instruction; }

struct TR_VFPState;
namespace TR { class X86VFPSaveInstruction;        }
namespace TR { class X86VFPRestoreInstruction;     }
namespace TR { class X86VFPDedicateInstruction;    }
namespace TR { class X86VFPReleaseInstruction;     }
namespace TR { class X86VFPCallCleanupInstruction; }

#ifdef J9_PROJECT_SPECIFIC
namespace TR { class X86CallSnippet; }
namespace TR { class X86CheckFailureSnippet; }
namespace TR { class X86CheckFailureSnippetWithResolve; }
namespace TR { class X86BoundCheckWithSpineCheckSnippet; }
namespace TR { class X86SpineCheckSnippet; }
namespace TR { class X86ForceRecompilationSnippet; }
namespace TR { class X86RecompilationSnippet; }
#endif

namespace TR { class PPCAlignmentNopInstruction;         }
namespace TR { class PPCDepInstruction;                  }
namespace TR { class PPCLabelInstruction;                }
namespace TR { class PPCDepLabelInstruction;             }
namespace TR { class PPCConditionalBranchInstruction;    }
namespace TR { class PPCDepConditionalBranchInstruction; }
namespace TR { class PPCAdminInstruction;                }
namespace TR { class PPCImmInstruction;                  }
namespace TR { class PPCSrc1Instruction;                 }
namespace TR { class PPCDepImmSymInstruction;            }
namespace TR { class PPCTrg1Instruction;                 }
namespace TR { class PPCTrg1Src1Instruction;             }
namespace TR { class PPCTrg1ImmInstruction;              }
namespace TR { class PPCTrg1Src1ImmInstruction;          }
namespace TR { class PPCTrg1Src1Imm2Instruction;         }
namespace TR { class PPCSrc2Instruction;                 }
namespace TR { class PPCSrc3Instruction;                 }
namespace TR { class PPCTrg1Src2Instruction;             }
namespace TR { class PPCTrg1Src2ImmInstruction;          }
namespace TR { class PPCTrg1Src3Instruction;             }
namespace TR { class PPCMemSrc1Instruction;              }
namespace TR { class PPCMemInstruction;                  }
namespace TR { class PPCTrg1MemInstruction;              }
namespace TR { class PPCControlFlowInstruction;          }
namespace TR { class PPCVirtualGuardNOPInstruction;      }
namespace TR { class PPCUnresolvedCallSnippet; }
namespace TR { class PPCVirtualSnippet; }
namespace TR { class PPCVirtualUnresolvedSnippet; }
namespace TR { class PPCInterfaceCallSnippet; }
namespace TR { class PPCHelperCallSnippet; }
namespace TR { class PPCMonitorEnterSnippet; }
namespace TR { class PPCMonitorExitSnippet; }
namespace TR { class PPCReadMonitorSnippet; }

namespace TR { class PPCAllocPrefetchSnippet; }

namespace TR { class PPCLockReservationEnterSnippet; }
namespace TR { class PPCLockReservationExitSnippet; }
namespace TR { class PPCArrayCopyCallSnippet; }

#ifdef J9_PROJECT_SPECIFIC
namespace TR { class PPCInterfaceCastSnippet; }
namespace TR { class PPCStackCheckFailureSnippet; }
namespace TR { class PPCForceRecompilationSnippet; }
namespace TR { class PPCRecompilationSnippet; }
namespace TR { class PPCCallSnippet; }
#endif


namespace TR { class ARMLabelInstruction; }
namespace TR { class ARMConditionalBranchInstruction; }
namespace TR { class ARMVirtualGuardNOPInstruction; }
namespace TR { class ARMAdminInstruction; }
namespace TR { class ARMImmInstruction; }
namespace TR { class ARMImmSymInstruction; }
namespace TR { class ARMTrg1Src2Instruction; }
namespace TR { class ARMTrg2Src1Instruction; }
namespace TR { class ARMMulInstruction; }
namespace TR { class ARMMemSrc1Instruction; }
namespace TR { class ARMTrg1Instruction; }
namespace TR { class ARMMemInstruction; }
namespace TR { class ARMTrg1MemInstruction; }
namespace TR { class ARMTrg1MemSrc1Instruction; }
namespace TR { class ARMControlFlowInstruction; }
namespace TR { class ARMMultipleMoveInstruction; }
class TR_ARMMemoryReference;
class TR_ARMOperand2;
class TR_ARMRealRegister;
namespace TR { class ARMCallSnippet; }
namespace TR { class ARMUnresolvedCallSnippet; }
namespace TR { class ARMVirtualSnippet; }
namespace TR { class ARMVirtualUnresolvedSnippet; }
namespace TR { class ARMInterfaceCallSnippet; }
namespace TR { class ARMHelperCallSnippet; }
namespace TR { class ARMMonitorEnterSnippet; }
namespace TR { class ARMMonitorExitSnippet; }
namespace TR { class ARMStackCheckFailureSnippet; }
namespace TR { class ARMRecompilationSnippet; }

namespace TR { class S390LabelInstruction; }
namespace TR { class S390BranchInstruction; }
namespace TR { class S390BranchOnCountInstruction; }
namespace TR { class S390VirtualGuardNOPInstruction; }
namespace TR { class S390BranchOnIndexInstruction; }
namespace TR { class S390AnnotationInstruction; }
namespace TR { class S390PseudoInstruction; }
namespace TR { class S390ImmInstruction; }
namespace TR { class S390ImmSnippetInstruction; }
namespace TR { class S390ImmSymInstruction; }
namespace TR { class S390Imm2Instruction; }
namespace TR { class S390RegInstruction; }
namespace TR { class S390RRInstruction; }
namespace TR { class S390TranslateInstruction; }
namespace TR { class S390RRFInstruction; }
namespace TR { class S390RRRInstruction; }
namespace TR { class S390RXFInstruction; }
namespace TR { class S390RIInstruction; }
namespace TR { class S390RILInstruction; }
namespace TR { class S390RSInstruction; }
namespace TR { class S390RSLInstruction; }
namespace TR { class S390RSLbInstruction; }
namespace TR { class S390RXEInstruction; }
namespace TR { class S390RXInstruction; }
namespace TR { class S390MemInstruction; }
namespace TR { class S390SS1Instruction; }
namespace TR { class S390MIIInstruction; }
namespace TR { class S390SMIInstruction; }
namespace TR { class S390SS2Instruction; }
namespace TR { class S390SS4Instruction; }
namespace TR { class S390SSEInstruction; }
namespace TR { class S390SSFInstruction; }
namespace TR { class S390VRIInstruction; }
namespace TR { class S390VRIaInstruction; }
namespace TR { class S390VRIbInstruction; }
namespace TR { class S390VRIcInstruction; }
namespace TR { class S390VRIdInstruction; }
namespace TR { class S390VRIeInstruction; }
namespace TR { class S390VRRInstruction; }
namespace TR { class S390VRRaInstruction; }
namespace TR { class S390VRRbInstruction; }
namespace TR { class S390VRRcInstruction; }
namespace TR { class S390VRRdInstruction; }
namespace TR { class S390VRReInstruction; }
namespace TR { class S390VRRfInstruction; }
namespace TR { class S390VRSaInstruction; }
namespace TR { class S390VRSbInstruction; }
namespace TR { class S390VRScInstruction; }
namespace TR { class S390VRVInstruction; }
namespace TR { class S390VRXInstruction; }
namespace TR { class S390VStorageInstruction; }
namespace TR { class S390OpCodeOnlyInstruction; }
namespace TR { class S390IInstruction; }
namespace TR { class S390SInstruction; }
namespace TR { class S390SIInstruction; }
namespace TR { class S390SILInstruction; }
namespace TR { class S390NOPInstruction; }
namespace TR { class S390AlignmentNopInstruction; }
namespace TR { class S390RestoreGPR7Snippet; }
namespace TR { class S390CallSnippet; }
namespace TR { class S390ConstantDataSnippet; }
namespace TR { class S390WritableDataSnippet; }
namespace TR { class S390HelperCallSnippet; }
namespace TR { class S390JNICallDataSnippet; }

namespace TR { class S390StackCheckFailureSnippet; }
namespace TR { class S390HeapAllocSnippet; }
namespace TR { class S390RRSInstruction; }
namespace TR { class S390RIEInstruction; }
namespace TR { class S390RISInstruction; }
namespace TR { class S390IEInstruction; }

#ifdef J9_PROJECT_SPECIFIC
namespace TR { class S390ForceRecompilationSnippet; }
namespace TR { class S390ForceRecompilationDataSnippet; }
namespace TR { class S390J9CallSnippet; }
namespace TR { class S390UnresolvedCallSnippet; }
namespace TR { class S390VirtualSnippet; }
namespace TR { class S390VirtualUnresolvedSnippet; }
namespace TR { class S390InterfaceCallSnippet; }
namespace TR { class J9S390InterfaceCallDataSnippet; }
#endif

namespace TR { class ARM64ImmInstruction; }
namespace TR { class ARM64RelocatableImmInstruction; }
namespace TR { class ARM64ImmSymInstruction; }
namespace TR { class ARM64LabelInstruction; }
namespace TR { class ARM64ConditionalBranchInstruction; }
namespace TR { class ARM64CompareBranchInstruction; }
namespace TR { class ARM64TestBitBranchInstruction; }
namespace TR { class ARM64RegBranchInstruction; }
namespace TR { class ARM64AdminInstruction; }
namespace TR { class ARM64Trg1Instruction; }
namespace TR { class ARM64Trg1CondInstruction; }
namespace TR { class ARM64Trg1ImmInstruction; }
namespace TR { class ARM64Trg1ImmShiftedInstruction; }
namespace TR { class ARM64Trg1ImmSymInstruction; }
namespace TR { class ARM64Trg1Src1Instruction; }
namespace TR { class ARM64Trg1ZeroSrc1Instruction; }
namespace TR { class ARM64Trg1ZeroImmInstruction; }
namespace TR { class ARM64Trg1Src1ImmInstruction; }
namespace TR { class ARM64Trg1Src2Instruction; }
namespace TR { class ARM64CondTrg1Src2Instruction; }
namespace TR { class ARM64Trg1Src2ImmInstruction; }
namespace TR { class ARM64Trg1Src2ShiftedInstruction; }
namespace TR { class ARM64Trg1Src2ExtendedInstruction; }
namespace TR { class ARM64Trg1Src2IndexedElementInstruction; }
namespace TR { class ARM64Trg1Src2ZeroInstruction; }
namespace TR { class ARM64Trg1Src3Instruction; }
namespace TR { class ARM64Trg1MemInstruction; }
namespace TR { class ARM64Trg2MemInstruction; }
namespace TR { class ARM64MemInstruction; }
namespace TR { class ARM64MemImmInstruction; }
namespace TR { class ARM64MemSrc1Instruction; }
namespace TR { class ARM64MemSrc2Instruction; }
namespace TR { class ARM64Trg1MemSrc1Instruction; }
namespace TR { class ARM64Src1Instruction; }
namespace TR { class ARM64ZeroSrc1ImmInstruction; }
namespace TR { class ARM64Src2Instruction; }
namespace TR { class ARM64ZeroSrc2Instruction; }
namespace TR { class ARM64Src1ImmCondInstruction; }
namespace TR { class ARM64Src2CondInstruction; }
namespace TR { class ARM64HelperCallSnippet; }

namespace TR { class LabelInstruction; }
namespace TR { class AdminInstruction; }
#ifdef J9_PROJECT_SPECIFIC
namespace TR { class ARM64VirtualGuardNOPInstruction; }

namespace TR { class ARM64InterfaceCallSnippet; }
namespace TR { class ARM64StackCheckFailureSnippet; }
namespace TR { class ARM64ForceRecompilationSnippet; }
namespace TR { class ARM64RecompilationSnippet; }
namespace TR { class ARM64CallSnippet; }
namespace TR { class ARM64UnresolvedCallSnippet; }
namespace TR { class ARM64VirtualUnresolvedSnippet; }
#endif

namespace TR { class DataInstruction; }
namespace TR { class RtypeInstruction; }
namespace TR { class ItypeInstruction; }
namespace TR { class StypeInstruction; }
namespace TR { class BtypeInstruction; }
namespace TR { class UtypeInstruction; }
namespace TR { class JtypeInstruction; }
namespace TR { class LoadInstruction;  }
namespace TR { class StoreInstruction; }
namespace TR { class RVHelperCallSnippet; }

TR_Debug *createDebugObject(TR::Compilation *);


class TR_Debug
   {
public:

   void * operator new (size_t s, TR_HeapMemory m);
   void * operator new(size_t s, TR::PersistentAllocator &allocator);

   TR_Debug(TR::Compilation * c);

   TR::FILE *getFile() {return _file;}
   virtual void   setFile(TR::FILE *f) {_file = f;}

   TR::Logger *getLogger() { return _logger; }
   void setLogger(TR::Logger *log) { _logger = log; }

   virtual void resetDebugData();
   virtual void newNode(TR::Node *);
   virtual void newLabelSymbol(TR::LabelSymbol *);
   virtual void newRegister(TR::Register *);
   virtual void newVariableSizeSymbol(TR::AutomaticSymbol *sym);
   virtual void newInstruction(TR::Instruction *);
   virtual void addInstructionComment(TR::Instruction *, char*, ...);

   /**
    * @brief Check whether to trigger debugger breakpoint or launch debugger
    *        when \c artifact name is created.
    *
    * @param[in] artifactName : name of artifact to break or debug on
    */
   void breakOrDebugOnCreate(char *artifactName);

   virtual TR::CompilationFilters * getInlineFilters() { return _inlineFilters; }

   virtual TR_FrontEnd *fe() { return _fe; }
   virtual TR::Compilation *comp() { return _comp; }

   virtual char *formattedString(char *buf, uint32_t bufLen, const char *format, va_list args, TR_AllocationKind=heapAlloc);

   virtual TR::Node     *getCurrentParent() { return _currentParent; }
   virtual void        setCurrentParentAndChildIndex(TR::Node *n, int32_t i) { _currentParent = n; _currentChildIndex=i; }
   virtual void        setCurrentParent(TR::Node *n) { _currentParent = n;}

   virtual int32_t     getCurrentChildIndex() { return _currentChildIndex; }
   virtual void        setCurrentChildIndex(int32_t i) { _currentChildIndex = i;}

   // Print current stack back trace to standard error
   static void printStackBacktrace();

   // Print current stack back trace to trace log
   // Argument: comp is the current compilation object, which cannot be NULL
   static void printStackBacktraceToTraceLog(TR::Compilation* comp);

   void breakOn();

   void debugOnCreate();

   // Options processing
   //
   virtual TR::FILE *      findLogFile(TR::Options *aotCmdLineOptions, TR::Options *jitCmdLineOptions, TR::OptionSet *optSet, char *logFileName, TR::Logger *&logger);
   virtual int32_t         findLogFile(const char *logFileName, TR::Options *aotCmdLineOptions, TR::Options *jitCmdLineOptions, TR::Options **optionsArray, int32_t arraySize);
   virtual void            dumpOptionHelp(TR::OptionTable *jitOptions, TR::OptionTable *feOptions, TR::SimpleRegex *nameFilter);

   static void dumpOptions(char *optionsType, char *options, char *envOptions, TR::Options *cmdLineOptions, TR::OptionTable *jitOptions, TR::OptionTable *feOptions, void *, TR_FrontEnd *);
   virtual char *          limitfileOption(char *, void *, TR::OptionTable *, TR::Options *, bool loadLimit, TR_PseudoRandomNumbersListElement **pseudoRandomListHeadPtr = 0);
   virtual char *          inlinefileOption(char *, void *, TR::OptionTable *, TR::Options *);
   virtual char *          limitOption(char *, void *, TR::OptionTable *, TR::Options *, bool loadLimit);
   char *                  limitOption(char *, void *, TR::OptionTable *, TR::Options *, TR::CompilationFilters * &);
   virtual int32_t *       loadCustomStrategy(char *optFileName);
   virtual bool            methodCanBeCompiled(TR_Memory *mem, TR_ResolvedMethod *, TR_FilterBST * &);
   virtual bool            methodCanBeRelocated(TR_Memory *mem, TR_ResolvedMethod *, TR_FilterBST * &);
   virtual bool            methodSigCanBeCompiled(const char *, TR_FilterBST * & , TR::Method::Type methodType);
   virtual bool            methodSigCanBeRelocated(const char *, TR_FilterBST * & );
   virtual bool            methodSigCanBeCompiledOrRelocated(const char *, TR_FilterBST * &, bool isRelocation, TR::Method::Type methodType);
   virtual bool            methodCanBeFound(TR_Memory *, TR_ResolvedMethod *, TR::CompilationFilters *, TR_FilterBST * &);
   virtual bool            methodSigCanBeFound(const char *, TR::CompilationFilters *, TR_FilterBST * &, TR::Method::Type methodType);
   virtual TR::CompilationFilters * getCompilationFilters() { return _compilationFilters; }
   virtual TR::CompilationFilters * getRelocationFilters() { return _relocationFilters; }
   virtual void            clearFilters(TR::CompilationFilters *);
   void                    clearFilters(bool loadLimit);
   virtual bool            scanInlineFilters(FILE *, int32_t &, TR::CompilationFilters *);
   virtual TR_FilterBST *  addFilter(char * &, int32_t, int32_t, int32_t, TR::CompilationFilters *);
   virtual TR_FilterBST *  addFilter(char * &, int32_t, int32_t, int32_t, bool loadLimit);
   virtual TR_FilterBST *  addExcludedMethodFilter(bool loadLimit);
   virtual int32_t         scanFilterName(char *, TR_FilterBST *);
   virtual void            printFilters(TR::CompilationFilters *);
   virtual void            printFilters();
   virtual void            print(TR_FilterBST * filter);
   virtual void         printHeader(TR::Logger *log);
   virtual void         printMethodHotness(TR::Logger *log);
   virtual void         printInstrDumpHeader(TR::Logger *log, const char * title);

   virtual void         printByteCodeAnnotations(TR::Logger *log);
   virtual void         printAnnotationInfoEntry(TR::Logger *log, J9AnnotationInfo *, J9AnnotationInfoEntry *, int32_t);

   virtual void         printOptimizationHeader(TR::Logger *log, const char *, const char *, int32_t, bool mustBeDone);

   virtual const char * getName(TR::ILOpCode);
   virtual const char * getName(TR::ILOpCodes);
   virtual const char * getName(TR::DataType);
   virtual const char * getName(TR_RawBCDSignCode);
   virtual const char * getName(TR::LabelSymbol *);
   virtual const char * getName(TR::SymbolReference *);
   virtual const char * getName(TR::Register *, TR_RegisterSizes = TR_WordReg);
   virtual const char * getRealRegisterName(uint32_t regNum);
   virtual const char * getGlobalRegisterName(TR_GlobalRegisterNumber regNum, TR_RegisterSizes size = TR_WordReg);
   virtual const char * getName(TR::Snippet *);
   virtual const char * getName(TR::Node *);
   virtual const char * getName(TR::Symbol *);
   virtual const char * getName(TR::Instruction *);
   virtual const char * getName(TR_Structure *);
   virtual const char * getName(TR::CFGNode *);
   virtual const char * getName(TR_ResolvedMethod *m) { return getName((void *) m, "(TR_ResolvedMethod*)", 0, false); }
   virtual const char * getName(TR_OpaqueClassBlock *c) { return getName((void *) c, "(TR_OpaqueClassBlock*)", 0, false); }
   virtual const char * getName(void *, const char *, uint32_t, bool);
   virtual const char * getName(const char *s) { return s; }
   virtual const char * getName(const char *s, int32_t len) { return s; }
   virtual const char * getVSSName(TR::AutomaticSymbol *sym);
   virtual const char * getWriteBarrierKindName(int32_t);
   virtual const char * getSpillKindName(uint8_t);
   virtual const char * getLinkageConventionName(uint8_t);
   virtual const char * getVirtualGuardKindName(TR_VirtualGuardKind kind);
   virtual const char * getVirtualGuardTestTypeName(TR_VirtualGuardTestType testType);
   virtual const char * getRuntimeHelperName(int32_t);

   virtual void         roundAddressEnumerationCounters(uint32_t boundary=16);

   virtual void         print(TR::Logger *log, uint8_t*, uint8_t*);
   virtual void         printIRTrees(TR::Logger *log, const char *, TR::ResolvedMethodSymbol *);
   virtual void         printBlockOrders(TR::Logger *log, const char *, TR::ResolvedMethodSymbol *);
   virtual void         print(TR::Logger *log, TR::CFG *);
   virtual void         print(TR::Logger *log, TR_Structure * structure, uint32_t indentation);
   virtual void         print(TR::Logger *log, TR_RegionAnalysis * structure, uint32_t indentation);
   virtual int32_t      print(TR::Logger *log, TR::TreeTop *);
   virtual int32_t      print(TR::Logger *log, TR::Node *, uint32_t indentation=0, bool printSubtree=true);
   virtual void         print(TR::Logger *log, TR::SymbolReference *);
   virtual void         print(TR::SymbolReference *, TR_PrettyPrinterString&, bool hideHelperMethodInfo=false, bool verbose=false);
   virtual void         print(TR::Logger *log, TR::LabelSymbol *);
   virtual void         print(TR::LabelSymbol *, TR_PrettyPrinterString&);
   virtual void         print(TR::Logger *log, TR_BitVector *);
   virtual void         print(TR::Logger *log, TR_SingleBitContainer *);
   virtual void         print(TR::Logger *log, TR::BitVector * bv);
   virtual void         print(TR::Logger *log, TR::SparseBitVector * sparse);
   virtual void         print(TR::Logger *log, TR::SymbolReferenceTable *);
   virtual void         printAliasInfo(TR::Logger *log, TR::SymbolReferenceTable *);
   virtual void         printAliasInfo(TR::Logger *log, TR::SymbolReference *);

   virtual int32_t      printWithFixedPrefix(TR::Logger *log, TR::Node *, uint32_t indentation, bool printChildren, bool printRefCounts, const char *prefix);
   virtual void         printVCG(TR::Logger *log, TR::CFG *, const char *);
   virtual void         printVCG(TR::Logger *log, TR::Node *, uint32_t indentation);

   virtual void         print(TR::Logger *log, J9JITExceptionTable *data, TR_ResolvedMethod *feMethod, bool fourByteOffsets);

   virtual void         clearNodeChecklist();
   virtual void         saveNodeChecklist(TR_BitVector &saveArea);
   virtual void         restoreNodeChecklist(TR_BitVector &saveArea);
   virtual void         dumpLiveRegisters(TR::Logger *log);
   virtual void         setupToDumpTreesAndInstructions(TR::Logger *log, const char *);
   virtual void         dumpSingleTreeWithInstrs(TR::Logger *log, TR::TreeTop *, TR::Instruction *, bool, bool, bool, bool);
   virtual void         dumpMethodInstrs(TR::Logger *log, const char *, bool, bool header = false);
   virtual void         dumpMixedModeDisassembly(TR::Logger *log);
   virtual void         dumpInstructionComments(TR::Logger *log, TR::Instruction *, bool needsStartComment = true );
   virtual void         print(TR::Logger *log, TR::Instruction *);
   virtual void         print(TR::Logger *log, TR::Instruction *, const char *);
   virtual void         print(TR::Logger *log, List<TR::Snippet> &);
   virtual void         print(TR::Logger *log, TR::list<TR::Snippet*> &);
   virtual void         print(TR::Logger *log, TR::Snippet *);

   virtual void         print(TR::Logger *log, TR::RegisterMappedSymbol *, bool);
   virtual void         print(TR::Logger *log, TR::GCStackAtlas *);
   virtual void         print(TR::Logger *log, TR_GCStackMap *, TR::GCStackAtlas *atlas = NULL);

   virtual void         printRegisterMask(TR::Logger *log, TR_RegisterMask mask, TR_RegisterKinds rk);
   virtual void         print(TR::Logger *log, TR::Register *, TR_RegisterSizes size = TR_WordReg);
   virtual void         printFullRegInfo(TR::Logger *log, TR::Register *);
   virtual const char * getRegisterKindName(TR_RegisterKinds);
   virtual const char * toString(TR_RematerializationInfo *);

   virtual void         print(TR::Logger *log, TR::VPConstraint *);

#ifdef J9_PROJECT_SPECIFIC
   virtual void         dump(TR::Logger *log, TR_CHTable *);
#endif

   virtual void         trace(const char *, ...);
   virtual void         traceLnFromLogTracer(const char *);
   virtual bool         performTransformationImpl(bool, const char *, ...);

   virtual void         printInstruction(TR::Instruction*);

   virtual const char * getDiagnosticFormat(const char *, char *, int32_t);

   virtual void         dumpGlobalRegisterTable(TR::Logger *log);
   virtual void         dumpSimulatedNode(TR::Logger *log, TR::Node *node, char tagChar);

   virtual uint32_t     getLabelNumber() { return _nextLabelNumber; }

   // Verification Passes
   //
   virtual void         verifyTrees (TR::ResolvedMethodSymbol *s);
   virtual void         verifyBlocks(TR::ResolvedMethodSymbol *s);
   virtual void         verifyCFG   (TR::ResolvedMethodSymbol *s);

   virtual TR::Node     *verifyFinalNodeReferenceCounts(TR::ResolvedMethodSymbol *s);

   virtual void startTracingRegisterAssignment(TR::Logger *log) { startTracingRegisterAssignment(log, "backward"); }
   virtual void startTracingRegisterAssignment(TR::Logger *log, const char *direction, TR_RegisterKinds kindsToAssign = TR_RegisterKinds(TR_GPR_Mask|TR_FPR_Mask));
   virtual void         stopTracingRegisterAssignment(TR::Logger *log);
   virtual void         pauseTracingRegisterAssignment();
   virtual void         resumeTracingRegisterAssignment();
   virtual void         traceRegisterAssignment(TR::Logger *log, const char *format, va_list args);
   virtual void         traceRegisterAssignment(TR::Logger *log, TR::Instruction *instr, bool insertedByRA = true, bool postRA = false);
   virtual void         traceRegisterAssigned(TR::Logger *log, TR_RegisterAssignmentFlags flags, TR::Register *virtReg, TR::Register *realReg);
   virtual void         traceRegisterFreed(TR::Logger *log, TR::Register *virtReg, TR::Register *realReg);
   virtual void         traceRegisterInterference(TR::Logger *log, TR::Register *virtReg, TR::Register *interferingVirtual, int32_t distance);
   virtual void         traceRegisterWeight(TR::Logger *log, TR::Register *realReg, uint32_t weight);

   virtual void         printGPRegisterStatus(TR::Logger *log, OMR::MachineConnector *machine);
   virtual void         printFPRegisterStatus(TR::Logger *log, OMR::MachineConnector *machine);

   virtual const char * getPerCodeCacheHelperName(TR_CCPreLoadedCode helper);

#if defined(TR_TARGET_X86)
   virtual const char * getOpCodeName(TR::InstOpCode *);
   virtual const char * getMnemonicName(TR::InstOpCode *);
   virtual void         printReferencedRegisterInfo(TR::Logger *log, TR::Instruction *);
   virtual void         dumpInstructionWithVFPState(TR::Logger *log, TR::Instruction *instr, const TR_VFPState *prevState);

   void print(TR::Logger *log, TR::RealRegister *, TR_RegisterSizes size = TR_WordReg);
   const char * getNamex(TR::Snippet *);
   void printRegMemInstruction(TR::Logger *log, const char *, TR::RealRegister *reg, TR::RealRegister *base = 0, int32_t = 0);
   void printRegRegInstruction(TR::Logger *log, const char *, TR::RealRegister *reg1, TR::RealRegister *reg2 = 0);
   void printRegImmInstruction(TR::Logger *log, const char *, TR::RealRegister *reg, int32_t imm);
   void printMemRegInstruction(TR::Logger *log, const char *, TR::RealRegister *base, int32_t offset, TR::RealRegister * = 0);
   void printMemImmInstruction(TR::Logger *log, const char *, TR::RealRegister *base, int32_t offset, int32_t imm);
#endif
#if defined(TR_TARGET_POWER)
   virtual const char * getOpCodeName(TR::InstOpCode *);
   const char * getName(TR::RealRegister *, TR_RegisterSizes size = TR_WordReg);
   void print(TR::Logger *log, TR::PPCHelperCallSnippet *);
#endif
#if defined(TR_TARGET_ARM)
   virtual void printARMDelayedOffsetInstructions(TR::Logger *log, TR::ARMMemInstruction *instr);
   virtual void printARMHelperBranch(TR::Logger *log, TR::SymbolReference *symRef, uint8_t *cursor, const char * opcodeName = "bl");
   virtual const char * getOpCodeName(TR::InstOpCode *);
   const char * getName(TR::RealRegister *, TR_RegisterSizes size = TR_WordReg);
   const char * getName(uint32_t realRegisterIndex, TR_RegisterSizes = (TR_RegisterSizes)-1);
   void print(TR::Logger *log, TR::ARMHelperCallSnippet *);
#endif
#if defined(TR_TARGET_S390)
   virtual void printRegisterDependencies(TR::Logger *log, TR::RegisterDependencyGroup *rgd, int numberOfRegisters);
   const char * getName(TR::RealRegister *, TR_RegisterSizes size = TR_WordReg);
#endif
#if defined(TR_TARGET_ARM64)
   virtual const char * getOpCodeName(TR::InstOpCode *);
   const char * getName(TR::RealRegister *, TR_RegisterSizes size = TR_WordReg);
#endif
#if defined(TR_TARGET_RISCV)
   virtual const char * getOpCodeName(TR::InstOpCode *);
   const char * getName(TR::RealRegister *, TR_RegisterSizes size = TR_WordReg);
#endif


#if defined(AIXPPC)
   virtual void setupDebugger(void *);
#elif defined(LINUX) || defined(J9ZOS390) || defined(OMR_OS_WINDOWS)
   virtual void setupDebugger(void *, void *, bool);
#endif /* defined(AIXPPC) */

   virtual void setSingleAllocMetaData(bool usesSingleAllocMetaData);

   const char* internalNamePrefix() { return "_"; }

   TR_OpaqueClassBlock * containingClass(TR::SymbolReference *);
   const char * signature(TR::ResolvedMethodSymbol *s);
   virtual void nodePrintAllFlags(TR::Node *, TR_PrettyPrinterString &);

   // used by DebugExt and may be overridden
   virtual void printDestination(TR::Logger *log, TR::TreeTop *);
   virtual void printDestination(TR::TreeTop *, TR_PrettyPrinterString&);
   virtual void printNodeInfo(TR::Logger *log, TR::Node *);
   virtual void printNodeInfo(TR::Node *, TR_PrettyPrinterString& output, bool);
   virtual void print(TR::Logger *log, TR::CFGNode *, uint32_t indentation);
   virtual void printNodesInEdgeListIterator(TR::Logger *log, TR::CFGEdgeList &li, bool fromNode);
   virtual void print(TR::Logger *log, TR::Block * block, uint32_t indentation);
   virtual void print(TR::Logger *log, TR_RegionStructure * regionStructure, uint32_t indentation);
   virtual void printSubGraph(TR::Logger *log, TR_RegionStructure * regionStructure, uint32_t indentation);
   virtual void print(TR::Logger *log, TR_InductionVariable * inductionVariable, uint32_t indentation);
   virtual void* dxMallocAndRead(uintptr_t size, void *remotePtr, bool dontAddToMap = false){return remotePtr;}
   virtual void* dxMallocAndReadString(void *remotePtr, bool dontAddToMap = false){return remotePtr;}
   virtual void  dxFree(void * localPtr){return;}
   void printTopLegend(TR::Logger *log);
   void printBottomLegend(TR::Logger *log);
   void printSymRefTable(TR::Logger *log, bool printFullTable = false);
   void printBasicPreNodeInfoAndIndent(TR::Logger *log, TR::Node *, uint32_t indentation);
   void printBasicPostNodeInfo(TR::Logger *log, TR::Node *, uint32_t indentation);

   bool isAlignStackMaps()
      {
#if defined(TR_HOST_ARM)
      return true;
#else
      return false;
#endif
      }

   void printNodeFlags(TR::Logger *log, TR::Node *);
#ifdef J9_PROJECT_SPECIFIC
   void printBCDNodeInfo(TR::Logger *log, TR::Node * node);
   void printBCDNodeInfo(TR::Node * node, TR_PrettyPrinterString& output);
#endif

   int32_t * printStackAtlas(TR::Logger *log, uintptr_t startPC, uint8_t * mapBits, int32_t numberOfSlotsMapped, bool fourByteOffsets, int32_t * sizeOfStackAtlas, int32_t frameSize);
   uint16_t printStackAtlasDetails(TR::Logger *log, uintptr_t startPC, uint8_t * mapBits, int numberOfSlotsMapped, bool fourByteOffsets, int32_t * sizeOfStackAtlas, int32_t frameSize, int32_t *offsetInfo);
   uint8_t *printMapInfo(TR::Logger *log, uintptr_t startPC, uint8_t * mapBits, int32_t numberOfSlotsMapped, bool fourByteOffsets, int32_t * sizeOfStackAtlas, TR_ByteCodeInfo *byteCodeInfo, uint16_t indexOfFirstInternalPtr, int32_t offsetInfo[], bool nummaps=false);
   void printStackMapInfo(TR::Logger *log, uint8_t * & mapBits, int32_t numberOfSlotsMapped, int32_t * sizeOfStackAtlas, int32_t * offsetInfo, bool nummaps=false);
   void printJ9JITExceptionTableDetails(TR::Logger *log, J9JITExceptionTable *data, J9JITExceptionTable *dbgextRemotePtr = NULL);
   void printSnippetLabel(TR::Logger *log, TR::LabelSymbol *label, uint8_t *cursor, const char *comment1, const char *comment2 = 0);
   uint8_t * printPrefix(TR::Logger *log, TR::Instruction *, uint8_t *cursor, uint8_t size);

   void printLabelInstruction(TR::Logger *log, const char *, TR::LabelSymbol *label);
   int32_t printRestartJump(TR::Logger *log, TR::X86RestartSnippet *, uint8_t *);
   int32_t printRestartJump(TR::Logger *log, TR::X86RestartSnippet *, uint8_t *, int32_t, const char *);

   char * printSymbolName(TR::Logger *log, TR::Symbol *, TR::SymbolReference *, TR::MemoryReference *mr=NULL)  ;
   bool isBranchToTrampoline(TR::SymbolReference *, uint8_t *, int32_t &);

   virtual void printDebugCounters(TR::DebugCounterGroup *counterGroup, const char *name);



// ------------------------------

   void printFirst(TR::Logger *log, int32_t i);
   void printCPIndex(TR::Logger *log, int32_t i);
   void printConstant(TR::Logger *log, int32_t i);
   void printConstant(TR::Logger *log, double d);
   void printFirstAndConstant(TR::Logger *log, int32_t i, int32_t j);

   void printLoadConst(TR::Logger *log, TR::Node *);
   void printLoadConst(TR::Node *, TR_PrettyPrinterString&);

   TR::CompilationFilters * findOrCreateFilters(TR::CompilationFilters *);
   TR::CompilationFilters * findOrCreateFilters(bool loadLimit);

   void printFilterTree(TR_FilterBST *root);

   TR::ResolvedMethodSymbol * getOwningMethodSymbol(TR::SymbolReference *);
   TR_ResolvedMethod     * getOwningMethod(TR::SymbolReference *);

   const char * getAutoName(TR::SymbolReference *);
   const char * getParmName(TR::SymbolReference *);
   const char * getStaticName(TR::SymbolReference *);
   const char * getStaticName_ForListing(TR::SymbolReference *);

   virtual const char * getMethodName(TR::SymbolReference *);

   const char * getShadowName(TR::SymbolReference *);
   const char * getMetaDataName(TR::SymbolReference *);

   TR::FILE *findLogFile(TR::Options *, TR::OptionSet *, char *, TR::Logger *&logger);
   void   findLogFile(const char *logFileName, TR::Options *cmdOptions, TR::Options **optionArray, int32_t arraySize, int32_t &index);

   void printPreds(TR::Logger *log, TR::CFGNode *);
   void printBaseInfo(TR::Logger *log, TR_Structure * structure, uint32_t indentation);
   void print(TR::Logger *log, TR_BlockStructure * blockStructure, uint32_t indentation);
   void print(TR::Logger *log, TR_StructureSubGraphNode * node, uint32_t indentation);

   void printBlockInfo(TR::Logger *log, TR::Node * node);

   void printVCG(TR::Logger *log, TR_Structure * structure);
   void printVCG(TR::Logger *log, TR_RegionStructure * regionStructure);
   void printVCG(TR::Logger *log, TR_StructureSubGraphNode * node, bool isEntry);
   void printVCGEdges(TR::Logger *log, TR_StructureSubGraphNode * node);
   void printVCG(TR::Logger *log, TR::Block * block, int32_t vorder = -1, int32_t horder = -1);

   void printByteCodeStack(TR::Logger *log, int32_t parentStackIndex, uint16_t byteCodeIndex, char * indent);
   void print(TR::Logger *log, TR::GCRegisterMap *);

   void verifyTreesPass1(TR::Node *node);
   void verifyTreesPass2(TR::Node *node, bool isTreeTop);
   void verifyBlocksPass1(TR::Node *node);
   void verifyBlocksPass2(TR::Node *node);
   void verifyGlobalIndices(TR::Node * node, TR::Node **nodesByGlobalIndex);

   TR::Node *verifyFinalNodeReferenceCounts(TR::Node *node);
   uint32_t getIntLength( uint32_t num ) const; // Number of digits in an integer
   // Number of spaces that must be inserted after index when index's length < maxIndexLength so the information following it will be aligned
   uint32_t getNumSpacesAfterIndex( uint32_t index, uint32_t maxIndexLength ) const;

#if defined(TR_TARGET_X86)
   void printPrefix(TR::Logger *log, TR::Instruction *instr);
   int32_t printPrefixAndMnemonicWithoutBarrier(TR::Logger *log, TR::Instruction *instr, int32_t barrier);
   void printPrefixAndMemoryBarrier(TR::Logger *log, TR::Instruction *instr, int32_t barrier, int32_t barrierOffset);
   void dumpDependencyGroup(TR::Logger *log, TR::RegisterDependencyGroup *group, int32_t numConditions, char *prefix, bool omitNullDependencies);
   void dumpDependencies(TR::Logger *log, TR::Instruction *);
   void printRegisterInfoHeader(TR::Logger *log, TR::Instruction *);
   void printBoundaryAvoidanceInfo(TR::Logger *log, TR::X86BoundaryAvoidanceInstruction *);

   void printX86OOLSequences(TR::Logger *log);

   void printx(TR::Logger *log, TR::Instruction *);
   void print(TR::Logger *log, TR::X86LabelInstruction *);
   void print(TR::Logger *log, TR::X86PaddingInstruction *);
   void print(TR::Logger *log, TR::X86AlignmentInstruction *);
   void print(TR::Logger *log, TR::X86BoundaryAvoidanceInstruction *);
   void print(TR::Logger *log, TR::X86PatchableCodeAlignmentInstruction *);
   void print(TR::Logger *log, TR::X86FenceInstruction *);
#ifdef J9_PROJECT_SPECIFIC
   void print(TR::Logger *log, TR::X86VirtualGuardNOPInstruction *);
#endif
   void print(TR::Logger *log, TR::X86ImmInstruction *);
   void print(TR::Logger *log, TR::X86ImmSnippetInstruction *);
   void print(TR::Logger *log, TR::X86ImmSymInstruction *);
   void print(TR::Logger *log, TR::X86RegInstruction *);
   void print(TR::Logger *log, TR::X86RegRegInstruction *);
   void print(TR::Logger *log, TR::X86RegImmInstruction *);
   void print(TR::Logger *log, TR::X86RegRegImmInstruction *);
   void print(TR::Logger *log, TR::X86RegRegRegInstruction *);
   void print(TR::Logger *log, TR::X86RegMaskRegInstruction *);
   void print(TR::Logger *log, TR::X86RegMaskRegRegInstruction *);
   void print(TR::Logger *log, TR::X86RegMaskRegRegImmInstruction *);
   void print(TR::Logger *log, TR::X86MemInstruction *);
   void print(TR::Logger *log, TR::X86MemImmInstruction *);
   void print(TR::Logger *log, TR::X86MemRegInstruction *);
   void print(TR::Logger *log, TR::X86MemMaskRegInstruction *);
   void print(TR::Logger *log, TR::X86MemRegImmInstruction *);
   void print(TR::Logger *log, TR::X86RegMemInstruction *);
   void print(TR::Logger *log, TR::X86RegMaskMemInstruction *);
   void print(TR::Logger *log, TR::X86RegMemImmInstruction *);
   void print(TR::Logger *log, TR::X86RegRegMemInstruction *);
   void print(TR::Logger *log, TR::X86FPRegInstruction *);
   void print(TR::Logger *log, TR::X86FPRegRegInstruction *);
   void print(TR::Logger *log, TR::X86FPMemRegInstruction *);
   void print(TR::Logger *log, TR::X86FPRegMemInstruction *);
   void print(TR::Logger *log, TR::AMD64Imm64Instruction *);
   void print(TR::Logger *log, TR::AMD64Imm64SymInstruction *);
   void print(TR::Logger *log, TR::AMD64RegImm64Instruction *);
   void print(TR::Logger *log, TR::X86VFPSaveInstruction *);
   void print(TR::Logger *log, TR::X86VFPRestoreInstruction *);
   void print(TR::Logger *log, TR::X86VFPDedicateInstruction *);
   void print(TR::Logger *log, TR::X86VFPReleaseInstruction *);
   void print(TR::Logger *log, TR::X86VFPCallCleanupInstruction *);

   void printReferencedRegisterInfo(TR::Logger *log, TR::X86RegRegRegInstruction *);
   void printReferencedRegisterInfo(TR::Logger *log, TR::X86RegInstruction *);
   void printReferencedRegisterInfo(TR::Logger *log, TR::X86RegRegInstruction *);
   void printReferencedRegisterInfo(TR::Logger *log, TR::X86MemInstruction *);
   void printReferencedRegisterInfo(TR::Logger *log, TR::X86MemRegInstruction *);
   void printReferencedRegisterInfo(TR::Logger *log, TR::X86RegMemInstruction *);
   void printReferencedRegisterInfo(TR::Logger *log, TR::X86RegRegMemInstruction *);

   void printFullRegisterDependencyInfo(TR::Logger *log, TR::RegisterDependencyConditions * conditions);
   void printDependencyConditions(TR::Logger *log, TR::RegisterDependencyGroup *, uint8_t, char *);

   void print(TR::Logger *log, TR::MemoryReference *, TR_RegisterSizes);
   void printReferencedRegisterInfo(TR::Logger *log, TR::MemoryReference *);

   int32_t printIntConstant(TR::Logger *log, int64_t value, int8_t radix, TR_RegisterSizes size = TR_WordReg, bool padWithZeros = false);
   int32_t printDecimalConstant(TR::Logger *log, int64_t value, int8_t width, bool padWithZeros);
   int32_t printHexConstant(TR::Logger *log, int64_t value, int8_t width, bool padWithZeros);
   void printInstructionComment(TR::Logger *log, int32_t tabStops, TR::Instruction *instr);
   void printFPRegisterComment(TR::Logger *log, TR::Register *target, TR::Register *source);
   void printMemoryReferenceComment(TR::Logger *log, TR::MemoryReference *mr);
   TR_RegisterSizes getTargetSizeFromInstruction(TR::Instruction *instr);
   TR_RegisterSizes getSourceSizeFromInstruction(TR::Instruction *instr);
   TR_RegisterSizes getImmediateSizeFromInstruction(TR::Instruction *instr);

   void printFullRegInfo(TR::Logger *log, TR::RealRegister *);
   void printX86GCRegisterMap(TR::Logger *log, TR::GCRegisterMap *);

   const char * getName(TR::RealRegister *, TR_RegisterSizes = TR_WordReg);
   const char * getName(uint32_t realRegisterIndex, TR_RegisterSizes = (TR_RegisterSizes)-1);

   void printx(TR::Logger *log, TR::Snippet *);

#ifdef J9_PROJECT_SPECIFIC
   void print(TR::Logger *log, TR::X86CallSnippet *);
   void print(TR::Logger *log, TR::X86PicDataSnippet *);
   void print(TR::Logger *log, TR::X86CheckFailureSnippet *);
   void print(TR::Logger *log, TR::X86CheckFailureSnippetWithResolve *);
   void print(TR::Logger *log, TR::X86BoundCheckWithSpineCheckSnippet *);
   void print(TR::Logger *log, TR::X86SpineCheckSnippet *);
   void print(TR::Logger *log, TR::X86ForceRecompilationSnippet *);
   void print(TR::Logger *log, TR::X86RecompilationSnippet *);
#endif

   void print(TR::Logger *log, TR::X86DivideCheckSnippet *);
   void print(TR::Logger *log, TR::X86FPConvertToIntSnippet *);
   void print(TR::Logger *log, TR::X86FPConvertToLongSnippet *);
   void print(TR::Logger *log, TR::X86GuardedDevirtualSnippet *);
   void print(TR::Logger *log, TR::X86HelperCallSnippet *);
   void printBody(TR::Logger *log, TR::X86HelperCallSnippet *, uint8_t *bufferPos);

   void print(TR::Logger *log, TR::UnresolvedDataSnippet *);

#ifdef TR_TARGET_64BIT
   uint8_t *printArgumentFlush(TR::Logger *log, TR::Node *, bool, uint8_t *);
#endif
#endif
#ifdef TR_TARGET_POWER
   void printPrefix(TR::Logger *log, TR::Instruction *);

   void print(TR::Logger *log, TR::PPCAlignmentNopInstruction *);
   void print(TR::Logger *log, TR::PPCDepInstruction *);
   void print(TR::Logger *log, TR::PPCLabelInstruction *);
   void print(TR::Logger *log, TR::PPCDepLabelInstruction *);
   void print(TR::Logger *log, TR::PPCConditionalBranchInstruction *);
   void print(TR::Logger *log, TR::PPCDepConditionalBranchInstruction *);
   void print(TR::Logger *log, TR::PPCAdminInstruction *);
   void print(TR::Logger *log, TR::PPCImmInstruction *);
   void print(TR::Logger *log, TR::PPCSrc1Instruction *);
   void print(TR::Logger *log, TR::PPCDepImmSymInstruction *);
   void print(TR::Logger *log, TR::PPCTrg1Instruction *);
   void print(TR::Logger *log, TR::PPCTrg1Src1Instruction *);
   void print(TR::Logger *log, TR::PPCTrg1ImmInstruction *);
   void print(TR::Logger *log, TR::PPCTrg1Src1ImmInstruction *);
   void print(TR::Logger *log, TR::PPCTrg1Src1Imm2Instruction *);
   void print(TR::Logger *log, TR::PPCSrc2Instruction *);
   void print(TR::Logger *log, TR::PPCSrc3Instruction *);
   void print(TR::Logger *log, TR::PPCTrg1Src2Instruction *);
   void print(TR::Logger *log, TR::PPCTrg1Src2ImmInstruction *);
   void print(TR::Logger *log, TR::PPCTrg1Src3Instruction *);
   void print(TR::Logger *log, TR::PPCMemSrc1Instruction *);
   void print(TR::Logger *log, TR::PPCMemInstruction *);
   void print(TR::Logger *log, TR::PPCTrg1MemInstruction *);
   void print(TR::Logger *log, TR::PPCControlFlowInstruction *);
#ifdef J9_PROJECT_SPECIFIC
   void print(TR::Logger *log, TR::PPCVirtualGuardNOPInstruction *);
#endif

   TR::Instruction* getOutlinedTargetIfAny(TR::Instruction *instr);
   void printPPCOOLSequences(TR::Logger *log);

   const char * getPPCRegisterName(uint32_t regNum, bool useVSR = false);

   void print(TR::Logger *log, TR::RealRegister *, TR_RegisterSizes size = TR_WordReg);
   void print(TR::Logger *log, TR::RegisterDependency *);
   void print(TR::Logger *log, TR::RegisterDependencyConditions *);
   void printPPCGCRegisterMap(TR::Logger *log, TR::GCRegisterMap *);

   void print(TR::Logger *log, TR::MemoryReference *, bool d_form = true);

   uint8_t * printEmitLoadPPCHelperAddrToCtr(TR::Logger *log, uint8_t*, int32_t, TR::RealRegister *);
   uint8_t * printEmitLoadIndirectPPCHelperAddrToCtr(TR::Logger *log, uint8_t*, TR::RealRegister *, TR::RealRegister *, int32_t);
   uint8_t * printPPCArgumentsFlush(TR::Logger *log, TR::Node *, uint8_t *, int32_t);
   void printInstructionComment(TR::Logger *log, int32_t tabStops, TR::Instruction *instr);

   void printp(TR::Logger *log, TR::Snippet *);
   void print(TR::Logger *log, TR::PPCMonitorEnterSnippet *);
   void print(TR::Logger *log, TR::PPCMonitorExitSnippet *);
   void print(TR::Logger *log, TR::PPCReadMonitorSnippet *);
   void print(TR::Logger *log, TR::PPCAllocPrefetchSnippet *);


   void print(TR::Logger *log, TR::UnresolvedDataSnippet *);

#ifdef J9_PROJECT_SPECIFIC
   void print(TR::Logger *log, TR::PPCStackCheckFailureSnippet *);
   void print(TR::Logger *log, TR::PPCInterfaceCastSnippet *);
   void print(TR::Logger *log, TR::PPCUnresolvedCallSnippet *);
   void print(TR::Logger *log, TR::PPCVirtualSnippet *);
   void print(TR::Logger *log, TR::PPCVirtualUnresolvedSnippet *);
   void print(TR::Logger *log, TR::PPCInterfaceCallSnippet *);
   void print(TR::Logger *log, TR::PPCForceRecompilationSnippet *);
   void print(TR::Logger *log, TR::PPCRecompilationSnippet *);
   void print(TR::Logger *log, TR::PPCCallSnippet *);
#endif



   void printMemoryReferenceComment(TR::Logger *log, TR::MemoryReference *mr);
   void print(TR::Logger *log, TR::PPCLockReservationEnterSnippet *);
   void print(TR::Logger *log, TR::PPCLockReservationExitSnippet *);
   uint8_t* print(TR::Logger *log, TR::PPCArrayCopyCallSnippet *snippet, uint8_t *cursor);

#endif
#ifdef TR_TARGET_ARM
   char * fullOpCodeName(TR::Instruction *instr);
   void printPrefix(TR::Logger *log, TR::Instruction *);
   void printBinaryPrefix(char *prefixBuffer, TR::Instruction *);
   void dumpDependencyGroup(TR::Logger *log, TR::RegisterDependencyGroup *group, int32_t numConditions, char *prefix, bool omitNullDependencies);
   void dumpDependencies(TR::Logger *log, TR::Instruction *);
   void print(TR::Logger *log, TR::ARMLabelInstruction *);
#ifdef J9_PROJECT_SPECIFIC
   void print(TR::Logger *log, TR::ARMVirtualGuardNOPInstruction *);
#endif
   void print(TR::Logger *log, TR::ARMAdminInstruction *);
   void print(TR::Logger *log, TR::ARMImmInstruction *);
   void print(TR::Logger *log, TR::ARMImmSymInstruction *);
   void print(TR::Logger *log, TR::ARMTrg1Src2Instruction *);
   void print(TR::Logger *log, TR::ARMTrg2Src1Instruction *);
   void print(TR::Logger *log, TR::ARMMulInstruction *);
   void print(TR::Logger *log, TR::ARMMemSrc1Instruction *);
   void print(TR::Logger *log, TR::ARMTrg1Instruction *);
   void print(TR::Logger *log, TR::ARMMemInstruction *);
   void print(TR::Logger *log, TR::ARMTrg1MemInstruction *);
   void print(TR::Logger *log, TR::ARMTrg1MemSrc1Instruction *);
   void print(TR::Logger *log, TR::ARMControlFlowInstruction *);
   void print(TR::Logger *log, TR::ARMMultipleMoveInstruction *);

   void print(TR::Logger *log, TR::MemoryReference *);

   void print(TR::Logger *log, TR_ARMOperand2 * op, TR_RegisterSizes size = TR_WordReg);

   const char * getNamea(TR::Snippet *);

   void print(TR::Logger *log, TR::RealRegister *, TR_RegisterSizes size = TR_WordReg);
   void printARMGCRegisterMap(TR::Logger *log, TR::GCRegisterMap *);
   void printInstructionComment(TR::Logger *log, int32_t tabStops, TR::Instruction *instr);

   void printa(TR::Logger *log, TR::Snippet *);
   void print(TR::Logger *log, TR::ARMCallSnippet *);
   void print(TR::Logger *log, TR::ARMUnresolvedCallSnippet *);
   void print(TR::Logger *log, TR::ARMVirtualSnippet *);
   void print(TR::Logger *log, TR::ARMVirtualUnresolvedSnippet *);
   void print(TR::Logger *log, TR::ARMInterfaceCallSnippet *);
   void print(TR::Logger *log, TR::ARMMonitorEnterSnippet *);
   void print(TR::Logger *log, TR::ARMMonitorExitSnippet *);
   void print(TR::Logger *log, TR::ARMStackCheckFailureSnippet *);
   void print(TR::Logger *log, TR::UnresolvedDataSnippet *);
   void print(TR::Logger *log, TR::ARMRecompilationSnippet *);
#endif
#ifdef TR_TARGET_S390
   void printPrefix(TR::Logger *log, TR::Instruction *);

   void printS390RegisterDependency(TR::Logger *log, TR::Register * virtReg, int realReg, bool uses, bool defs);

   TR::Instruction* getOutlinedTargetIfAny(TR::Instruction *instr);
   void printS390OOLSequences(TR::Logger *log);

   void dumpDependencies(TR::Logger *log, TR::Instruction *, bool, bool);
   void printAssocRegDirective(TR::Logger *log, TR::Instruction * instr);
   void printz(TR::Logger *log, TR::Instruction *);
   void printz(TR::Logger *log, TR::Instruction *, const char *);
   void print(TR::Logger *log, TR::S390LabelInstruction *);
   void print(TR::Logger *log, TR::S390BranchInstruction *);
   void print(TR::Logger *log, TR::S390BranchOnCountInstruction *);
#ifdef J9_PROJECT_SPECIFIC
   void print(TR::Logger *log, TR::S390VirtualGuardNOPInstruction *);
#endif
   void print(TR::Logger *log, TR::S390BranchOnIndexInstruction *);
   void print(TR::Logger *log, TR::S390ImmInstruction *);
   void print(TR::Logger *log, TR::S390ImmSnippetInstruction *);
   void print(TR::Logger *log, TR::S390ImmSymInstruction *);
   void print(TR::Logger *log, TR::S390Imm2Instruction *);
   void print(TR::Logger *log, TR::S390RegInstruction *);
   void print(TR::Logger *log, TR::S390TranslateInstruction *);
   void print(TR::Logger *log, TR::S390RRInstruction *);
   void print(TR::Logger *log, TR::S390RRFInstruction *);
   void print(TR::Logger *log, TR::S390RRRInstruction *);
   void print(TR::Logger *log, TR::S390RIInstruction *);
   void print(TR::Logger *log, TR::S390RILInstruction *);
   void print(TR::Logger *log, TR::S390RSInstruction *);
   void print(TR::Logger *log, TR::S390RSLInstruction *);
   void print(TR::Logger *log, TR::S390RSLbInstruction *);
   void print(TR::Logger *log, TR::S390MemInstruction *);
   void print(TR::Logger *log, TR::S390SS1Instruction *);
   void print(TR::Logger *log, TR::S390SS2Instruction *);
   void print(TR::Logger *log, TR::S390SMIInstruction *);
   void print(TR::Logger *log, TR::S390MIIInstruction *);
   void print(TR::Logger *log, TR::S390SS4Instruction *);
   void print(TR::Logger *log, TR::S390SSFInstruction *);
   void print(TR::Logger *log, TR::S390SSEInstruction *);
   void print(TR::Logger *log, TR::S390SIInstruction *);
   void print(TR::Logger *log, TR::S390SILInstruction *);
   void print(TR::Logger *log, TR::S390SInstruction *);
   void print(TR::Logger *log, TR::S390RXInstruction *);
   void print(TR::Logger *log, TR::S390RXEInstruction *);
   void print(TR::Logger *log, TR::S390RXFInstruction *);
   void print(TR::Logger *log, TR::S390AnnotationInstruction *);
   void print(TR::Logger *log, TR::S390PseudoInstruction *);
   void print(TR::Logger *log, TR::S390NOPInstruction *);
   void print(TR::Logger *log, TR::S390AlignmentNopInstruction *);
   void print(TR::Logger *log, TR::MemoryReference *, TR::Instruction *);
   void print(TR::Logger *log, TR::S390RRSInstruction *);
   void print(TR::Logger *log, TR::S390RIEInstruction *);
   void print(TR::Logger *log, TR::S390RISInstruction *);
   void print(TR::Logger *log, TR::S390OpCodeOnlyInstruction *);
   void print(TR::Logger *log, TR::S390IInstruction *);
   void print(TR::Logger *log, TR::S390IEInstruction *);
   void print(TR::Logger *log, TR::S390VRIInstruction *);
   void print(TR::Logger *log, TR::S390VRRInstruction *);
   void print(TR::Logger *log, TR::S390VStorageInstruction *);


   const char * getS390RegisterName(uint32_t regNum, bool isVRF = false);
   void print(TR::Logger *log, TR::RealRegister *, TR_RegisterSizes size = TR_WordReg);
   void printFullRegInfo(TR::Logger *log, TR::RealRegister *);
   void printS390GCRegisterMap(TR::Logger *log, TR::GCRegisterMap *);

   uint8_t * printS390ArgumentsFlush(TR::Logger *log, TR::Node *, uint8_t *, int32_t);

   void printz(TR::Logger *log, TR::Snippet *);

   void print(TR::Logger *log, TR::S390ConstantDataSnippet *);

   void print(TR::Logger *log, TR::S390HelperCallSnippet *);
#ifdef J9_PROJECT_SPECIFIC
   void print(TR::Logger *log, TR::S390ForceRecompilationSnippet *);
   void print(TR::Logger *log, TR::S390ForceRecompilationDataSnippet *);
   void print(TR::Logger *log, TR::S390UnresolvedCallSnippet *);
   void print(TR::Logger *log, TR::S390VirtualSnippet *);
   void print(TR::Logger *log, TR::S390VirtualUnresolvedSnippet *);
   void print(TR::Logger *log, TR::S390InterfaceCallSnippet *);
   void print(TR::Logger *log, TR::J9S390InterfaceCallDataSnippet *);
#endif
   void print(TR::Logger *log, TR::S390StackCheckFailureSnippet *);
   void print(TR::Logger *log, TR::UnresolvedDataSnippet *);
   void print(TR::Logger *log, TR::S390HeapAllocSnippet *);
   void print(TR::Logger *log, TR::S390JNICallDataSnippet *);
   void print(TR::Logger *log, TR::S390RestoreGPR7Snippet *);

   // Assembly related snippet display
   void printGPRegisterStatus(TR::Logger *log, TR::Machine *machine);
   void printFPRegisterStatus(TR::Logger *log, TR::Machine *machine);
   uint32_t getBitRegNum (TR::RealRegister *reg);

   void printInstructionComment(TR::Logger *log, int32_t tabStops, TR::Instruction *instr, bool needsStartComment = false);
   uint8_t *printLoadVMThreadInstruction(TR::Logger *log, uint8_t* cursor);
   uint8_t *printRuntimeInstrumentationOnOffInstruction(TR::Logger *log, uint8_t* cursor, bool isRION, bool isPrivateLinkage = false);
   const char *updateBranchName(const char * opCodeName, const char * brCondName);
#endif
#ifdef TR_TARGET_ARM64
   void printPrefix(TR::Logger *log, TR::Instruction *);

   void print(TR::Logger *log, TR::ARM64ImmInstruction *);
   void print(TR::Logger *log, TR::ARM64RelocatableImmInstruction *);
   void print(TR::Logger *log, TR::ARM64ImmSymInstruction *);
   void print(TR::Logger *log, TR::ARM64LabelInstruction *);
   void print(TR::Logger *log, TR::ARM64ConditionalBranchInstruction *);
   void print(TR::Logger *log, TR::ARM64CompareBranchInstruction *);
   void print(TR::Logger *log, TR::ARM64TestBitBranchInstruction *);
   void print(TR::Logger *log, TR::ARM64RegBranchInstruction *);
   void print(TR::Logger *log, TR::ARM64AdminInstruction *);
   void print(TR::Logger *log, TR::ARM64Trg1Instruction *);
   void print(TR::Logger *log, TR::ARM64Trg1CondInstruction *);
   void print(TR::Logger *log, TR::ARM64Trg1ImmInstruction *);
   void print(TR::Logger *log, TR::ARM64Trg1ImmShiftedInstruction *);
   void print(TR::Logger *log, TR::ARM64Trg1ImmSymInstruction *);
   void print(TR::Logger *log, TR::ARM64Trg1Src1Instruction *);
   void print(TR::Logger *log, TR::ARM64Trg1ZeroSrc1Instruction *);
   void print(TR::Logger *log, TR::ARM64Trg1ZeroImmInstruction *);
   void print(TR::Logger *log, TR::ARM64Trg1Src1ImmInstruction *);
   void print(TR::Logger *log, TR::ARM64Trg1Src2Instruction *);
   void print(TR::Logger *log, TR::ARM64CondTrg1Src2Instruction *);
   void print(TR::Logger *log, TR::ARM64Trg1Src2ImmInstruction *);
   void print(TR::Logger *log, TR::ARM64Trg1Src2ShiftedInstruction *);
   void print(TR::Logger *log, TR::ARM64Trg1Src2ExtendedInstruction *);
   void print(TR::Logger *log, TR::ARM64Trg1Src2IndexedElementInstruction *);
   void print(TR::Logger *log, TR::ARM64Trg1Src2ZeroInstruction *);
   void print(TR::Logger *log, TR::ARM64Trg1Src3Instruction *);
   void print(TR::Logger *log, TR::ARM64Trg1MemInstruction *);
   void print(TR::Logger *log, TR::ARM64Trg2MemInstruction *);
   void print(TR::Logger *log, TR::ARM64MemInstruction *);
   void print(TR::Logger *log, TR::ARM64MemImmInstruction *);
   void print(TR::Logger *log, TR::ARM64MemSrc1Instruction *);
   void print(TR::Logger *log, TR::ARM64MemSrc2Instruction *);
   void print(TR::Logger *log, TR::ARM64Trg1MemSrc1Instruction *);
   void print(TR::Logger *log, TR::ARM64Src1Instruction *);
   void print(TR::Logger *log, TR::ARM64ZeroSrc1ImmInstruction *);
   void print(TR::Logger *log, TR::ARM64Src2Instruction *);
   void print(TR::Logger *log, TR::ARM64ZeroSrc2Instruction *);
   void print(TR::Logger *log, TR::ARM64Src1ImmCondInstruction *);
   void print(TR::Logger *log, TR::ARM64Src2CondInstruction *);
#ifdef J9_PROJECT_SPECIFIC
   void print(TR::Logger *log, TR::ARM64VirtualGuardNOPInstruction *);
#endif
   void print(TR::Logger *log, TR::RealRegister *, TR_RegisterSizes size = TR_WordReg);
   void print(TR::Logger *log, TR::RegisterDependency *);
   void print(TR::Logger *log, TR::RegisterDependencyConditions *);
   void print(TR::Logger *log, TR::MemoryReference *);
   void print(TR::Logger *log, TR::UnresolvedDataSnippet *);

   void printARM64OOLSequences(TR::Logger *log);
   void printARM64GCRegisterMap(TR::Logger *log, TR::GCRegisterMap *);
   void printInstructionComment(TR::Logger *log, int32_t, TR::Instruction *);
   void printMemoryReferenceComment(TR::Logger *log, TR::MemoryReference *);
   void printAssocRegDirective(TR::Logger *log, TR::Instruction *);

   const char *getARM64RegisterName(uint32_t, bool = true);

   void printa64(TR::Logger *log, TR::Snippet *);
   const char * getNamea64(TR::Snippet *);

#ifdef J9_PROJECT_SPECIFIC
   void print(TR::Logger *log, TR::ARM64CallSnippet *);
   void print(TR::Logger *log, TR::ARM64UnresolvedCallSnippet *);
   void print(TR::Logger *log, TR::ARM64VirtualUnresolvedSnippet *);
   void print(TR::Logger *log, TR::ARM64InterfaceCallSnippet *);
   void print(TR::Logger *log, TR::ARM64StackCheckFailureSnippet *);
   void print(TR::Logger *log, TR::ARM64ForceRecompilationSnippet *);
   void print(TR::Logger *log, TR::ARM64RecompilationSnippet *);
   uint8_t *printARM64ArgumentsFlush(TR::Logger *log, TR::Node *, uint8_t *, int32_t);
#endif
   void print(TR::Logger *log, TR::ARM64HelperCallSnippet *);

#endif
#ifdef TR_TARGET_RISCV
   void printPrefix(TR::Logger *log, TR::Instruction *);

   void print(TR::Logger *log, TR::LabelInstruction *);
   void print(TR::Logger *log, TR::AdminInstruction *);
   void print(TR::Logger *log, TR::DataInstruction *);

   void print(TR::Logger *log, TR::RtypeInstruction *);
   void print(TR::Logger *log, TR::ItypeInstruction *);
   void print(TR::Logger *log, TR::StypeInstruction *);
   void print(TR::Logger *log, TR::BtypeInstruction *);
   void print(TR::Logger *log, TR::UtypeInstruction *);
   void print(TR::Logger *log, TR::JtypeInstruction *);
   void print(TR::Logger *log, TR::LoadInstruction * );
   void print(TR::Logger *log, TR::StoreInstruction *);
   void print(TR::Logger *log, TR::RVHelperCallSnippet *);

   void print(TR::Logger *log, TR::RealRegister *, TR_RegisterSizes size = TR_WordReg);
   void print(TR::Logger *log, TR::RegisterDependency *);
   void print(TR::Logger *log, TR::RegisterDependencyConditions *);
   void print(TR::Logger *log, TR::MemoryReference *);
   void print(TR::Logger *log, TR::UnresolvedDataSnippet *);

   void printRVOOLSequences(TR::Logger *log);
   void printRVGCRegisterMap(TR::Logger *log, TR::GCRegisterMap *);
   void printInstructionComment(TR::Logger *log, int32_t, TR::Instruction *);
   void printMemoryReferenceComment(TR::Logger *log, TR::MemoryReference *);

   const char *getRVRegisterName(uint32_t, bool = true);
#endif

   friend class TR_CFGChecker;

protected:
   virtual void vtrace(const char *, va_list args);

   TR::FILE                  * _file;
   TR::Compilation           * _comp;
   TR_FrontEnd               * _fe;

   TR::Logger                *_logger;

   uint32_t                   _nextLabelNumber;
   uint32_t                   _nextRegisterNumber;
   uint32_t                   _nextNodeNumber;
   uint32_t                   _nextSymbolNumber;
   uint32_t                   _nextInstructionNumber;
   uint32_t                   _nextStructureNumber;
   uint32_t                   _nextVariableSizeSymbolNumber;
   TR::CompilationFilters    * _compilationFilters;
   TR::CompilationFilters    * _relocationFilters;
   TR::CompilationFilters    * _inlineFilters;
   bool                       _usesSingleAllocMetaData;
   TR_BitVector               _nodeChecklist;
   TR_BitVector               _structureChecklist;

   TR::CodeGenerator *_cg;

   int32_t                    _lastFrequency;
   bool                       _isCold;
   bool                       _mainEntrySeen;
   TR::Node                  * _currentParent;
   int32_t                    _currentChildIndex;

#define TRACERA_IN_PROGRESS             (0x0001)
#define TRACERA_INSTRUCTION_INSERTED    (0x0002)
   uint16_t                   _registerAssignmentTraceFlags;
   uint16_t                   _pausedRegisterAssignmentTraceFlags;
   int16_t                    _registerAssignmentTraceCursor;
   TR_RegisterKinds           _registerKindsToAssign;

   const static uint32_t      DEFAULT_NODE_LENGTH = 82; //Tree information printed to the right of node OPCode will be aligned if node OPCode's length <= DEFAULT_NODE_LENGTH, or will follow the node OPCode immediately (non-aligned) otherwise.
   const static uint32_t      MAX_GLOBAL_INDEX_LENGTH = 5;
   };


class TR_PrettyPrinterString
   {
   public:
      TR_PrettyPrinterString(TR_Debug* debug);

      /**
       * @brief Append a null-terminated string with format specifiers to the buffer.
       *        The buffer is guaranteed not to overflow and will be null-terminated.
       *
       * @param[in] format : null-terminated string to append with optional format specifiers
       * @param[in] ... : optional arguments for format specifiers
       */
      void appendf(char const *format, ...);

      /**
       * @brief Append a null-terminated string to the buffer.  No format specifiers
       *        are permitted.  The buffer is guaranteed not to overflow and will be
       *        null-terminated.
       *
       * @param[in] str : null-terminated string to append
       */
      void appends(char const *str);

      const char* getStr() {return buffer;}
      int32_t getLength() {return len;}
      void reset() {buffer[0] = '\0'; len = 0;}
      bool isEmpty() { return len <= 0; }

      static const int32_t maxBufferLength = 2000;

   private:
      char buffer[maxBufferLength];
      int32_t len;
      TR::Compilation *_comp;
      TR_Debug *_debug;
   };

typedef TR_Debug * (* TR_CreateDebug_t)(TR::Compilation *);
#endif
