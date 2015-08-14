// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_WINDOW_MANAGER_WINDOW_MANAGER_IMPL_H_
#define SERVICES_WINDOW_MANAGER_WINDOW_MANAGER_IMPL_H_

#include "base/logging.h"
#include "base/macros.h"
#include "mojo/public/cpp/bindings/strong_binding.h"
#include "mojo/services/view_manager/public/cpp/types.h"
#include "mojo/services/window_manager/public/interfaces/window_manager.mojom.h"

namespace window_manager {

class WindowManagerRoot;

class WindowManagerImpl : public mojo::WindowManager {
 public:
  // See description above |from_vm_| for details on |from_vm|.
  // WindowManagerImpl deletes itself on connection errors.  WindowManagerRoot
  // also deletes WindowManagerImpl in its destructor.
  WindowManagerImpl(WindowManagerRoot* window_manager, bool from_vm);
  ~WindowManagerImpl() override;

  void Bind(mojo::ScopedMessagePipeHandle window_manager_pipe);

  void NotifyViewFocused(mojo::Id focused_id);
  void NotifyWindowActivated(mojo::Id active_id);
  void NotifyCaptureChanged(mojo::Id capture_id);

 private:
  // mojo::WindowManager:
  void Embed(const mojo::String& url,
             mojo::InterfaceRequest<mojo::ServiceProvider> services,
             mojo::ServiceProviderPtr exposed_services) override;
  void SetCapture(uint32_t view_id,
                  const mojo::Callback<void(bool)>& callback) override;
  void FocusWindow(uint32_t view_id,
                   const mojo::Callback<void(bool)>& callback) override;
  void ActivateWindow(uint32_t view_id,
                      const mojo::Callback<void(bool)>& callback) override;
  void GetFocusedAndActiveViews(
      mojo::WindowManagerObserverPtr observer,
      const mojo::WindowManager::GetFocusedAndActiveViewsCallback& callback)
      override;

  WindowManagerRoot* window_manager_;

  // Whether this connection originated from the ViewManager. Connections that
  // originate from the view manager are expected to have clients. Connections
  // that don't originate from the view manager do not have clients.
  const bool from_vm_;

  mojo::StrongBinding<mojo::WindowManager> binding_;
  mojo::WindowManagerObserverPtr observer_;

  DISALLOW_COPY_AND_ASSIGN(WindowManagerImpl);
};

}  // namespace window_manager

#endif  // SERVICES_WINDOW_MANAGER_WINDOW_MANAGER_IMPL_H_
