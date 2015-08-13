// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_VIEW_MANAGER_VIEW_MANAGER_ROOT_CONNECTION_H_
#define SERVICES_VIEW_MANAGER_VIEW_MANAGER_ROOT_CONNECTION_H_

#include "base/memory/scoped_ptr.h"
#include "mojo/common/tracing_impl.h"
#include "mojo/public/cpp/application/application_delegate.h"
#include "mojo/public/cpp/application/interface_factory.h"
#include "mojo/public/cpp/bindings/binding.h"
#include "mojo/services/view_manager/public/interfaces/view_manager.mojom.h"
#include "mojo/services/window_manager/public/interfaces/window_manager_internal.mojom.h"
#include "services/view_manager/connection_manager_delegate.h"

namespace view_manager {

class ViewManagerRootConnection
    : public mojo::InterfaceFactory<mojo::ViewManagerService>,
      public mojo::InterfaceFactory<mojo::WindowManagerInternalClient> {
 public:
  ViewManagerRootConnection(mojo::ApplicationImpl* application_impl,
                            mojo::ApplicationConnection* connection);

  void OnLostConnectionToWindowManager();

 private:
  // mojo::InterfaceFactory<mojo::ViewManagerService>:
  void Create(
      mojo::ApplicationConnection* connection,
      mojo::InterfaceRequest<mojo::ViewManagerService> request) override;

  // mojo::InterfaceFactory<mojo::WindowManagerInternalClient>:
  void Create(mojo::ApplicationConnection* connection,
              mojo::InterfaceRequest<mojo::WindowManagerInternalClient> request)
      override;

  mojo::ApplicationImpl* app_impl_;
  scoped_ptr<mojo::Binding<mojo::WindowManagerInternalClient>>
      wm_internal_client_binding_;
  mojo::InterfaceRequest<mojo::ViewManagerClient> wm_internal_client_request_;
  mojo::WindowManagerInternalPtr wm_internal_;
  scoped_ptr<DisplayManager> display_manager_;
};

}  // namespace view_manager
