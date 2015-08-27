// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_KIOSK_WM_KIOSK_WM_H_
#define SERVICES_KIOSK_WM_KIOSK_WM_H_

#include "base/memory/scoped_ptr.h"
#include "mojo/public/cpp/application/application_connection.h"
#include "mojo/public/cpp/application/application_delegate.h"
#include "mojo/public/cpp/application/application_impl.h"
#include "services/window_manager/window_manager_app.h"
#include "services/window_manager/window_manager_delegate.h"

namespace kiosk_wm {

class KioskWMController;

class KioskWM : public mojo::ApplicationDelegate,
                public window_manager::WindowManagerControllerFactory {
 public:
  KioskWM();

  // Overridden from window_manager::WindowManagerControllerFactory
  window_manager::WindowManagerController* CreateWindowManagerController(
      mojo::ApplicationConnection* connection,
      window_manager::WindowManagerRoot* wm_root) override;

 private:
  // Overridden from mojo::ApplicationDelegate:
  void Initialize(mojo::ApplicationImpl* app) override;
  bool ConfigureIncomingConnection(
      mojo::ApplicationConnection* connection) override;

  scoped_ptr<window_manager::WindowManagerApp> window_manager_app_;
  std::string default_url_;

  DISALLOW_COPY_AND_ASSIGN(KioskWM);
};

}  // namespace kiosk_wm

#endif  // SERVICES_KIOSK_WM_KIOSK_WM_H_
