// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/kiosk_wm/kiosk_wm.h"

#include "services/kiosk_wm/kiosk_wm_controller.h"
#include "services/window_manager/window_manager_delegate.h"

namespace kiosk_wm {

KioskWM::KioskWM()
    : window_manager_app_(new window_manager::WindowManagerApp(this)) {}

void KioskWM::Initialize(mojo::ApplicationImpl* app) {
  window_manager_app_->Initialize(app);

  // Format: --args-for="app_url default_url"
  if (app->args().size() > 1)
    default_url_ = app->args()[1];
}

window_manager::WindowManagerController* KioskWM::CreateWindowManagerController(
    mojo::ApplicationConnection* connection,
    window_manager::WindowManagerRoot* wm_root) {
  return new KioskWMController(wm_root, default_url_);
}

bool KioskWM::ConfigureIncomingConnection(
    mojo::ApplicationConnection* connection) {
  window_manager_app_->ConfigureIncomingConnection(connection);
  return true;
}

bool KioskWM::ConfigureOutgoingConnection(
    mojo::ApplicationConnection* connection) {
  window_manager_app_->ConfigureOutgoingConnection(connection);
  return true;
}

}  // namespace kiosk_wm
