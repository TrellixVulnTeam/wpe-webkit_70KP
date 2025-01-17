/*
 * Copyright (C) 2008, 2010 Apple Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "CSSFunctionValue.h"

#include "CSSParserValues.h"
#include "CSSValueList.h"
#include <wtf/text/StringBuilder.h>

namespace WebCore {

CSSFunctionValue::CSSFunctionValue(CSSParserFunction* function)
    : CSSValue(FunctionClass)
    , m_nameDeprecated(function->name)
    , m_args(function->args ? RefPtr<CSSValueList>(CSSValueList::createFromParserValueList(*function->args)) : nullptr)
{
}

CSSFunctionValue::CSSFunctionValue(const String& name, Ref<CSSValueList>&& args)
    : CSSValue(FunctionClass)
    , m_nameDeprecated(name)
    , m_args(WTFMove(args))
{
}

CSSFunctionValue::CSSFunctionValue(CSSValueID keyword)
    : CSSValue(FunctionClass)
    , m_name(keyword)
{
}
    
String CSSFunctionValue::customCSSText() const
{
    StringBuilder result;
    if (m_name != CSSValueInvalid) {
        result.append(getValueName(m_name));
        result.append('(');
    } else
        result.append(m_nameDeprecated); // Includes the '('
    if (m_args)
        result.append(m_args->cssText());
    result.append(')');
    return result.toString();
}

bool CSSFunctionValue::equals(const CSSFunctionValue& other) const
{
    return m_name == other.m_name && compareCSSValuePtr(m_args, other.m_args);
}

void CSSFunctionValue::append(Ref<CSSValue>&& value)
{
    if (!m_args)
        m_args = CSSValueList::createCommaSeparated();
    m_args->append(WTFMove(value));
}
    
bool CSSFunctionValue::buildParserValueSubstitutingVariables(CSSParserValue* result, const CustomPropertyValueMap& customProperties) const
{
    result->id = CSSValueInvalid;
    result->unit = CSSParserValue::Function;
    result->function = new CSSParserFunction;
    result->function->name.init(m_nameDeprecated);
    bool success = true;
    if (m_args) {
        CSSParserValueList* argList = new CSSParserValueList;
        success = m_args->buildParserValueListSubstitutingVariables(argList, customProperties);
        result->function->args.reset(argList);
    }
    return success;
}

}
