/*
 * Copyright (c) 2008, 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SKY_ENGINE_CORE_INSPECTOR_SCRIPTCALLSTACK_H_
#define SKY_ENGINE_CORE_INSPECTOR_SCRIPTCALLSTACK_H_

#include "gen/sky/core/InspectorTypeBuilder.h"
#include "sky/engine/core/inspector/ScriptCallFrame.h"
#include "sky/engine/platform/heap/Handle.h"
#include "sky/engine/wtf/Forward.h"
#include "sky/engine/wtf/RefCounted.h"
#include "sky/engine/wtf/Vector.h"

namespace blink {

class ScriptAsyncCallStack;

class ScriptCallStack final : public RefCounted<ScriptCallStack> {
    DECLARE_EMPTY_DESTRUCTOR_WILL_BE_REMOVED(ScriptCallStack);
public:
    static const size_t maxCallStackSizeToCapture = 200;

    static PassRefPtr<ScriptCallStack> create(Vector<ScriptCallFrame>&);

    const ScriptCallFrame &at(size_t) const;
    size_t size() const;

    PassRefPtr<ScriptAsyncCallStack> asyncCallStack() const;
    void setAsyncCallStack(PassRefPtr<ScriptAsyncCallStack>);

    PassRefPtr<TypeBuilder::Array<TypeBuilder::Console::CallFrame> > buildInspectorArray() const;

private:
    explicit ScriptCallStack(Vector<ScriptCallFrame>&);

    Vector<ScriptCallFrame> m_frames;
    RefPtr<ScriptAsyncCallStack> m_asyncCallStack;
};

} // namespace blink

#endif  // SKY_ENGINE_CORE_INSPECTOR_SCRIPTCALLSTACK_H_
