// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "examples/bitmap_uploader/bitmap_uploader.h"

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif  // GL_GLEXT_PROTOTYPES

#include <GLES2/gl2.h>
#include <GLES2/gl2extmojo.h>
#include <MGL/mgl.h>

#include "base/bind.h"
#include "mojo/public/cpp/application/connect.h"
#include "mojo/public/interfaces/application/shell.mojom.h"
#include "mojo/services/geometry/public/cpp/geometry_util.h"
#include "mojo/services/surfaces/public/cpp/surfaces_utils.h"
#include "mojo/services/view_manager/public/cpp/lib/view_manager_client_impl.h"
#include "ui/gfx/geometry/rect.h"

#define TRANSPARENT_COLOR 0x00000000

namespace mojo {

namespace {
void LostContext(void*) {
  DCHECK(false);
}

}

BitmapUploader::BitmapUploader(View* view)
    : view_(view),
      color_(TRANSPARENT_COLOR),
      width_(0),
      height_(0),
      format_(BGRA),
      next_resource_id_(1u),
      id_namespace_(0u),
      local_id_(0u),
      returner_binding_(this) {
}

void BitmapUploader::Init(Shell* shell) {
  ServiceProviderPtr surfaces_service_provider;
  shell->ConnectToApplication("mojo:surfaces_service",
                              GetProxy(&surfaces_service_provider), nullptr);
  ConnectToService(surfaces_service_provider.get(), &surface_);
  surface_->GetIdNamespace(
      base::Bind(&BitmapUploader::SetIdNamespace, base::Unretained(this)));
  mojo::ResourceReturnerPtr returner_ptr;
  returner_binding_.Bind(GetProxy(&returner_ptr));
  surface_->SetResourceReturner(returner_ptr.Pass());

  ServiceProviderPtr gpu_service_provider;
  shell->ConnectToApplication("mojo:native_viewport_service",
                              GetProxy(&gpu_service_provider), nullptr);
  ConnectToService(gpu_service_provider.get(), &gpu_service_);

  CommandBufferPtr gles2_client;
  gpu_service_->CreateOffscreenGLES2Context(GetProxy(&gles2_client));
  mgl_context_ = MGLCreateContext(
      MGL_API_VERSION_GLES2,
      gles2_client.PassInterface().PassHandle().release().value(),
      MGL_NO_CONTEXT, &LostContext, nullptr,
      Environment::GetDefaultAsyncWaiter());
  MGLMakeCurrent(mgl_context_);
}

BitmapUploader::~BitmapUploader() {
  MGLDestroyContext(mgl_context_);
}

void BitmapUploader::SetColor(uint32_t color) {
  if (color_ == color)
    return;
  color_ = color;
  if (surface_)
    Upload();
}

void BitmapUploader::SetBitmap(int width,
                               int height,
                               scoped_ptr<std::vector<unsigned char>> data,
                               Format format) {
  width_ = width;
  height_ = height;
  bitmap_ = data.Pass();
  format_ = format;
  if (surface_)
    Upload();
}

void BitmapUploader::Upload() {
  Size size;
  size.width = view_->bounds().width;
  size.height = view_->bounds().height;
  if (!size.width || !size.height) {
    view_->SetSurfaceId(SurfaceId::New());
    return;
  }
  if (id_namespace_ == 0u)  // Can't generate a qualified ID yet.
    return;

  if (size != surface_size_) {
    if (local_id_ != 0u) {
      surface_->DestroySurface(local_id_);
    }
    local_id_++;
    surface_->CreateSurface(local_id_);
    surface_size_ = size;
    auto qualified_id = SurfaceId::New();
    qualified_id->id_namespace = id_namespace_;
    qualified_id->local = local_id_;
    view_->SetSurfaceId(qualified_id.Pass());
  }

  Rect bounds;
  bounds.width = size.width;
  bounds.height = size.height;
  PassPtr pass = CreateDefaultPass(1, bounds);
  FramePtr frame = Frame::New();
  frame->resources.resize(0u);

  pass->quads.resize(0u);
  pass->shared_quad_states.push_back(CreateDefaultSQS(size));

  MGLMakeCurrent(mgl_context_);
  if (bitmap_.get()) {
    Size bitmap_size;
    bitmap_size.width = width_;
    bitmap_size.height = height_;
    GLuint texture_id = BindTextureForSize(bitmap_size);
    glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    0,
                    0,
                    bitmap_size.width,
                    bitmap_size.height,
                    TextureFormat(),
                    GL_UNSIGNED_BYTE,
                    &((*bitmap_)[0]));

    GLbyte mailbox[GL_MAILBOX_SIZE_CHROMIUM];
    glGenMailboxCHROMIUM(mailbox);
    glProduceTextureCHROMIUM(GL_TEXTURE_2D, mailbox);
    GLuint sync_point = glInsertSyncPointCHROMIUM();

    TransferableResourcePtr resource = TransferableResource::New();
    resource->id = next_resource_id_++;
    resource_to_texture_id_map_[resource->id] = texture_id;
    resource->format = mojo::RESOURCE_FORMAT_RGBA_8888;
    resource->filter = GL_LINEAR;
    resource->size = bitmap_size.Clone();
    MailboxHolderPtr mailbox_holder = MailboxHolder::New();
    mailbox_holder->mailbox = Mailbox::New();
    for (int i = 0; i < GL_MAILBOX_SIZE_CHROMIUM; ++i)
      mailbox_holder->mailbox->name.push_back(mailbox[i]);
    mailbox_holder->texture_target = GL_TEXTURE_2D;
    mailbox_holder->sync_point = sync_point;
    resource->mailbox_holder = mailbox_holder.Pass();
    resource->is_repeated = false;
    resource->is_software = false;

    QuadPtr quad = Quad::New();
    quad->material = MATERIAL_TEXTURE_CONTENT;

    RectPtr rect = Rect::New();
    if (width_ <= size.width && height_ <= size.height) {
      rect->width = width_;
      rect->height = height_;
    } else {
      // The source bitmap is larger than the viewport. Resize it while
      // maintaining the aspect ratio.
      float width_ratio = static_cast<float>(width_) / size.width;
      float height_ratio = static_cast<float>(height_) / size.height;
      if (width_ratio > height_ratio) {
        rect->width = size.width;
        rect->height = height_ / width_ratio;
      } else {
        rect->height = size.height;
        rect->width = width_ / height_ratio;
      }
    }
    quad->rect = rect.Clone();
    quad->opaque_rect = rect.Clone();
    quad->visible_rect = rect.Clone();
    quad->needs_blending = true;
    quad->shared_quad_state_index = 0u;

    TextureQuadStatePtr texture_state = TextureQuadState::New();
    texture_state->resource_id = resource->id;
    texture_state->premultiplied_alpha = true;
    texture_state->uv_top_left = PointF::New();
    texture_state->uv_bottom_right = PointF::New();
    texture_state->uv_bottom_right->x = 1.f;
    texture_state->uv_bottom_right->y = 1.f;
    texture_state->background_color = Color::New();
    texture_state->background_color->rgba = TRANSPARENT_COLOR;
    for (int i = 0; i < 4; ++i)
      texture_state->vertex_opacity.push_back(1.f);
    texture_state->flipped = false;

    frame->resources.push_back(resource.Pass());
    quad->texture_quad_state = texture_state.Pass();
    pass->quads.push_back(quad.Pass());
  }

  if (color_ != TRANSPARENT_COLOR) {
    QuadPtr quad = Quad::New();
    quad->material = MATERIAL_SOLID_COLOR;
    quad->rect = bounds.Clone();
    quad->opaque_rect = Rect::New();
    quad->visible_rect = bounds.Clone();
    quad->needs_blending = true;
    quad->shared_quad_state_index = 0u;

    SolidColorQuadStatePtr color_state = SolidColorQuadState::New();
    color_state->color = Color::New();
    color_state->color->rgba = color_;
    color_state->force_anti_aliasing_off = false;

    quad->solid_color_quad_state = color_state.Pass();
    pass->quads.push_back(quad.Pass());
  }

  frame->passes.push_back(pass.Pass());

  surface_->SubmitFrame(local_id_, frame.Pass(), mojo::Closure());
}

void BitmapUploader::SetIdNamespace(uint32_t id_namespace) {
  id_namespace_ = id_namespace;
  if (color_ != TRANSPARENT_COLOR || bitmap_.get())
    Upload();
}

void BitmapUploader::ReturnResources(Array<ReturnedResourcePtr> resources) {
  MGLMakeCurrent(mgl_context_);
  // TODO(jamesr): Recycle.
  for (size_t i = 0; i < resources.size(); ++i) {
    ReturnedResourcePtr resource = resources[i].Pass();
    DCHECK_EQ(1, resource->count);
    glWaitSyncPointCHROMIUM(resource->sync_point);
    uint32_t texture_id = resource_to_texture_id_map_[resource->id];
    DCHECK_NE(0u, texture_id);
    resource_to_texture_id_map_.erase(resource->id);
    glDeleteTextures(1, &texture_id);
  }
}

uint32_t BitmapUploader::BindTextureForSize(const Size size) {
  // TODO(jamesr): Recycle textures.
  GLuint texture = 0u;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D,
               0,
               TextureFormat(),
               size.width,
               size.height,
               0,
               TextureFormat(),
               GL_UNSIGNED_BYTE,
               0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  return texture;
}

uint32_t BitmapUploader::TextureFormat() {
  return format_ == BGRA ? GL_BGRA_EXT : GL_RGBA;
}

}  // namespace mojo
