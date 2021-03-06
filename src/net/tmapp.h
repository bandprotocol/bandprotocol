// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the LICENSE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#pragma once

#include <vector>

#include "abci/abci.h"
#include "inc/essential.h"
#include "net/netapp.h"
#include "util/bytes.h"

class TendermintApplication : public NetApplication
{
public:
  /// What's the name of the running application?
  virtual std::string get_name() const = 0;

  /// What's the version of the running application?
  virtual std::string get_version() const = 0;

  /// What's the current application's Merkle hash?
  virtual std::string get_current_app_hash() const = 0;

  /// Initialize the blockchain based on the given app data.
  virtual void
  init(const std::vector<std::pair<VerifyKey, uint64_t>>& validators,
       const std::string& init_state) = 0;

  /// Query blockchain state.
  virtual std::string query(const std::string& path,
                            const std::string& data) = 0;

  /// Apply an incoming message. Throw if the message is not valid.
  virtual void check(const std::string& msg_raw) = 0;

  /// Apply an incoming message to the blockchain.
  virtual std::string apply(const std::string& msg_raw) = 0;

  virtual void begin_block(uint64_t block_time,
                           const Address& block_proposer) = 0;

  virtual std::vector<std::pair<VerifyKey, uint64_t>> end_block() = 0;

  virtual void commit_block() = 0;

protected:
  uint64_t last_block_height = 0;

  uint16_t number_validators;
  std::vector<std::pair<VerifyKey, uint64_t>> validators;

private:
  /// Return Tendermint version that this app supports.
  std::string get_tm_version()
  {
    return "0.25.0-d297f022";
  }

  void do_info(const RequestInfo&, ResponseInfo&);
  void do_init_chain(const RequestInitChain&);
  void do_query(const RequestQuery&, ResponseQuery&);
  void do_begin_block(const RequestBeginBlock&);
  void do_check_tx(const RequestCheckTx&, ResponseCheckTx&);
  void do_deliver_tx(const RequestDeliverTx&, ResponseDeliverTx&);
  void do_end_block(const RequestEndBlock&, ResponseEndBlock&);
  void do_commit(ResponseCommit&);

  /// Process incoming message and write to the outgoing buffer.
  bool process(Buffer& read_buffer, Buffer& write_buffer) final;

  static inline auto log = logger::get("tmapp");
};
