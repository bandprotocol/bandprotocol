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

#include <boost/scope_exit.hpp>
#include <enum/enum.h>

#include "inc/essential.h"
#include "store/storage.h"
#include "util/msg.h"

/// The mode in which this primary listener is running. Switching back and
/// forth while the validator is running.
ENUM(PrimaryMode, uint8_t, Check, Apply)

/// Primary listener to maintain the blockchain state necessary for running
/// a fullnode without providing the query interface. This listener is
/// responsible for validating transactions and updating the validator set.
class PrimaryListener
{
public:
  PrimaryListener(Storage& storage);

  /// Validate the given set of transaction information. Mutate the user's
  /// nonce appropriately. Mode must be specified before this is called.
  /// Throw exception if the validation fails.
  void validateTransaction(PrimaryMode mode,
                           const HeaderMsg& hdr,
                           gsl::span<const byte> data);

  /// Load the current state of this listener. May involve blocking remote
  /// database calls.
  void load();

  /// Initialize blockchain state according to the given genesis struct.
  void init(const GenesisMsg& genesis);

  /// Begin a new block. The listener may override this function to perform
  /// necessary transactional operations.
  void begin(const BlockMsg& blk);

  /// Commit the current block.
  void commit(const BlockMsg& blk);

  /// Set up storage environment and call into primary logic to handle the
  /// transaction. Also responsible for flushing/reseting storage cache.
  template <typename T>
  auto process(const BlockMsg& blk, const HeaderMsg& hdr, const T& msg)
  {
    BOOST_SCOPE_EXIT_TPL(&storage)
    {
      storage.reset();
    }
    BOOST_SCOPE_EXIT_END

    storage.switchToApply();
    auto result = handle(blk, hdr, msg);
    storage.flush();

    return result;
  }

private:
  /// Primary listener must implement logic to handle each of the messages and
  /// return blockchain response.
#define BASE_PROCESS_MESSAGE(R, _, MSG)                                        \
  BAND_MACRO_RESPONSE(MSG)                                                     \
  handle(const BlockMsg& blk, const HeaderMsg& hdr,                            \
         const BAND_MACRO_MSG(MSG) & msg);

  BAND_MACRO_MESSAGE_FOR_EACH(BASE_PROCESS_MESSAGE)

#undef BASE_PROCESS_MESSAGE

private:
  /// Reference to the key-value storage manager.
  Storage& storage;
};
