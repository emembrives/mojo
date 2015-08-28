// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file is auto-generated from
// gpu/command_buffer/build_gles2_cmd_buffer.py
// It's formatted by clang-format using chromium coding style:
//    clang-format -i -style=chromium filename
// DO NOT EDIT!

#ifndef MOJO_PUBLIC_PLATFORM_NATIVE_GLES2_IMPL_KHR_BLEND_EQUATION_ADVANCED_THUNKS_H_
#define MOJO_PUBLIC_PLATFORM_NATIVE_GLES2_IMPL_KHR_BLEND_EQUATION_ADVANCED_THUNKS_H_

#include <stddef.h>

#define GL_GLEXT_PROTOTYPES
#include "mojo/public/c/gpu/GLES2/gl2extmojo.h"

// Specifies the frozen API for the KHR_blend_equation_advanced extension.
#pragma pack(push, 8)
struct MojoGLES2ImplKHRBlendEquationAdvancedThunks {
  size_t size;  // Should be set to sizeof(*this).

#define VISIT_GL_CALL(Function, ReturnType, PARAMETERS, ARGUMENTS) \
  ReturnType(GL_APIENTRY* Function) PARAMETERS;
#include "mojo/public/platform/native/gles2/call_visitor_khr_blend_equation_advanced_autogen.h"
#undef VISIT_GL_CALL
};
#pragma pack(pop)

#ifdef __cplusplus
// Intended to be called from the embedder to get the embedder's implementation
// of khr_blend_equation_advanced.
inline MojoGLES2ImplKHRBlendEquationAdvancedThunks
MojoMakeGLES2ImplKHRBlendEquationAdvancedThunks() {
  MojoGLES2ImplKHRBlendEquationAdvancedThunks
      gles2_impl_khr_blend_equation_advanced_thunks = {
          sizeof(MojoGLES2ImplKHRBlendEquationAdvancedThunks),
#define VISIT_GL_CALL(Function, ReturnType, PARAMETERS, ARGUMENTS) gl##Function,
#include "mojo/public/platform/native/gles2/call_visitor_khr_blend_equation_advanced_autogen.h"
#undef VISIT_GL_CALL
      };

  return gles2_impl_khr_blend_equation_advanced_thunks;
}
#endif  // __cplusplus

// Use this type for the function found by dynamically discovering it in
// a DSO linked with mojo_system.
// The contents of |gles2_impl_khr_blend_equation_advanced_thunks| are copied.
typedef size_t (*MojoSetGLES2ImplKHRBlendEquationAdvancedThunksFn)(
    const MojoGLES2ImplKHRBlendEquationAdvancedThunks* thunks);

#endif  // MOJO_PUBLIC_PLATFORM_NATIVE_GLES2_IMPL_KHR_BLEND_EQUATION_ADVANCED_THUNKS_H_
