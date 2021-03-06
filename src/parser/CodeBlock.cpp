/*
 * Copyright (c) 2016-present Samsung Electronics Co., Ltd
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 */

#include "Escargot.h"
#include "CodeBlock.h"
#include "runtime/Context.h"
#include "interpreter/ByteCode.h"
#include "runtime/Environment.h"
#include "runtime/EnvironmentRecord.h"

namespace Escargot {

void* CallBoundFunctionData::operator new(size_t size)
{
    static bool typeInited = false;
    static GC_descr descr;
    if (!typeInited) {
        GC_word obj_bitmap[GC_BITMAP_SIZE(CallBoundFunctionData)] = { 0 };
        GC_set_bit(obj_bitmap, GC_WORD_OFFSET(CallBoundFunctionData, m_boundTargetFunction));
        GC_set_bit(obj_bitmap, GC_WORD_OFFSET(CallBoundFunctionData, m_boundThis));
        GC_set_bit(obj_bitmap, GC_WORD_OFFSET(CallBoundFunctionData, m_boundArguments));
        descr = GC_make_descriptor(obj_bitmap, GC_WORD_LEN(CallBoundFunctionData));
        typeInited = true;
    }
    return GC_MALLOC_EXPLICITLY_TYPED(size, descr);
}

void* CodeBlock::operator new(size_t size)
{
#ifdef GC_DEBUG
    return CustomAllocator<CodeBlock>().allocate(1);
#else
    static bool typeInited = false;
    static GC_descr descr;
    if (!typeInited) {
        GC_word obj_bitmap[GC_BITMAP_SIZE(CodeBlock)] = { 0 };
        GC_set_bit(obj_bitmap, GC_WORD_OFFSET(CodeBlock, m_context));
        GC_set_bit(obj_bitmap, GC_WORD_OFFSET(CodeBlock, m_byteCodeBlock));
        descr = GC_make_descriptor(obj_bitmap, GC_WORD_LEN(CodeBlock));
        typeInited = true;
    }
    return GC_MALLOC_EXPLICITLY_TYPED(size, descr);
#endif
}

void* InterpretedCodeBlock::operator new(size_t size)
{
#ifdef GC_DEBUG
    return CustomAllocator<InterpretedCodeBlock>().allocate(1);
#else
    static bool typeInited = false;
    static GC_descr descr;
    if (!typeInited) {
        GC_word obj_bitmap[GC_BITMAP_SIZE(InterpretedCodeBlock)] = { 0 };
        GC_set_bit(obj_bitmap, GC_WORD_OFFSET(InterpretedCodeBlock, m_context));
        GC_set_bit(obj_bitmap, GC_WORD_OFFSET(InterpretedCodeBlock, m_script));
        GC_set_bit(obj_bitmap, GC_WORD_OFFSET(InterpretedCodeBlock, m_identifierInfos));
        GC_set_bit(obj_bitmap, GC_WORD_OFFSET(InterpretedCodeBlock, m_parametersInfomation));
        GC_set_bit(obj_bitmap, GC_WORD_OFFSET(InterpretedCodeBlock, m_parentCodeBlock));
        GC_set_bit(obj_bitmap, GC_WORD_OFFSET(InterpretedCodeBlock, m_childBlocks));
        GC_set_bit(obj_bitmap, GC_WORD_OFFSET(InterpretedCodeBlock, m_byteCodeBlock));
#ifndef NDEBUG
        GC_set_bit(obj_bitmap, GC_WORD_OFFSET(InterpretedCodeBlock, m_scopeContext));
#endif
        descr = GC_make_descriptor(obj_bitmap, GC_WORD_LEN(InterpretedCodeBlock));
        typeInited = true;
    }
    return GC_MALLOC_EXPLICITLY_TYPED(size, descr);
#endif
}

CodeBlock::CodeBlock(Context* ctx, const NativeFunctionInfo& info)
    : m_context(ctx)
    , m_isConstructor(info.m_isConstructor)
    , m_isStrict(info.m_isStrict)
    , m_hasCallNativeFunctionCode(true)
    , m_isFunctionNameSaveOnHeap(false)
    , m_isFunctionNameExplicitlyDeclared(false)
    , m_canUseIndexedVariableStorage(true)
    , m_canAllocateEnvironmentOnStack(true)
    , m_needsComplexParameterCopy(false)
    , m_hasEval(false)
    , m_hasWith(false)
    , m_hasCatch(false)
    , m_hasYield(false)
    , m_inCatch(false)
    , m_inWith(false)
    , m_usesArgumentsObject(false)
    , m_isFunctionExpression(false)
    , m_isFunctionDeclaration(false)
    , m_isFunctionDeclarationWithSpecialBinding(false)
    , m_isArrowFunctionExpression(false)
    , m_isClassConstructor(info.m_isClassConstructor)
    , m_isInWithScope(false)
    , m_isEvalCodeInFunction(false)
    , m_isBindedFunction(false)
    , m_needsVirtualIDOperation(false)
    , m_needToLoadThisValue(false)
    , m_hasRestElement(false)
    , m_parameterCount(info.m_argumentCount)
    , m_functionName(info.m_name)
{
    auto data = new (PointerFreeGC) CallNativeFunctionData();
    m_nativeFunctionData = (CallNativeFunctionData*)data;

    data->m_ctorFn = info.m_nativeFunctionConstructor;
    data->m_fn = info.m_nativeFunction;
}

CodeBlock::CodeBlock(Context* ctx, AtomicString name, size_t argc, bool isStrict, bool isCtor, CallNativeFunctionData* info)
    : m_context(ctx)
    , m_isConstructor(isCtor)
    , m_isStrict(isStrict)
    , m_hasCallNativeFunctionCode(true)
    , m_isFunctionNameSaveOnHeap(false)
    , m_isFunctionNameExplicitlyDeclared(false)
    , m_canUseIndexedVariableStorage(true)
    , m_canAllocateEnvironmentOnStack(true)
    , m_needsComplexParameterCopy(false)
    , m_hasEval(false)
    , m_hasWith(false)
    , m_hasCatch(false)
    , m_hasYield(false)
    , m_inCatch(false)
    , m_inWith(false)
    , m_usesArgumentsObject(false)
    , m_isFunctionExpression(false)
    , m_isFunctionDeclaration(false)
    , m_isFunctionDeclarationWithSpecialBinding(false)
    , m_isArrowFunctionExpression(false)
    , m_isClassConstructor(false)
    , m_isInWithScope(false)
    , m_isEvalCodeInFunction(false)
    , m_isBindedFunction(false)
    , m_needsVirtualIDOperation(false)
    , m_needToLoadThisValue(false)
    , m_hasRestElement(false)
    , m_parameterCount(argc)
    , m_functionName(name)
    , m_nativeFunctionData(info)
{
}

static Value functionBindImpl(ExecutionState& state, Value thisValue, size_t calledArgc, Value* calledArgv, bool isNewExpression)
{
    CodeBlock* dataCb = state.executionContext()->lexicalEnvironment()->record()->asDeclarativeEnvironmentRecord()->asFunctionEnvironmentRecord()->functionObject()->codeBlock();
    CallBoundFunctionData* code = (CallBoundFunctionData*)(dataCb->nativeFunctionData());

    // Collect arguments info when current function is called.
    int mergedArgc = code->m_boundArgumentsCount + calledArgc;
    Value* mergedArgv = ALLOCA(mergedArgc * sizeof(Value), Value, state);
    if (code->m_boundArgumentsCount) {
        for (size_t i = 0; i < code->m_boundArgumentsCount; i++) {
            mergedArgv[i] = code->m_boundArguments[i];
        }
    }
    if (calledArgc) {
        memcpy(mergedArgv + code->m_boundArgumentsCount, calledArgv, sizeof(Value) * calledArgc);
    }

    FunctionObject* targetFunction = Value(code->m_boundTargetFunction).asFunction();
    Value receiver = isNewExpression ? thisValue : Value(code->m_boundThis);
    return FunctionObject::call(state, targetFunction, receiver, mergedArgc, mergedArgv, isNewExpression);
}

CodeBlock::CodeBlock(ExecutionState& state, FunctionObject* targetFunction, Value& boundThis, size_t boundArgc, Value* boundArgv)
    : m_context(state.context())
    , m_isConstructor(false)
    , m_isStrict(false)
    , m_hasCallNativeFunctionCode(true)
    , m_isFunctionNameSaveOnHeap(false)
    , m_isFunctionNameExplicitlyDeclared(false)
    , m_canUseIndexedVariableStorage(true)
    , m_canAllocateEnvironmentOnStack(true)
    , m_needsComplexParameterCopy(false)
    , m_hasEval(false)
    , m_hasWith(false)
    , m_hasCatch(false)
    , m_hasYield(false)
    , m_inCatch(false)
    , m_inWith(false)
    , m_usesArgumentsObject(false)
    , m_isFunctionExpression(false)
    , m_isFunctionDeclaration(false)
    , m_isFunctionDeclarationWithSpecialBinding(false)
    , m_isArrowFunctionExpression(false)
    , m_isClassConstructor(false)
    , m_isInWithScope(false)
    , m_isEvalCodeInFunction(false)
    , m_isBindedFunction(true)
    , m_needsVirtualIDOperation(false)
    , m_needToLoadThisValue(false)
    , m_hasRestElement(false)
    , m_functionName()
{
    CodeBlock* targetCodeBlock = targetFunction->codeBlock();

    size_t targetFunctionLength = targetCodeBlock->parameterCount();
    m_parameterCount = targetFunctionLength > boundArgc ? targetFunctionLength - boundArgc : 0;

    auto data = new CallBoundFunctionData();
    m_nativeFunctionData = data;

    data->m_fn = functionBindImpl;
    data->m_ctorFn = nullptr;
    data->m_boundTargetFunction = targetFunction;
    data->m_boundThis = boundThis;
    data->m_boundArgumentsCount = boundArgc;
    if (boundArgc) {
        data->m_boundArguments = (SmallValue*)GC_MALLOC(boundArgc * sizeof(SmallValue));
        for (size_t i = 0; i < boundArgc; i++) {
            data->m_boundArguments[i] = boundArgv[i];
        }
    } else {
        data->m_boundArguments = nullptr;
    }
}

InterpretedCodeBlock::InterpretedCodeBlock(Context* ctx, Script* script, StringView src, ASTScopeContext* scopeCtx, ExtendedNodeLOC sourceElementStart)
    : m_script(script)
    , m_src(src)
    , m_sourceElementStart(sourceElementStart)
    , m_shouldReparseArguments(false)
    , m_identifierOnStackCount(0)
    , m_identifierOnHeapCount(0)
    , m_parentCodeBlock(nullptr)
#ifndef NDEBUG
    , m_locStart(SIZE_MAX, SIZE_MAX, SIZE_MAX)
    , m_locEnd(SIZE_MAX, SIZE_MAX, SIZE_MAX)
    , m_scopeContext(nullptr)
#endif
{
    m_context = ctx;
    m_byteCodeBlock = nullptr;

    m_parameterCount = 0;
    m_isConstructor = false;
    m_hasCallNativeFunctionCode = false;
    m_isFunctionDeclaration = false;
    m_isFunctionDeclarationWithSpecialBinding = false;
    m_isFunctionExpression = false;
    m_isArrowFunctionExpression = false;
    m_isStrict = scopeCtx->m_isStrict;
    m_isClassConstructor = scopeCtx->m_isClassConstructor;

    m_hasEval = scopeCtx->m_hasEval;
    m_hasWith = scopeCtx->m_hasWith;
    m_hasCatch = scopeCtx->m_hasCatch;
    m_hasYield = scopeCtx->m_hasYield;
    m_inCatch = false;
    m_inWith = false;

    m_usesArgumentsObject = false;
    m_canUseIndexedVariableStorage = false;
    m_canAllocateEnvironmentOnStack = false;
    m_needsComplexParameterCopy = false;
    m_isInWithScope = false;
    m_isEvalCodeInFunction = false;
    m_isBindedFunction = false;
    m_needsVirtualIDOperation = false;
    m_needToLoadThisValue = false;
    m_hasRestElement = scopeCtx->m_hasRestElement;
    m_isFunctionNameExplicitlyDeclared = false;
    m_isFunctionNameSaveOnHeap = false;

    const ASTScopeContextNameInfoVector& innerIdentifiers = scopeCtx->m_names;

    for (size_t i = 0; i < innerIdentifiers.size(); i++) {
        IdentifierInfo info;
        info.m_name = innerIdentifiers[i].name();
        info.m_needToAllocateOnStack = false;
        info.m_isMutable = true;
        info.m_isExplicitlyDeclaredOrParameterName = innerIdentifiers[i].isExplicitlyDeclaredOrParameterName();
        info.m_indexForIndexedStorage = SIZE_MAX;
        m_identifierInfos.push_back(info);
    }
}

InterpretedCodeBlock::InterpretedCodeBlock(Context* ctx, Script* script, StringView src, ASTScopeContext* scopeCtx, ExtendedNodeLOC sourceElementStart, InterpretedCodeBlock* parentBlock)
    : m_script(script)
    , m_paramsSrc(scopeCtx->m_hasNonIdentArgument ? StringView(src, scopeCtx->m_paramsStart.index, scopeCtx->m_locStart.index) : StringView())
    , m_src(StringView(src, scopeCtx->m_locStart.index, scopeCtx->m_locEnd.index))
    , m_sourceElementStart(sourceElementStart)
    , m_identifierOnStackCount(0)
    , m_identifierOnHeapCount(0)
    , m_parentCodeBlock(parentBlock)
#ifndef NDEBUG
    , m_locStart(SIZE_MAX, SIZE_MAX, SIZE_MAX)
    , m_locEnd(SIZE_MAX, SIZE_MAX, SIZE_MAX)
    , m_scopeContext(nullptr)
#endif
{
    bool isFE = scopeCtx->m_nodeType == FunctionExpression || scopeCtx->m_nodeType == ArrowFunctionExpression;
    bool isFD = scopeCtx->m_nodeType == FunctionDeclaration;

    if (scopeCtx->m_needsSpecialInitialize)
        isFD = false;

    const AtomicStringTightVector& parameterNames = scopeCtx->m_parameters;

    m_context = ctx;
    m_byteCodeBlock = nullptr;
    m_functionName = scopeCtx->m_functionName;
    m_parameterCount = scopeCtx->m_hasRestElement ? parameterNames.size() - 1 : parameterNames.size();
    m_isConstructor = true;
    m_hasCallNativeFunctionCode = false;
    m_isStrict = scopeCtx->m_isStrict;
    m_hasEval = scopeCtx->m_hasEval;
    m_hasWith = scopeCtx->m_hasWith;
    m_hasCatch = scopeCtx->m_hasCatch;
    m_hasYield = scopeCtx->m_hasYield;
    m_inCatch = scopeCtx->m_inCatch;
    m_inWith = scopeCtx->m_inWith;
    m_usesArgumentsObject = false;
    m_isFunctionDeclaration = isFD;
    m_isFunctionDeclarationWithSpecialBinding = scopeCtx->m_needsSpecialInitialize;
    m_isFunctionExpression = isFE;
    m_isArrowFunctionExpression = scopeCtx->m_isArrowFunctionExpression;
    m_isConstructor = !scopeCtx->m_isArrowFunctionExpression;
    m_isClassConstructor = scopeCtx->m_isClassConstructor;
    m_isFunctionNameExplicitlyDeclared = false;
    m_isFunctionNameSaveOnHeap = false;
    m_needsComplexParameterCopy = false;
    m_isInWithScope = false;
    m_isEvalCodeInFunction = false;
    m_isBindedFunction = false;
    m_needsVirtualIDOperation = false;
    m_needToLoadThisValue = false;
    m_hasRestElement = scopeCtx->m_hasRestElement;
    m_shouldReparseArguments = scopeCtx->m_hasNonIdentArgument;

    m_parametersInfomation.resizeWithUninitializedValues(parameterNames.size());
    for (size_t i = 0; i < parameterNames.size(); i++) {
        m_parametersInfomation[i].m_name = parameterNames[i];
        m_parametersInfomation[i].m_isDuplicated = false;
    }

    m_canUseIndexedVariableStorage = !hasEvalWithYield() && !m_inCatch && !m_inWith;
    m_canAllocateEnvironmentOnStack = m_canUseIndexedVariableStorage;

    const ASTScopeContextNameInfoVector& innerIdentifiers = scopeCtx->m_names;
    for (size_t i = 0; i < innerIdentifiers.size(); i++) {
        IdentifierInfo info;
        info.m_name = innerIdentifiers[i].name();
        info.m_needToAllocateOnStack = m_canUseIndexedVariableStorage;
        info.m_isMutable = true;
        info.m_isExplicitlyDeclaredOrParameterName = innerIdentifiers[i].isExplicitlyDeclaredOrParameterName();
        info.m_indexForIndexedStorage = SIZE_MAX;
        m_identifierInfos.push_back(info);
    }
}

bool InterpretedCodeBlock::needToStoreThisValue()
{
    return hasName(m_context->staticStrings().stringThis);
}

void InterpretedCodeBlock::captureThis()
{
    ASSERT(!isGlobalScopeCodeBlock());

    if (hasName(m_context->staticStrings().stringThis)) {
        return;
    }

    m_canAllocateEnvironmentOnStack = false;

    IdentifierInfo info;
    info.m_name = m_context->staticStrings().stringThis;
    info.m_needToAllocateOnStack = false;
    info.m_isMutable = true;
    info.m_isExplicitlyDeclaredOrParameterName = false;
    info.m_indexForIndexedStorage = SIZE_MAX;
    m_identifierInfos.push_back(info);
}

void InterpretedCodeBlock::captureArguments()
{
    AtomicString arguments = m_context->staticStrings().arguments;
    ASSERT(!hasParameter(arguments));
    ASSERT(!isGlobalScopeCodeBlock() && !isArrowFunctionExpression());

    if (m_usesArgumentsObject) {
        return;
    }

    m_usesArgumentsObject = true;
    if (!hasName(arguments)) {
        IdentifierInfo info;
        info.m_indexForIndexedStorage = SIZE_MAX;
        info.m_name = arguments;
        info.m_needToAllocateOnStack = true;
        info.m_isMutable = true;
        m_identifierInfos.pushBack(info);
    }
    if (m_parameterCount) {
        m_canAllocateEnvironmentOnStack = false;
        for (size_t j = 0; j < m_parametersInfomation.size(); j++) {
            for (size_t k = 0; k < m_identifierInfos.size(); k++) {
                if (m_identifierInfos[k].m_name == m_parametersInfomation[j].m_name) {
                    m_identifierInfos[k].m_needToAllocateOnStack = false;
                    break;
                }
            }
        }
    }
}

bool InterpretedCodeBlock::tryCaptureIdentifiersFromChildCodeBlock(AtomicString name)
{
    for (size_t i = 0; i < m_identifierInfos.size(); i++) {
        if (m_identifierInfos[i].m_name == name) {
            m_canAllocateEnvironmentOnStack = false;
            m_identifierInfos[i].m_needToAllocateOnStack = false;
            return true;
        }
    }
    return false;
}

void InterpretedCodeBlock::notifySelfOrChildHasEvalWithYield()
{
    m_canAllocateEnvironmentOnStack = false;
    m_canUseIndexedVariableStorage = false;

    for (size_t i = 0; i < m_identifierInfos.size(); i++) {
        m_identifierInfos[i].m_indexForIndexedStorage = SIZE_MAX;
        m_identifierInfos[i].m_needToAllocateOnStack = false;
    }
}

// global block id map
// [this, ..]
// function block id map
// [this, functionName, arg0, arg1...]

void InterpretedCodeBlock::computeVariables()
{
    if (m_usesArgumentsObject) {
        m_canAllocateEnvironmentOnStack = false;
    }

    if (inEvalWithYieldScope() || inNotIndexedCodeBlockScope() || hasCatch()) {
        m_canAllocateEnvironmentOnStack = false;
    }

    if (!m_canAllocateEnvironmentOnStack) {
        CodeBlock* cb = parentCodeBlock();
        while (cb && cb->isInterpretedCodeBlock()) {
            cb->m_canAllocateEnvironmentOnStack = false;
            cb = cb->asInterpretedCodeBlock()->parentCodeBlock();
        }
    }

    if (m_functionName.string()->length()) {
        if (m_isFunctionExpression) {
            // name of function expression is immuable
            for (size_t i = 0; i < m_identifierInfos.size(); i++) {
                if (m_identifierInfos[i].m_name == m_functionName && !m_identifierInfos[i].m_isExplicitlyDeclaredOrParameterName) {
                    m_identifierInfos[i].m_isMutable = false;
                    m_needsComplexParameterCopy = true;
                    break;
                }
            }
        }

        for (size_t i = 0; i < m_parametersInfomation.size(); i++) {
            AtomicString name = m_parametersInfomation[i].m_name;
            if (name == m_functionName) {
                m_needsComplexParameterCopy = true;
                break;
            }
        }
    }

    if (canUseIndexedVariableStorage()) {
        size_t s = isGlobalScopeCodeBlock() ? 1 : 2;
        size_t h = 0;

        for (size_t i = 0; i < m_identifierInfos.size(); i++) {
            if (m_identifierInfos[i].m_name == m_functionName) {
                m_needsComplexParameterCopy = true;
                if (m_identifierInfos[i].m_isExplicitlyDeclaredOrParameterName) {
                    m_isFunctionNameExplicitlyDeclared = true;
                }

                if (!m_identifierInfos[i].m_needToAllocateOnStack) {
                    m_isFunctionNameSaveOnHeap = true;
                    m_identifierInfos[i].m_indexForIndexedStorage = h;
                    h++;
                } else {
                    if (m_isFunctionNameExplicitlyDeclared) {
                        s++;
                    }
                    m_identifierInfos[i].m_indexForIndexedStorage = 1;
                }
                continue;
            }
            if (m_identifierInfos[i].m_needToAllocateOnStack) {
                m_identifierInfos[i].m_indexForIndexedStorage = s;
                s++;
            } else {
                m_identifierInfos[i].m_indexForIndexedStorage = h;
                h++;
            }
        }

        m_identifierOnStackCount = s;
        m_identifierOnHeapCount = h;

        size_t siz = m_parametersInfomation.size();

        std::vector<std::pair<AtomicString, size_t>> computedNameIndex;

        for (size_t i = 0; i < siz; i++) {
            AtomicString name = m_parametersInfomation[i].m_name;
            size_t idIndex = findName(name);
            bool isHeap = !m_identifierInfos[idIndex].m_needToAllocateOnStack;
            size_t indexInIdInfo = m_identifierInfos[idIndex].m_indexForIndexedStorage;
            if (!isHeap) {
                indexInIdInfo -= 2;
            }
            m_parametersInfomation[i].m_isHeapAllocated = isHeap;
            m_parametersInfomation[i].m_name = name;

            if (name == m_functionName) {
                if (m_identifierInfos[idIndex].m_needToAllocateOnStack) {
                    m_parametersInfomation[i].m_index = -1;
                    m_parametersInfomation[i].m_isHeapAllocated = false;
                } else {
                    m_parametersInfomation[i].m_index = indexInIdInfo;
                    m_parametersInfomation[i].m_isHeapAllocated = true;
                }
                continue;
            }

            bool computed = false;
            size_t computedIndex = SIZE_MAX;
            for (size_t j = 0; j < computedNameIndex.size(); j++) {
                if (computedNameIndex[j].first == name) {
                    // found dup parameter name!
                    m_needsComplexParameterCopy = true;
                    computed = true;
                    computedIndex = computedNameIndex[j].second;

                    for (size_t k = i - 1;; k--) {
                        if (m_parametersInfomation[k].m_name == name) {
                            m_parametersInfomation[k].m_isDuplicated = true;
                            break;
                        }
                    }

                    break;
                }
            }

            if (!computed) {
                if (isHeap) {
                    m_parametersInfomation[i].m_index = indexInIdInfo;
                    m_needsComplexParameterCopy = true;
                    computedNameIndex.push_back(std::make_pair(name, indexInIdInfo));
                } else {
                    m_parametersInfomation[i].m_index = indexInIdInfo;
                    computedNameIndex.push_back(std::make_pair(name, indexInIdInfo));
                }
            } else {
                m_parametersInfomation[i].m_index = computedNameIndex[computedIndex].second;
            }
        }
    } else {
        m_needsComplexParameterCopy = true;

        if (m_isEvalCodeInFunction) {
            AtomicString arguments = m_context->staticStrings().arguments;
            for (size_t i = 0; i < m_identifierInfos.size(); i++) {
                if (m_identifierInfos[i].m_name == arguments) {
                    m_identifierInfos.erase(i);
                    break;
                }
            }
        }

        size_t s = isGlobalScopeCodeBlock() ? 1 : 2;
        size_t h = 0;
        for (size_t i = 0; i < m_identifierInfos.size(); i++) {
            m_identifierInfos[i].m_needToAllocateOnStack = false;

            if (m_identifierInfos[i].m_name == m_functionName) {
                if (m_identifierInfos[i].m_isExplicitlyDeclaredOrParameterName) {
                    m_isFunctionNameExplicitlyDeclared = true;
                }
                m_isFunctionNameSaveOnHeap = true;
                m_identifierInfos[i].m_indexForIndexedStorage = h;
                h++;
                continue;
            }
            m_identifierInfos[i].m_indexForIndexedStorage = h;
            h++;
        }

        size_t siz = m_parametersInfomation.size();
        for (size_t i = 0; i < siz; i++) {
            m_parametersInfomation[i].m_isHeapAllocated = true;
            m_parametersInfomation[i].m_index = std::numeric_limits<int32_t>::max();
        }

        m_identifierOnStackCount = s;
        m_identifierOnHeapCount = h;
    }
}
}
