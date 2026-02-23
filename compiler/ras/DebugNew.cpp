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


const char *OMR::SymbolReference::getName(TR::Region &region, TR::Compilation *comp)
{
    const int32_t bufLen = 128;
    char buf[bufLen];
    const char *nameBuf = NULL;

    /**
     * Print to the local array first to determine how much space is needed.
     * Then, allocate the exact size from the provided region.
     */
    OMR::MemoryBufferLogger log(buf, bufLen);
    int32_t res = self()->printName(&log, comp);
    if (res >= 0) {
        res = strlen(buf) + 1; // +1 for NUL char
        nameBuf = (char *)region.allocate(res);
        if (nameBuf) {
            strncpy(nameBuf, buf, res); // includes the NUL char
        }
    }

    return nameBuf ? nameBuf : "<unknown symref>";
}

int32_t OMR::SymbolReference::printName(OMR::Logger *log, TR::Compilation *comp)
{
    TR::SymbolReferenceTable *symRefTab = comp->getSymRefTab();
    int32_t numHelperSymbols = symRefTab->getNumHelperSymbols();
    int32_t nonhelperIndex = symRefTab->getNonhelperIndex(symRefTab->getLastCommonNonhelperSymbol());
    int32_t index = self()->getReferenceNumber();
    const char *helperName = NULL;

    TR_Debug *debug = comp->debug();

    if (index < numHelperSymbols) {
        helperName = debug->getRuntimeHelperName(index);
    } else if (index < nonhelperIndex) {
        if (index >= numHelperSymbols + TR::SymbolReferenceTable::firstArrayShadowSymbol
            && index < numHelperSymbols + TR::SymbolReferenceTable::firstArrayShadowSymbol + TR::NumAllTypes) {
            helperName = "<array-shadow>";
        } else if (index >= numHelperSymbols + TR::SymbolReferenceTable::firstPerCodeCacheHelperSymbol
            && index <= numHelperSymbols + TR::SymbolReferenceTable::lastPerCodeCacheHelperSymbol) {
            helperName = self()->getPerCodeCacheHelperName((TR_CCPreLoadedCode)(index - numHelperSymbols
                - TR::SymbolReferenceTable::firstPerCodeCacheHelperSymbol));
        } else {
            switch (index - numHelperSymbols) {
                case TR::SymbolReferenceTable::contiguousArraySizeSymbol:
                    helperName = "<contiguous-array-size>";
                    break;
                case TR::SymbolReferenceTable::discontiguousArraySizeSymbol:
                    helperName = "<discontiguous-array-size>";
                    break;
                case TR::SymbolReferenceTable::arrayClassRomPtrSymbol:
                    helperName = "<array-class-rom-ptr>";
                    break;
                case TR::SymbolReferenceTable::vftSymbol:
                    helperName = "<vft-symbol>";
                    break;
                case TR::SymbolReferenceTable::currentThreadSymbol:
                    helperName = "<current-thread>";
                    break;
                case TR::SymbolReferenceTable::thisRangeExtensionSymbol:
                    helperName = "<this-range-extension>";
                    break;
                case TR::SymbolReferenceTable::recompilationCounterSymbol:
                    helperName = "<recompilation-counter>";
                    break;
                case TR::SymbolReferenceTable::counterAddressSymbol:
                    helperName = "<recompilation-counter-address>";
                    break;
                case TR::SymbolReferenceTable::countForRecompileSymbol:
                    helperName = "<count-for-recompile>";
                    break;
                case TR::SymbolReferenceTable::gcrPatchPointSymbol:
                    helperName = "<gcr-patch-point>";
                    break;
                case TR::SymbolReferenceTable::startPCSymbol:
                    helperName = "<start-PC>";
                    break;
                case TR::SymbolReferenceTable::compiledMethodSymbol:
                    helperName = "<J9Method>";
                    break;
                case TR::SymbolReferenceTable::excpSymbol:
                    helperName = "<exception-symbol>";
                    break;
                case TR::SymbolReferenceTable::indexableSizeSymbol:
                    helperName = "<indexable-size>";
                    break;
                case TR::SymbolReferenceTable::resolveCheckSymbol:
                    helperName = "<resolve check>";
                    break;
                case TR::SymbolReferenceTable::arraySetSymbol:
                    helperName = "<arrayset>";
                    break;
                case TR::SymbolReferenceTable::arrayCopySymbol:
                    helperName = "<arraycopy>";
                    break;
                case TR::SymbolReferenceTable::prefetchSymbol:
                    helperName = "<prefetch>";
                    break;
                case TR::SymbolReferenceTable::arrayTranslateSymbol:
                    helperName = "<arraytranslate>";
                    break;
                case TR::SymbolReferenceTable::arrayTranslateAndTestSymbol:
                    helperName = "<arraytranslateandtest>";
                    break;
                case TR::SymbolReferenceTable::long2StringSymbol:
                    helperName = "<long2String>";
                    break;
                case TR::SymbolReferenceTable::bitOpMemSymbol:
                    helperName = "<bitOpMem>";
                    break;
                case TR::SymbolReferenceTable::reverseLoadSymbol:
                    helperName = "<reverse-load>";
                    break;
                case TR::SymbolReferenceTable::reverseStoreSymbol:
                    helperName = "<reverse-store>";
                    break;
                case TR::SymbolReferenceTable::arrayCmpSymbol:
                    helperName = "<arraycmp>";
                    break;
                case TR::SymbolReferenceTable::arrayCmpLenSymbol:
                    helperName = "<arraycmplen>";
                    break;
                case TR::SymbolReferenceTable::currentTimeMaxPrecisionSymbol:
                    helperName = "<currentTimeMaxPrecision>";
                    break;
                case TR::SymbolReferenceTable::encodeASCIISymbol:
                    helperName = "<encodeASCII>";
                    break;
                case TR::SymbolReferenceTable::singlePrecisionSQRTSymbol:
                    helperName = "<fsqrt>";
                    break;
                case TR::SymbolReferenceTable::killsAllMethodSymbol:
                    helperName = "<killsAllMethod>";
                    break;
                case TR::SymbolReferenceTable::usesAllMethodSymbol:
                    helperName = "<usesAllMethod>";
                    break;
                case TR::SymbolReferenceTable::synchronizedFieldLoadSymbol:
                    helperName = "<synchronizedFieldLoad>";
                    break;
                case TR::SymbolReferenceTable::atomicAddSymbol:
                    helperName = "<atomicAdd>";
                    break;
                case TR::SymbolReferenceTable::atomicFetchAndAddSymbol:
                    helperName = "<atomicFetchAndAdd>";
                    break;
                case TR::SymbolReferenceTable::atomicFetchAndAdd32BitSymbol:
                    helperName = "<atomicFetchAndAdd32Bit>";
                    break;
                case TR::SymbolReferenceTable::atomicFetchAndAdd64BitSymbol:
                    helperName = "<atomicFetchAndAdd64Bit>";
                    break;
                case TR::SymbolReferenceTable::atomicSwapSymbol:
                    helperName = "<atomicSwap>";
                    break;
                case TR::SymbolReferenceTable::atomicSwap32BitSymbol:
                    helperName = "<atomicSwap32Bit>";
                    break;
                case TR::SymbolReferenceTable::atomicSwap64BitSymbol:
                    helperName = "<atomicSwap64Bit>";
                    break;
                case TR::SymbolReferenceTable::atomicCompareAndSwapReturnStatusSymbol:
                    helperName = "<atomicCompareAndSwapReturnStatus>";
                    break;
                case TR::SymbolReferenceTable::atomicCompareAndSwapReturnValueSymbol:
                    helperName = "<atomicCompareAndSwapReturnValue>";
                    break;
                case TR::SymbolReferenceTable::potentialOSRPointHelperSymbol:
                    helperName = "<potentialOSRPointHelper>";
                    break;
                case TR::SymbolReferenceTable::osrFearPointHelperSymbol:
                    helperName = "<osrFearPointHelper>";
                    break;
                case TR::SymbolReferenceTable::eaEscapeHelperSymbol:
                    helperName = "<eaEscapeHelper>";
                    break;
                case TR::SymbolReferenceTable::j9VMThreadTempSlotFieldSymbol:
                    helperName = "<j9VMThreadTempSlotFieldSymbol>";
                    break;
                case TR::SymbolReferenceTable::computedStaticCallSymbol:
                    helperName = "<computedStaticCall>";
                    break;
                case TR::SymbolReferenceTable::j9VMThreadFloatTemp1Symbol:
                    helperName = "<j9VMThreadFloatTemp1>";
                    break;
                case TR::SymbolReferenceTable::objectEqualityComparisonSymbol:
                    helperName = "<objectEqualityComparison>";
                    break;
                case TR::SymbolReferenceTable::objectInequalityComparisonSymbol:
                    helperName = "<objectInequalityComparison>";
                    break;
                case TR::SymbolReferenceTable::nonNullableArrayNullStoreCheckSymbol:
                    helperName = "<nonNullableArrayNullStoreCheck>";
                    break;
                case TR::SymbolReferenceTable::loadFlattenableArrayElementNonHelperSymbol:
                    helperName = "<loadFlattenableArrayElementNonHelper>";
                    break;
                case TR::SymbolReferenceTable::storeFlattenableArrayElementNonHelperSymbol:
                    helperName = "<storeFlattenableArrayElementNonHelper>";
                    break;
                case TR::SymbolReferenceTable::isIdentityObjectNonHelperSymbol:
                    helperName = "<isIdentityObject>";
                    break;
                case TR::SymbolReferenceTable::J9JNIMethodIDvTableIndexFieldSymbol:
                    helperName = "<J9JNIMethodIDvTableIndexField>";
                    break;
                case TR::SymbolReferenceTable::contiguousArrayDataAddrFieldSymbol:
                    helperName = "<contiguousArrayDataAddrField>";
                    break;
                case TR::SymbolReferenceTable::jitDispatchJ9MethodSymbol:
                    helperName = "<jitDispatchJ9Method>";
                    break;
                case TR::SymbolReferenceTable::jProfileValueSymbol:
                    helperName = "<jProfileValue>";
                    break;
                case TR::SymbolReferenceTable::jProfileValueWithNullCHKSymbol:
                    helperName = "<jProfileValueWithNullCHK>";
                    break;
                default:
                    break;
            }
        }
    }

    if (helperName) {
        return log->prints(helperName);
    }

    int32_t res;
    TR::Symbol *sym = self()->getSymbol();
    switch (sym->getKind()) {
        case TR::Symbol::IsAutomatic:
            res = self()->printAutoName(log, comp);
            break;
        case TR::Symbol::IsParameter:
            res = self()->printParmName(log, comp);
            break;
        case TR::Symbol::IsStatic:
            res = self()->printStaticName(log, comp);
            break;
        case TR::Symbol::IsResolvedMethod:
        case TR::Symbol::IsMethod:
            res = self()->printMethodName(log, comp);
            break;
        case TR::Symbol::IsShadow:
            res = self()->printShadowName(log, comp);
            break;
        case TR::Symbol::IsMethodMetaData:
            res = self()->printMetaDataName(log, comp);
            break;
        case TR::Symbol::IsLabel:
            res = log->prints(getName((TR::LabelSymbol *)sym));
            break;
        default:
            TR_ASSERT_FATAL(0, "unexpected symbol kind");
    }

    return res;
}

int32_t OMR::SymbolReference::printAutoName(OMR::Logger *log, TR::Compilation *comp)
{
    int32_t res;
    int32_t slot = self()->getCPIndex();
    TR::Symbol *sym = self()->getSymbol();

    TR_Debug *debug = comp->debug();

    if (sym->isSpillTempAuto()) {
        const char *spillName = (sym->getDataType() == TR::Float || sym->getDataType() == TR::Double) ?
            "#FPSPILL" : "#SPILL";
        res = log->printf("<%s%zu_%d " POINTER_PRINTF_FORMAT ">", spillName, sym->getSize(), self()->getReferenceNumber(), sym);
    } else if (self()->isTempVariableSizeSymRef()) {
        TR_ASSERT(sym->isVariableSizeSymbol(), "symRef #%d must contain a variable size symbol\n",
            self()->getReferenceNumber());
        TR::AutomaticSymbol *autoSym = sym->getVariableSizeSymbol();
        res = log->printf("<%s rc=%d>", getVSSName(autoSym), autoSym->getReferenceCount());
    } else if (sym->isPendingPush()) {
        log->printf("<pending push temp %d>", -slot - 1);
    } else if (slot < self()->getOwningMethodSymbol())
        int debugNameLen;
        const char *debugName = self()->getOwningMethod(comp)->localName(slot, 0, debugNameLen,
            comp->trMemory()); // TODO: Proper bcIndex somehow; TODO: proper length
        if (!debugName) {
            debugName = "";
            debugNameLen = 0;
        }
        debugNameLen = std::min(debugNameLen, 15); // Don't overrun the buffer

        const char *nameFormat;
        if (sym->castToAutoSymbol()->isPinningArrayPointer())
            nameFormat = "%.*s<pinning array auto slot %d>";
        else if (sym->castToAutoSymbol()->holdsMonitoredObject()) {
            if (self()->holdsMonitoredObjectForSyncMethod())
                nameFormat = "%.*s<auto slot %d holds monitoredObject syncMethod>";
            else
                nameFormat = "%.*s<auto slot %d holds monitoredObject>";
        } else {
           nameFormat = "%.*s<auto slot %d>";
        }

        res = log->printf(nameFormat, debugNameLen, debugName, slot);
    } else {
        const char *nameFormat;
        if (sym->castToAutoSymbol()->isInternalPointer())
            nameFormat = "<internal pointer temp slot %d>";
        else {
            if (sym->castToAutoSymbol()->isPinningArrayPointer())
                nameFormat = "<pinning array temp slot %d>";
            else if (sym->castToAutoSymbol()->holdsMonitoredObject()) {
                if (self()->holdsMonitoredObjectForSyncMethod())
                    nameFormat = "<temp slot %d holds monitoredObject syncMethod>";
                else
                    nameFormat = "<temp slot %d holds monitoredObject>";
            } else {
                nameFormat = "<temp slot %d>";
            }
        }

        res = log->printf(nameFormat, slot);
    }

    return res;
}

int32_t OMR::SymbolReference::printParmName(OMR::Logger *log, TR::Compilation *comp)
{
    int32_t parmNameLen, typeSigLen;
    int32_t slot = self()->getCPIndex();
    const char *typeSig = self()->getSymbol()->castToParmSymbol()->getTypeSignature(typeSigLen);
    const char *parmName = self()->getOwningMethod(comp)->localName(slot, 0, parmNameLen,
        comp->trMemory()); // TODO: Proper bcIndex somehow; TODO: proper length

    if (!parmName) {
        parmName = "";
        parmNameLen = 0;
    }

    int32_t res;
    if (slot == 0 && !self()->getOwningMethodSymbol(comp)->isStatic()) {
        res = log->printf("%.*s<'this' parm %.*s>", parmNameLen, parmName, typeSigLen, typeSig);
    } else {
        res = log->printf("%.*s<parm %d %.*s>", parmNameLen, parmName, self()->getCPIndex(), typeSigLen, typeSig);
    }

    return res;
}

int32_t OMR::SymbolReference::printMethodName(OMR::Logger *log, TR::Compilation *comp)
{
    TR::Method *method = self()->getSymbol()->castToMethodSymbol()->getMethod();

    int32_t res;
    if (method == NULL) {
        res = log->prints("unknown");
    } else {
        res = log->prints(method->signature(comp->trMemory(), stackAlloc));
    }

    return res;
}

int32_t OMR::SymbolReference::printStaticName(OMR::Logger *log, TR::Compilation *comp)
{
    TR::StaticSymbol *sym = self()->getSymbol()->castToStaticSymbol();
    void *staticAddress = sym->getStaticAddress();

    const char *staticName = NULL;

    if (sym->isClassObject()) {
        if (!sym->addressIsCPIndexOfStatic() && staticAddress) {
            int32_t len;
            const char *name = TR::Compiler->cls.classNameChars(comp, self(), len);
            if (name) {
                log->printf("%.*s", len, name);
                return;
            }
        }

        staticName = "unknown class object";
    } else if (sym->isConstantPoolAddress()) {
        staticName = "<constant pool address>";
    } else if (sym->isAddressOfClassObject()) {
        staticName = "<address of class object>";
    } else if (sym->isConstString()) {
#ifdef J9_PROJECT_SPECIFIC
        TR::VMAccessCriticalSection getStaticNameCriticalSection(comp,
            TR::VMAccessCriticalSection::tryToAcquireVMAccess);

        if (!self()->isUnresolved() && getStaticNameCriticalSection.acquiredVMAccess()) {
            uintptr_t stringLocation = (uintptr_t)sym->castToStaticSymbol()->getStaticAddress();
            if (stringLocation) {
                uintptr_t string = comp->fej9()->getStaticReferenceFieldAtAddress(stringLocation);
                uint64_t length64 = comp->fej9()->getStringUTF8UnabbreviatedLength(string);
                if (length64 >= 1024) {
                    // Don't bother converting very long strings to UTF8 just to
                    // trace the first few and last few characters.
                    log->prints("<string (long text omitted)>");
                    return;
                }

                TR::StackMemoryRegion stackMemoryRegion(*comp->trMemory());
                size_t length = (size_t)length64;
                char *contents
                    = (char *)comp->trMemory()->allocateMemory(length + 1, stackAlloc, TR_MemoryBase::UnknownType);

                comp->fej9()->getStringUTF8(string, contents, length + 1);

                /*
                 * If the string exceeds LENGTH_LIMIT, only output a section extracted
                 * from the beginning of the string and a section extracted from the
                 * end, separated by an ellipsis.
                 *
                 * This is to keep the logs tidy when outputting excessively long strings.
                 */

                // Use ellipsis if the string is too long
                const size_t LENGTH_LIMIT = 80;
                size_t prefixLength = length;
                size_t suffixLength = 0;
                const char *etc = "";
                size_t etcLength = 0;
                if (length > LENGTH_LIMIT) {
                    etc = "\"...\"";
                    etcLength = 5;
                    suffixLength = (LENGTH_LIMIT - etcLength) / 2;
                    prefixLength = LENGTH_LIMIT - etcLength - suffixLength;
                }

                // Replace unprintable characters with ?
                for (size_t i = 0; i < prefixLength; i++) {
                    if (!isprint(contents[i])) {
                        contents[i] = '?';
                    }
                }

                for (size_t i = length - suffixLength; i < length; i++) {
                    if (!isprint(contents[i])) {
                        contents[i] = '?';
                    }
                }

                return log->printf("<string \"%.*s%s%s\">", (int32_t)prefixLength, contents, etc,
                    contents + (length - suffixLength));
            }
        }
#endif
        staticName = "<string>";
    } else if (sym->isConstMethodType()) {
        staticName = "<method type>"; // TODO: Print the signature
    } else if (sym->isConstMethodHandle()) {
        staticName = "<method handle>"; // TODO: Print some kind of identification
    } else if (sym->isConstObjectRef()) {
        staticName = "<constant object ref>";
    } else if (sym->isConst()) {
        staticName = "<constant>";
    } else if (self()->getCPIndex() >= 0) {
        staticName = self()->getOwningMethod(comp)->staticName(self()->getCPIndex(), comp->trMemory());
    } else if (comp->getSymRefTab()->isVtableEntrySymbolRef(self())) {
        staticName = "<class_loader>";
    }

    int32_t res;
    if (staticName) {
        res = log->prints(staticName);
    }

#ifdef J9_PROJECT_SPECIFIC
    else if (sym->isCallSiteTableEntry()) {
        res = log->printf("<callSite entry @%d " POINTER_PRINTF_FORMAT ">",
            sym->castToCallSiteTableEntrySymbol()->getCallSiteIndex(), staticAddress);
    } else if (sym->isMethodTypeTableEntry()) {
        res = log->printf("<methodType entry @%d " POINTER_PRINTF_FORMAT ">",
            sym->castToMethodTypeTableEntrySymbol()->getMethodTypeIndex(), staticAddress);
    }
#endif

    else if (_comp->getSymRefTab()->isConstantAreaSymbol(sym) && sym->isStatic() && sym->isNamed()) {
        res = log->prints(sym->getName());
    } else if (staticAddress) {
        res = log->printf(POINTER_PRINTF_FORMAT, staticAddress);
    } else {
        res = log->prints("unknown static");
    }

    return res;
}

// Note: This array needs to match up with what is in compile/SymbolReferenceTable.hpp
static const char *commonNonhelperSymbolNames[]
    = { "<contiguousArraySize>", "<discontiguousArraySize>", "<arrayClassRomPtr>", "<classRomPtr>",
          "<javaLangClassFromClass>", "<classFromJavaLangClass>", "<addressOfClassOfMethod>", "<ramStaticsFromClass>",
          "<componentClass>", "<componentClassAsPrimitive>", "<isArray>", "<isClassDepthAndFlags>",
          "<initializeStatusFromClass>", "<isClassFlags>", "<vft>", "<currentThread>", "<recompilationCounter>",
          "<excp>", "<indexableSize>", "<resolveCheck>", "<arrayTranslate>", "<arrayTranslateAndTest>", "<long2String>",
          "<bitOpMem>", "<reverseLoad>", "<reverseStore>", "<currentTimeMaxPrecision>", "<encodeASCII>",
          "<headerFlags>", "<singlePrecisionSQRT>", "<threadPrivateFlags>", "<arrayletSpineFirstElement>", "<dltBlock>",
          "<countForRecompile>", "<gcrPatchPoint>", "<counterAddress>", "<startPC>", "<compiledMethod>",
          "<thisRangeExtension>", "<profilingBufferCursor>", "<profilingBufferEnd>", "<profilingBuffer>", "<osrBuffer>",
          "<osrScratchBuffer>", "<osrFrameIndex>", "<osrReturnAddress>", "<contiguousArrayDataAddrField>",
          "<potentialOSRPointHelper>", "<osrFearPointHelper>", "<eaEscapeHelper>", "<lowTenureAddress>",
          "<highTenureAddress>", "<fragmentParent>", "<globalFragment>", "<instanceShape>", "<instanceDescription>",
          "<descriptionWordFromPtr>", "<classFromJavaLangClassAsPrimitive>", "<javaVM>", "<heapBase>", "<heapTop>",
          "<j9methodExtraField>", "<j9methodConstantPoolField>", "<startPCLinkageInfo>", "<instanceShapeFromROMClass>",
          "<objectEqualityComparison>", "<objectInequalityComparison>", "<nonNullableArrayNullStoreCheck>",
          "<loadFlattenableArrayElementNonHelper>", "<storeFlattenableArrayElementNonHelper>", "<isIdentityObject>",
          "<synchronizedFieldLoad>", "<atomicAdd>", "<atomicFetchAndAdd>", "<atomicFetchAndAdd32Bit>",
          "<atomicFetchAndAdd64Bit>", "<atomicSwap>", "<atomicSwap32Bit>", "<atomicSwap64Bit>",
          "<atomicCompareAndSwapReturnStatus>", "<atomicCompareAndSwapReturnValue>", "<jProfileValueSymbol>",
          "<jProfileValueWithNullCHKSymbol>", "<j9VMThreadTempSlotField>", "<computedStaticCallSymbol>",
          "<j9VMThreadFloatTemp1>", "<J9JNIMethodIDvTableIndexFieldSymbol>", "<jitDispatchJ9Method>" };

int32_t OMR::SymbolReference::printShadowName(OMR::Logger *log, TR::Compilation *comp)
{
    TR::Symbol *sym = self()->getSymbol();
    TR::SymbolReferenceTable *symRefTab = comp->getSymRefTab();

    const char *shadowName = NULL;

    if (self()->getCPIndex() >= 0 && !sym->isArrayShadowSymbol()) {
        shadowName = self()->getOwningMethod(comp)->fieldName(self()->getCPIndex(), comp->trMemory());
    } else if (sym == symRefTab->findGenericIntShadowSymbol()) {
        if (self()->reallySharesSymbol(_comp)) {
            shadowName = "<generic int shadow>";
        } else {
            shadowName = "<immutable generic int shadow>";
        }
    } else if (symRefTab->isVtableEntrySymbolRef(self())) {
        shadowName = "<vtable-entry-symbol>";
    } else if (sym->isUnsafeShadowSymbol()) {
        shadowName = "<unsafe shadow sym>";
    } else if (self() == symRefTab->findHeaderFlagsSymbolRef()) {
        shadowName = "<object header flag word>";
    }

    if (sym) {
        if (symRefTab->isRefinedArrayShadow(self())) {
            shadowName = "<refined-array-shadow>";
        } else if (symRefTab->isImmutableArrayShadow(self())) {
            shadowName = "<immutable-array-shadow>";
        } else if (sym->isArrayletShadowSymbol()) {
            shadowName = "<arraylet-shadow>";
        } else if (sym->isGlobalFragmentShadowSymbol()) {
            shadowName = "<global-fragment>";
        } else if (sym->isMemoryTypeShadowSymbol()) {
            shadowName = "<memory-type>";
        } else if (sym->isNamedShadowSymbol()) {
            shadowName = sym->getNamedShadowSymbol()->getName();
        }
    }

    if (shadowName) {
        return log->prints(shadowName);
    }

    const int32_t numCommonNonhelperSymbols = TR::SymbolReferenceTable::lastCommonNonhelperSymbol
        - TR::SymbolReferenceTable::firstCommonNonhelperNonArrayShadowSymbol - TR_numCCPreLoadedCode;
    static_assert(sizeof(commonNonhelperSymbolNames) / sizeof(commonNonhelperSymbolNames[0])
            == numCommonNonhelperSymbols,
        "commonNonhelperSymbolNames array must match CommonNonhelperSymbol enumeration");

    for (int32_t i = TR::SymbolReferenceTable::firstCommonNonhelperNonArrayShadowSymbol;
         i < symRefTab->getLastCommonNonhelperSymbol(); i++) {
        TR::SymbolReference *other = symRefTab->element((TR::SymbolReferenceTable::CommonNonhelperSymbol)i);
        if (other && other->getSymbol() == sym) {
            return log->prints(commonNonhelperSymbolNames[i - TR::SymbolReferenceTable::firstCommonNonhelperNonArrayShadowSymbol]);
        }
    }

    return log->prints("unknown field");
}

int32_t OMR::SymbolReference::printMetaDataName(OMR::Logger *log, TR::Compilation *comp)
{
    const char *name = self()->getSymbol()->getMethodMetaDataSymbol()->getName();
    return log->prints(name ? name : "method meta data");
}

const char *OMR::SymbolReference::getPerCodeCacheHelperName(TR_CCPreLoadedCode helper)
{
#if defined(TR_TARGET_POWER)
    switch (helper) {
        case TR_AllocPrefetch:
            return "Alloc Prefetch";
        case TR_ObjAlloc:
            return "Object Alloc Helper";
        case TR_VariableLenArrayAlloc:
            return "Variable Length Array Alloc Helper";
        case TR_ConstLenArrayAlloc:
            return "Constant Length Array Alloc Helper";
        case TR_writeBarrier:
            return "Write Barrier Helper";
        case TR_writeBarrierAndCardMark:
            return "Write Barrier and Card Mark Helper";
        case TR_cardMark:
            return "Card Mark Helper";
        case TR_arrayStoreCHK:
            return "Array Store Check";
        default:
            break;
    }
#endif // TR_TARGET_POWER
    return "Unknown Helper";
}
