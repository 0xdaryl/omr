/*******************************************************************************
 * Copyright (c) 2000, 2021 IBM Corp. and others
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

#ifndef OMR_INLINER_INCL
#define OMR_INLINER_INCL

/*
 * The following #define and typedef must appear before any #includes in this file
 */
#ifndef OMR_INLINER_CONNECTOR
#define OMR_INLINER_CONNECTOR
namespace OMR { class Inliner; }
namespace OMR { typedef OMR::Inliner InlinerConnector; }
#endif

#include <stddef.h>
#include <stdint.h>
#include "codegen/RecognizedMethods.hpp"
#include "env/TRMemory.hpp"
#include "env/jittypes.h"
#include "il/ILOpCodes.hpp"
#include "infra/Assert.hpp"
#include "infra/Flags.hpp"
#include "infra/Link.hpp"
#include "infra/List.hpp"
#include "infra/Random.hpp"

namespace TR { class Block; }
namespace TR { class CFG; }
namespace TR { class Compilation; }
namespace TR { class Node; }
namespace TR { class Optimizer; }
namespace TR { class Optimization; }
namespace TR { class ResolvedMethodSymbol; }
namespace TR { class Symbol; }
namespace TR { class SymbolReference; }
namespace TR { class TreeTop; }
class TR_CallSite;
class TR_CallStack;
struct TR_CallTarget;
class TR_FrontEnd;
class OMR_InlinerPolicy;
class OMR_InlinerUtil;
class TR_InlinerTracer;
struct TR_ParameterMapping;
class TR_PrexArgInfo;
class TR_ResolvedMethod;
class TR_TransformInlinedFunction;
struct TR_VirtualGuardSelection;


namespace OMR
{

class Inliner : public TR_HasRandomGenerator
   {
   protected:
      virtual bool supportsMultipleTargetInlining () { return false ; }

   public:
      vcount_t getVisitCount() { return _visitCount;}

      void setPolicy(OMR_InlinerPolicy *policy){ _policy = policy; }
      void setUtil(OMR_InlinerUtil *util){ _util = util; }
      OMR_InlinerPolicy *getPolicy(){ TR_ASSERT(_policy != NULL, "inliner policy must be set"); return _policy; }
      OMR_InlinerUtil *getUtil(){ TR_ASSERT(_util != NULL, "inliner util must be set"); return _util; }

      bool allowBiggerMethods() { return comp()->isServerInlining(); }


      bool isEDODisableInlinedProfilingInfo () {return _EDODisableInlinedProfilingInfo; }

      void setInlineThresholds(TR::ResolvedMethodSymbol *callerSymbol);

      void performInlining(TR::ResolvedMethodSymbol *);

      int32_t getCurrentNumberOfNodes() { return _currentNumberOfNodes; }
      int32_t getMaxInliningCallSites() { return _maxInliningCallSites; }
      int32_t getNumAsyncChecks() { return _numAsyncChecks; }
      int32_t getNodeCountThreshold() { return _nodeCountThreshold;}
      int32_t getMaxRecursiveCallByteCodeSizeEstimate() { return _maxRecursiveCallByteCodeSizeEstimate;}
      void incCurrentNumberOfNodes(int32_t i) { _currentNumberOfNodes+=i; }

      bool inlineVirtuals()                     { return _flags.testAny(InlineVirtuals); }
      void setInlineVirtuals(bool b)            { _flags.set(InlineVirtuals, b); }
      bool inlineSynchronized()                 { return _flags.testAny(InlineSynchronized); }
      void setInlineSynchronized(bool b)        { _flags.set(InlineSynchronized, b); }
      bool firstPass()                          { return _flags.testAny(FirstPass); }
      void setFirstPass(bool b=true)            { _flags.set(FirstPass, b); }
      uint32_t getSizeThreshold()               { return _methodByteCodeSizeThreshold; }
      void     setSizeThreshold(uint32_t size);
      virtual bool inlineRecognizedMethod(TR::RecognizedMethod method);


      void createParmMap(TR::ResolvedMethodSymbol *calleeSymbol, TR_LinkHead<TR_ParameterMapping> &map);

      TR::Compilation * comp()                   { return _optimizer->comp();}
      TR_Memory * trMemory()                    { return _trMemory; }
      TR_StackMemory trStackMemory()            { return _trMemory; }
      TR_HeapMemory  trHeapMemory()             { return _trMemory; }
      TR_PersistentMemory * trPersistentMemory(){ return _trMemory->trPersistentMemory(); }
      TR_FrontEnd    * fe()                     { return _optimizer->comp()->fe(); }


      TR::Optimizer * getOptimizer()         { return _optimizer; }


      TR_InlinerTracer* tracer()                 { return _tracer; }

      void setStoreToCachedPrivateStatic(TR::Node *node) { _storeToCachedPrivateStatic = node; }
      TR::Node *getStoreToCachedPrivateStatic() { return _storeToCachedPrivateStatic; }
      bool alwaysWorthInlining(TR_ResolvedMethod * calleeMethod, TR::Node *callNode);

      void getSymbolAndFindInlineTargets(TR_CallStack *, TR_CallSite *, bool findNewTargets=true);

      void applyPolicyToTargets(TR_CallStack *, TR_CallSite *);
      bool callMustBeInlinedRegardlessOfSize(TR_CallSite *callsite);

      bool forceInline(TR_CallTarget *calltarget);
      bool forceVarInitInlining(TR_CallTarget *calltarget);
      bool forceCalcInlining(TR_CallTarget *calltarget);

   protected:

      Inliner(TR::Optimizer *, TR::Optimization *);

      TR_PrexArgInfo* buildPrexArgInfoForOutermostMethod(TR::ResolvedMethodSymbol* methodSymbol);
      bool inlineCallTarget(TR_CallStack *,TR_CallTarget *calltarget, bool inlinefromgraph, TR_PrexArgInfo *argInfo = 0, TR::TreeTop** cursorTreeTop = NULL);
      bool inlineCallTarget2(TR_CallStack *, TR_CallTarget *calltarget, TR::TreeTop** cursorTreeTop, bool inlinefromgraph, int32_t);

      //inlineCallTarget2 methods
      bool tryToInlineTrivialMethod (TR_CallStack* callStack, TR_CallTarget* calltarget);

      bool tryToGenerateILForMethod (TR::ResolvedMethodSymbol* calleeSymbol, TR::ResolvedMethodSymbol* callerSymbol, TR_CallTarget* calltarget);

      void inlineFromGraph(TR_CallStack *, TR_CallTarget *calltarget, TR_InnerPreexistenceInfo *);
      TR_CallSite* findAndUpdateCallSiteInGraph(TR_CallStack *callStack, TR_ByteCodeInfo &bcInfo, TR::TreeTop *tt, TR::Node *parent, TR::Node *callNode, TR_CallTarget *calltarget);

      void initializeControlFlowInfo(TR_CallStack *,TR::ResolvedMethodSymbol *);

      void cleanup(TR::ResolvedMethodSymbol *, bool);


      int checkInlineableWithoutInitialCalleeSymbol(TR_CallSite* callsite, TR::Compilation* comp);

      void getBorderFrequencies(int32_t &hotBorderFrequency, int32_t &coldBorderFrequency, TR_ResolvedMethod * calleeResolvedMethod, TR::Node *callNode);
      int32_t scaleSizeBasedOnBlockFrequency(int32_t bytecodeSize, int32_t frequency, int32_t borderFrequency, TR_ResolvedMethod * calleeResolvedMethod, TR::Node *callNode, int32_t coldBorderFrequency = 0); // not virtual because we want base class always calling base version of this
      virtual bool exceedsSizeThreshold(TR_CallSite *callSite, int bytecodeSize, TR::Block * callNodeBlock, TR_ByteCodeInfo & bcInfo,  int32_t numLocals=0, TR_ResolvedMethod * caller = 0, TR_ResolvedMethod * calleeResolvedMethod = 0, TR::Node * callNode = 0, bool allConsts = false);
      virtual bool inlineCallTargets(TR::ResolvedMethodSymbol *, TR_CallStack *, TR_InnerPreexistenceInfo *info)
         {
         TR_ASSERT(0, "invalid call to OMR::Inliner::inlineCallTargets");
         return false;
         }
      TR::TreeTop * addGuardForVirtual(TR::ResolvedMethodSymbol *, TR::ResolvedMethodSymbol *, TR::TreeTop *, TR::Node *,
                                      TR_OpaqueClassBlock *, TR::TreeTop *, TR::TreeTop *, TR::TreeTop *, TR_TransformInlinedFunction&,
                                      List<TR::SymbolReference> &, TR_VirtualGuardSelection *, TR::TreeTop **, TR_CallTarget *calltarget=0);

      void addAdditionalGuard(TR::Node *callNode, TR::ResolvedMethodSymbol * calleeSymbol, TR_OpaqueClassBlock *thisClass, TR::Block *prevBlock, TR::Block *inlinedBody, TR::Block *slowPath, TR_VirtualGuardKind kind, TR_VirtualGuardTestType type, bool favourVftCompare, TR::CFG *callerCFG);
      void rematerializeCallArguments(TR_TransformInlinedFunction & tif,
   TR_VirtualGuardSelection *guard, TR::Node *callNode, TR::Block *block1, TR::TreeTop *rematPoint);
      void replaceCallNode(TR::ResolvedMethodSymbol *, TR::Node *, rcount_t, TR::TreeTop *, TR::Node *, TR::Node *);
      void linkOSRCodeBlocks();
      bool IsOSRFramesSizeUnderThreshold();
      bool heuristicForUsingOSR(TR::Node *, TR::ResolvedMethodSymbol *, TR::ResolvedMethodSymbol *, bool);

      TR::Node * createVirtualGuard(TR::Node *, TR::ResolvedMethodSymbol *, TR::TreeTop *, int16_t, TR_OpaqueClassBlock *, bool, TR_VirtualGuardSelection *);
   private:

      void replaceCallNodeReferences(TR::Node *, TR::Node *, uint32_t, TR::Node *, TR::Node *, rcount_t &, TR::NodeChecklist &visitedNodes);
      void cloneChildren(TR::Node *, TR::Node *, uint32_t);

   protected:
      enum
         {
         //Available                 = 0x0001,
         InlineVirtuals              = 0x0002,
         InlineSynchronized          = 0x0004,
         FirstPass                   = 0x0008,
         lastDummyEnum
         };

      TR::Optimizer *       _optimizer;
      TR_Memory *              _trMemory;
      List<TR::SymbolReference> _availableTemps;
      List<TR::SymbolReference> _availableBasicBlockTemps;
      flags16_t                _flags;
      vcount_t                 _visitCount;
      bool                     _inliningAsWeWalk;
      bool                     _EDODisableInlinedProfilingInfo;
      bool                     _disableTailRecursion;
      bool                     _disableInnerPrex;
      bool                      _isInLoop;

      int32_t                  _maxRecursiveCallByteCodeSizeEstimate;
      int32_t                  _callerWeightLimit;
      int32_t                  _methodByteCodeSizeThreshold;
      int32_t                  _methodInColdBlockByteCodeSizeThreshold;
      int32_t                  _methodInWarmBlockByteCodeSizeThreshold;
      int32_t                  _nodeCountThreshold;
      int32_t                  _maxInliningCallSites;

      int32_t                  _numAsyncChecks;
      bool                      _aggressivelyInlineInLoops;

      int32_t                   _currentNumberOfNodes;

      TR_LinkHead<TR_CallSite> _deadCallSites;

      TR::Node *_storeToCachedPrivateStatic;

      TR_InlinerTracer*         _tracer;
      OMR_InlinerPolicy *_policy;
      OMR_InlinerUtil*_util;
   };

}


class TR_InlinerBase : public OMR::InlinerConnector
   {
public:

   /**
    * @param[in] optimizer: the \c TR::Optimizer object
    * @param[in] opt: the \c TR::Optimization object
    */
   TR_InlinerBase(TR::Optimizer *optimizer, TR::Optimization *opt) :
      OMR::InlinerConnector(optimizer, opt) {}

   };


namespace TR { typedef Inliner TR_InlinerBase; }

#endif
