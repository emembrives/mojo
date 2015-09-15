// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "examples/bank_app/bank.mojom.h"
#include "mojo/common/binding_set.h"
#include "mojo/public/c/system/main.h"
#include "mojo/public/cpp/application/application_connection.h"
#include "mojo/public/cpp/application/application_delegate.h"
#include "mojo/public/cpp/application/application_impl.h"
#include "mojo/public/cpp/application/application_runner.h"
#include "mojo/public/cpp/application/interface_factory.h"
#include "mojo/public/cpp/bindings/strong_binding.h"
#include "mojo/public/cpp/utility/run_loop.h"
#include "mojo/services/vanadium/security/public/interfaces/principal.mojom.h"

namespace examples {

using vanadium::PrincipalServicePtr;

class BankImpl : public Bank {
 public:
  BankImpl() : balance_(0) {}
  void Deposit(int32_t usd) override { balance_ += usd; }
  void Withdraw(int32_t usd) override { balance_ -= usd; }
  void GetBalance(const GetBalanceCallback& callback) override {
    callback.Run(balance_);
  }
 private:
  int32_t balance_;
};

class BankUser {
 public:
  explicit BankUser(std::string* user) : user_(user) { }
  void Run(const vanadium::BlessingPtr& b) const {
    user_->clear();
    if (b && b->chain.size() > 0) {
      user_->append(b->chain[0]->extension);
      for (size_t i = 1; i < b->chain.size(); i++) {
        user_->append(vanadium::ChainSeparator);
        user_->append(b->chain[i]->extension);
      }
    }
  }
 private:
  std::string *user_;
};

class BankApp : public mojo::ApplicationDelegate,
                public mojo::InterfaceFactory<Bank> {
 public:
  BankApp() {
  }
  void Initialize(mojo::ApplicationImpl* app) override {
    app->ConnectToService("mojo:principal_service", &login_service_);
  }

  // From ApplicationDelegate
  bool ConfigureIncomingConnection(
      mojo::ApplicationConnection* connection) override {
    std::string url = connection->GetRemoteApplicationURL();
    if (url.length() > 0) {
      vanadium::AppInstanceNamePtr app(vanadium::AppInstanceName::New());
      app->url = url;
      std::string user;
      login_service_->GetUserBlessing(app.Pass(), BankUser(&user));
      // Check and see whether we got a valid user blessing.
      if (!login_service_.WaitForIncomingResponse()) {
        MOJO_LOG(INFO) << "Failed to get a valid user blessing";
        return false;
      }
      // Record user access to the bank and reject customers that
      // don't have a user identity.
      if (user.empty()) {
        MOJO_LOG(INFO) << "Rejecting customer without a user identity";
        return false;
      }
      MOJO_LOG(INFO) << "Customer " << user << " accessing bank";
    }
    connection->AddService(this);
    return true;
  }

  void Create(mojo::ApplicationConnection* connection,
              mojo::InterfaceRequest<Bank> request) override {
    bindings_.AddBinding(&bank_impl_, request.Pass());
  }

 private:
  BankImpl bank_impl_;
  mojo::BindingSet<Bank> bindings_;
  vanadium::PrincipalServicePtr login_service_;
};

}  // namespace examples

MojoResult MojoMain(MojoHandle application_request) {
  mojo::ApplicationRunner runner(new examples::BankApp());
  return runner.Run(application_request);
}
