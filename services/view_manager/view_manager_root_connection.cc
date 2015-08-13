// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/view_manager/view_manager_root_connection.h"

namespace view_manager {

ViewManagerRootConnection::ViewManagerRootConnection(
    mojo::ApplicationImpl* application_impl,
    mojo::ApplicationConnection* connection)
    : app_impl_(application_impl) {
  // |connection| originates from the WindowManager. Let it connect directly
  // to the ViewManager and WindowManagerInternalClient.
  connection->AddService<ViewManagerService>(this);
  connection->AddService<WindowManagerInternalClient>(this);
  connection->ConnectToService(&wm_internal_);

  wm_internal_.set_connection_error_handler(
      base::Bind(&ViewManagerRootConnection::OnLostConnectionToWindowManager,
                 base::Unretained(this)));

  display_manager_.reset(new DefaultDisplayManager(
      app_impl_, connection,
      base::Bind(&ViewManagerRootConnection::OnLostConnectionToWindowManager,
                 base::Unretained(this))));
}

void ViewManagerRootConnection::Create(
    ApplicationConnection* connection,
    InterfaceRequest<ViewManagerService> request) {
  scoped_ptr<ViewManagerServiceImpl> service(new ViewManagerServiceImpl(
      connection_manager_.get(), kInvalidConnectionId, std::string(),
      std::string("mojo:window_manager"), RootViewId()));
  mojo::ViewManagerClientPtr client;
  wm_internal_client_request_ = GetProxy(&client);
  scoped_ptr<ClientConnection> client_connection(
      new DefaultClientConnection(service.Pass(), connection_manager_.get(),
                                  request.Pass(), client.Pass()));
  connection_manager_->SetWindowManagerClientConnection(
      client_connection.Pass());
}

void ViewManagerRootConnection::Create(
    ApplicationConnection* connection,
    InterfaceRequest<WindowManagerInternalClient> request) {
  if (wm_internal_client_binding_.get()) {
    VLOG(1) << "WindowManagerInternalClient requested more than once.";
    return;
  }

  wm_internal_client_binding_.reset(
      new mojo::Binding<WindowManagerInternalClient>(connection_manager_.get(),
                                                     request.Pass()));
  wm_internal_client_binding_->set_connection_error_handler(
      &ApplicationImpl::Terminate);
  wm_internal_->SetViewManagerClient(
      wm_internal_client_request_.PassMessagePipe());
}

}  // namespace view_manager
