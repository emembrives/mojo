// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_WINDOW_MANAGER_WINDOW_MANAGER_DELEGATE_H_
#define SERVICES_WINDOW_MANAGER_WINDOW_MANAGER_DELEGATE_H_

#include "base/memory/scoped_ptr.h"
#include "mojo/public/cpp/bindings/string.h"
#include "mojo/public/interfaces/application/service_provider.mojom.h"
#include "mojo/services/view_manager/public/cpp/view_manager_delegate.h"

namespace mojo {
class ApplicationConnection;
}

namespace window_manager {

class WindowManagerRoot;

class WindowManagerDelegate {
 public:
  // See WindowManager::Embed() for details.
  virtual void Embed(const mojo::String& url,
                     mojo::InterfaceRequest<mojo::ServiceProvider> services,
                     mojo::ServiceProviderPtr exposed_services) = 0;

 protected:
  virtual ~WindowManagerDelegate() {}
};

// Controls a single instance of WindowManagerRoot.
class WindowManagerController : public WindowManagerDelegate,
                                public mojo::ViewManagerDelegate {};

// Creates WindowManagerControllers.
// This class should be implemented by window managers and passed to the
// WindowManagerApp object. Each new WindowManagerController will be tied to a
// WindowManagerRoot, controlling one native window and all its Mojo-controlled
// sub-windows inside it.
class WindowManagerControllerFactory {
 public:
  virtual scoped_ptr<WindowManagerController> CreateWindowManagerController(
      mojo::ApplicationConnection* connection,
      WindowManagerRoot* wm_root_) = 0;
};

}  // namespace mojo

#endif  // SERVICES_WINDOW_MANAGER_WINDOW_MANAGER_DELEGATE_H_
