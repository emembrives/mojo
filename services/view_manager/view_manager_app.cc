// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/view_manager/view_manager_app.h"

#include "mojo/application/application_runner_chromium.h"
#include "mojo/common/tracing_impl.h"
#include "mojo/public/c/system/main.h"
#include "mojo/public/cpp/application/application_connection.h"
#include "mojo/public/cpp/application/application_impl.h"
#include "services/view_manager/client_connection.h"
#include "services/view_manager/connection_manager.h"
#include "services/view_manager/display_manager.h"
#include "services/view_manager/view_manager_service_impl.h"

using mojo::ApplicationConnection;
using mojo::ApplicationImpl;
using mojo::InterfaceRequest;
using mojo::ViewManagerService;
using mojo::WindowManagerInternalClient;

namespace view_manager {

ViewManagerApp::ViewManagerApp() : app_impl_(nullptr) {}

ViewManagerApp::~ViewManagerApp() {}

void ViewManagerApp::Initialize(ApplicationImpl* app) {
  app_impl_ = app;
  tracing_.Initialize(app);
  connection_manager_.reset(new ConnectionManager(this));
}

bool ViewManagerApp::ConfigureIncomingConnection(
    ApplicationConnection* connection) {
  connection_manager_.reset(
      new ConnectionManager(this, display_manager.Pass(), wm_internal_.get()));
  return true;
}

void ViewManagerApp::OnLostConnectionToWindowManager() {
  ApplicationImpl::Terminate();
}

ClientConnection* ViewManagerApp::CreateClientConnectionForEmbedAtView(
    ConnectionManager* connection_manager,
    mojo::InterfaceRequest<mojo::ViewManagerService> service_request,
    mojo::ConnectionSpecificId creator_id,
    const std::string& creator_url,
    const std::string& url,
    const ViewId& root_id) {
  mojo::ViewManagerClientPtr client;
  app_impl_->ConnectToService(url, &client);

  scoped_ptr<ViewManagerServiceImpl> service(new ViewManagerServiceImpl(
      connection_manager, creator_id, creator_url, url, root_id));
  return new DefaultClientConnection(service.Pass(), connection_manager,
                                     service_request.Pass(), client.Pass());
}

ClientConnection* ViewManagerApp::CreateClientConnectionForEmbedAtView(
    ConnectionManager* connection_manager,
    mojo::InterfaceRequest<mojo::ViewManagerService> service_request,
    mojo::ConnectionSpecificId creator_id,
    const std::string& creator_url,
    const ViewId& root_id,
    mojo::ViewManagerClientPtr view_manager_client) {
  scoped_ptr<ViewManagerServiceImpl> service(new ViewManagerServiceImpl(
      connection_manager, creator_id, creator_url, std::string(), root_id));
  return new DefaultClientConnection(service.Pass(), connection_manager,
                                     service_request.Pass(),
                                     view_manager_client.Pass());
}

}  // namespace view_manager
