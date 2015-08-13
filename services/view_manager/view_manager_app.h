// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_VIEW_MANAGER_VIEW_MANAGER_APP_H_
#define SERVICES_VIEW_MANAGER_VIEW_MANAGER_APP_H_

#include "base/memory/scoped_ptr.h"
#include "mojo/common/tracing_impl.h"
#include "mojo/public/cpp/application/application_delegate.h"
#include "mojo/public/cpp/application/interface_factory.h"
#include "mojo/public/cpp/bindings/binding.h"
#include "mojo/services/view_manager/public/interfaces/view_manager.mojom.h"
#include "mojo/services/window_manager/public/interfaces/window_manager_internal.mojom.h"
#include "services/view_manager/connection_manager_delegate.h"

namespace mojo {
class ApplicationImpl;
}

namespace view_manager {

class ConnectionManager;

class ViewManagerApp
    : public mojo::ApplicationDelegate,
      public ConnectionManagerDelegate,
      public mojo::InterfaceFactory<mojo::ViewManagerService>,
      public mojo::InterfaceFactory<mojo::WindowManagerInternalClient> {
 public:
  ViewManagerApp();
  ~ViewManagerApp() override;

 private:
  // ApplicationDelegate:
  void Initialize(mojo::ApplicationImpl* app) override;
  bool ConfigureIncomingConnection(
      mojo::ApplicationConnection* connection) override;

  // ConnectionManagerDelegate:
  void OnLostConnectionToWindowManager() override;
  ClientConnection* CreateClientConnectionForEmbedAtView(
      ConnectionManager* connection_manager,
      mojo::InterfaceRequest<mojo::ViewManagerService> service_request,
      mojo::ConnectionSpecificId creator_id,
      const std::string& creator_url,
      const std::string& url,
      const ViewId& root_id) override;
  ClientConnection* CreateClientConnectionForEmbedAtView(
      ConnectionManager* connection_manager,
      mojo::InterfaceRequest<mojo::ViewManagerService> service_request,
      mojo::ConnectionSpecificId creator_id,
      const std::string& creator_url,
      const ViewId& root_id,
      mojo::ViewManagerClientPtr view_manager_client) override;

  mojo::ApplicationImpl* app_impl_;
  scoped_ptr<ConnectionManager> connection_manager_;
  mojo::TracingImpl tracing_;

  DISALLOW_COPY_AND_ASSIGN(ViewManagerApp);
};

}  // namespace view_manager

#endif  // SERVICES_VIEW_MANAGER_VIEW_MANAGER_APP_H_
