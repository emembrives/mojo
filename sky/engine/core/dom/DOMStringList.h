/*
 * Copyright (C) 2010 Google Inc. All Rights Reserved.
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

#ifndef SKY_ENGINE_CORE_DOM_DOMSTRINGLIST_H_
#define SKY_ENGINE_CORE_DOM_DOMSTRINGLIST_H_

#include "sky/engine/bindings/core/v8/ScriptWrappable.h"
#include "sky/engine/platform/heap/Handle.h"
#include "sky/engine/wtf/PassRefPtr.h"
#include "sky/engine/wtf/RefCounted.h"
#include "sky/engine/wtf/Vector.h"
#include "sky/engine/wtf/text/WTFString.h"

namespace blink {

// FIXME: Some consumers of this class may benefit from lazily fetching items rather
//        than creating the list statically as is currently the only option.
class DOMStringList final : public RefCounted<DOMStringList>, public ScriptWrappable {
    DEFINE_WRAPPERTYPEINFO();
public:
    static PassRefPtr<DOMStringList> create()
    {
        return adoptRef(new DOMStringList());
    }

    bool isEmpty() const { return m_strings.isEmpty(); }
    void clear() { m_strings.clear(); }
    void append(const String& string) { m_strings.append(string); }
    void sort();

    // Implements the IDL.
    size_t length() const { return m_strings.size(); }
    String item(unsigned index) const;
    bool contains(const String& str) const;

    operator const Vector<String>&() const { return m_strings; }

private:
    DOMStringList()
    {
    }

    Vector<String> m_strings;
};

} // namespace blink

#endif  // SKY_ENGINE_CORE_DOM_DOMSTRINGLIST_H_
