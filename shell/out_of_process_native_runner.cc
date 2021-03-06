// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/out_of_process_native_runner.h"

#include "base/bind.h"
#include "base/callback_helpers.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "shell/child_controller.mojom.h"
#include "shell/child_process_host.h"
#include "shell/in_process_native_runner.h"

namespace shell {

OutOfProcessNativeRunner::OutOfProcessNativeRunner(Context* context)
    : context_(context) {
}

OutOfProcessNativeRunner::~OutOfProcessNativeRunner() {
  if (child_process_host_) {
    // TODO(vtl): Race condition: If |ChildProcessHost::DidStart()| hasn't been
    // called yet, we shouldn't call |Join()| here. (Until |DidStart()|, we may
    // not have a child process to wait on.) Probably we should fix |Join()|.
    child_process_host_->Join();
  }
}

void OutOfProcessNativeRunner::Start(
    const base::FilePath& app_path,
    mojo::InterfaceRequest<mojo::Application> application_request,
    const base::Closure& app_completed_callback) {
  app_path_ = app_path;

  DCHECK(app_completed_callback_.is_null());
  app_completed_callback_ = app_completed_callback;

  child_process_host_.reset(new ChildProcessHost(context_));
  child_process_host_->Start();

  // TODO(vtl): |app_path.AsUTF8Unsafe()| is unsafe.
  child_process_host_->StartApp(
      app_path.AsUTF8Unsafe(), application_request.Pass(),
      base::Bind(&OutOfProcessNativeRunner::AppCompleted,
                 base::Unretained(this)));
}

void OutOfProcessNativeRunner::AppCompleted(int32_t result) {
  DVLOG(2) << "OutOfProcessNativeRunner::AppCompleted(" << result << ")";

  if (child_process_host_) {
    child_process_host_->Join();
    child_process_host_.reset();
  }
  // This object may be deleted by this callback.
  base::Closure app_completed_callback = app_completed_callback_;
  app_completed_callback_.Reset();
  app_completed_callback.Run();
}

scoped_ptr<NativeRunner> OutOfProcessNativeRunnerFactory::Create(
    const Options& options) {
  if (options.force_in_process)
    return make_scoped_ptr(new InProcessNativeRunner(context_));

  return make_scoped_ptr(new OutOfProcessNativeRunner(context_));
}

}  // namespace shell
