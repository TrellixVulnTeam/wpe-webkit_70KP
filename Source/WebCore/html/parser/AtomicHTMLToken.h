/*
 * Copyright (C) 2013 Google, Inc. All Rights Reserved.
 * Copyright (C) 2015 Apple Inc. All Rights Reserved.
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

#pragma once

#include "HTMLToken.h"

namespace WebCore {

class AtomicHTMLToken {
public:
    explicit AtomicHTMLToken(HTMLToken&);
    AtomicHTMLToken(HTMLToken::Type, const AtomicString& name, Vector<Attribute>&& = Vector<Attribute>()); // Only StartTag or EndTag.

    HTMLToken::Type type() const;

    // StartTag, EndTag, DOCTYPE.

    void setName(const AtomicString&);

    const AtomicString& name() const;

    // DOCTYPE.

    bool forceQuirks() const;
    String publicIdentifier() const;
    String systemIdentifier() const;

    // StartTag, EndTag.

    Vector<Attribute>& attributes();

    bool selfClosing() const;
    const Vector<Attribute>& attributes() const;

    // Characters

    const UChar* characters() const;
    unsigned charactersLength() const;
    bool charactersIsAll8BitData() const;

    // Comment

    const String& comment() const;

private:
    HTMLToken::Type m_type;

    void initializeAttributes(const HTMLToken::AttributeList& attributes);

    AtomicString m_name; // StartTag, EndTag, DOCTYPE.

    String m_data; // Comment

    // We don't want to copy the the characters out of the HTMLToken, so we keep a pointer to its buffer instead.
    // This buffer is owned by the HTMLToken and causes a lifetime dependence between these objects.
    // FIXME: Add a mechanism for "internalizing" the characters when the HTMLToken is destroyed.
    const UChar* m_externalCharacters; // Character
    unsigned m_externalCharactersLength; // Character
    bool m_externalCharactersIsAll8BitData; // Character

    std::unique_ptr<DoctypeData> m_doctypeData; // DOCTYPE.

    bool m_selfClosing; // StartTag, EndTag.
    Vector<Attribute> m_attributes; // StartTag, EndTag.
};

Attribute* findAttribute(Vector<Attribute>&, const QualifiedName&);

inline HTMLToken::Type AtomicHTMLToken::type() const
{
    return m_type;
}

inline const AtomicString& AtomicHTMLToken::name() const
{
    ASSERT(m_type == HTMLToken::StartTag || m_type == HTMLToken::EndTag || m_type == HTMLToken::DOCTYPE);
    return m_name;
}

inline void AtomicHTMLToken::setName(const AtomicString& name)
{
    ASSERT(m_type == HTMLToken::StartTag || m_type == HTMLToken::EndTag || m_type == HTMLToken::DOCTYPE);
    m_name = name;
}

inline bool AtomicHTMLToken::selfClosing() const
{
    ASSERT(m_type == HTMLToken::StartTag || m_type == HTMLToken::EndTag);
    return m_selfClosing;
}

inline Vector<Attribute>& AtomicHTMLToken::attributes()
{
    ASSERT(m_type == HTMLToken::StartTag || m_type == HTMLToken::EndTag);
    return m_attributes;
}

inline const Vector<Attribute>& AtomicHTMLToken::attributes() const
{
    ASSERT(m_type == HTMLToken::StartTag || m_type == HTMLToken::EndTag);
    return m_attributes;
}

inline const UChar* AtomicHTMLToken::characters() const
{
    ASSERT(m_type == HTMLToken::Character);
    return m_externalCharacters;
}

inline unsigned AtomicHTMLToken::charactersLength() const
{
    ASSERT(m_type == HTMLToken::Character);
    return m_externalCharactersLength;
}

inline bool AtomicHTMLToken::charactersIsAll8BitData() const
{
    return m_externalCharactersIsAll8BitData;
}

inline const String& AtomicHTMLToken::comment() const
{
    ASSERT(m_type == HTMLToken::Comment);
    return m_data;
}

inline bool AtomicHTMLToken::forceQuirks() const
{
    ASSERT(m_type == HTMLToken::DOCTYPE);
    return m_doctypeData->forceQuirks;
}

inline String AtomicHTMLToken::publicIdentifier() const
{
    ASSERT(m_type == HTMLToken::DOCTYPE);
    if (!m_doctypeData->hasPublicIdentifier)
        return String();
    return StringImpl::create8BitIfPossible(m_doctypeData->publicIdentifier);
}

inline String AtomicHTMLToken::systemIdentifier() const
{
    if (!m_doctypeData->hasSystemIdentifier)
        return String();
    return StringImpl::create8BitIfPossible(m_doctypeData->systemIdentifier);
}

inline Attribute* findAttribute(Vector<Attribute>& attributes, const QualifiedName& name)
{
    for (auto& attribute : attributes) {
        if (attribute.name().matches(name))
            return &attribute;
    }
    return nullptr;
}

inline void AtomicHTMLToken::initializeAttributes(const HTMLToken::AttributeList& attributes)
{
    unsigned size = attributes.size();
    if (!size)
        return;

    m_attributes.reserveInitialCapacity(size);
    for (auto& attribute : attributes) {
        if (attribute.name.isEmpty())
            continue;

        QualifiedName name(nullAtom, AtomicString(attribute.name), nullAtom);

        // FIXME: This is N^2 for the number of attributes.
        if (!findAttribute(m_attributes, name))
            m_attributes.append(Attribute(name, AtomicString(attribute.value)));
    }
}

inline AtomicHTMLToken::AtomicHTMLToken(HTMLToken& token)
    : m_type(token.type())
{
    switch (m_type) {
    case HTMLToken::Uninitialized:
        ASSERT_NOT_REACHED();
        return;
    case HTMLToken::DOCTYPE:
        m_name = AtomicString(token.name());
        m_doctypeData = token.releaseDoctypeData();
        return;
    case HTMLToken::EndOfFile:
        return;
    case HTMLToken::StartTag:
    case HTMLToken::EndTag:
        m_selfClosing = token.selfClosing();
        m_name = AtomicString(token.name());
        initializeAttributes(token.attributes());
        return;
    case HTMLToken::Comment:
        if (token.commentIsAll8BitData())
            m_data = String::make8BitFrom16BitSource(token.comment());
        else
            m_data = String(token.comment());
        return;
    case HTMLToken::Character:
        m_externalCharacters = token.characters().data();
        m_externalCharactersLength = token.characters().size();
        m_externalCharactersIsAll8BitData = token.charactersIsAll8BitData();
        return;
    }
    ASSERT_NOT_REACHED();
}

inline AtomicHTMLToken::AtomicHTMLToken(HTMLToken::Type type, const AtomicString& name, Vector<Attribute>&& attributes)
    : m_type(type)
    , m_name(name)
    , m_selfClosing(false)
    , m_attributes(WTFMove(attributes))
{
    ASSERT(type == HTMLToken::StartTag || type == HTMLToken::EndTag);
}

} // namespace WebCore
