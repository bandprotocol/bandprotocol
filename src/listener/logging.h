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

#include ""

///
class LoggingListener : public BaseListener
{
private:
  /// "handle<MsgType>" functions are implemented as log function that basically
  /// logs all the incoming message to the stdout console.
#define BASE_PROCESS_MESSAGE(R, _, MSG)                                        \
  virtual void BAND_MACRO_HANDLE(MSG)(const BAND_MACRO_MSG(MSG) & msg) final   \
  {                                                                            \
    INFO(log, "{}", msg);                                                      \
  }

  BAND_MACRO_MESSAGE_FOR_EACH(BASE_PROCESS_MESSAGE)

#undef BASE_PROCESS_MESSAGE

private:
  static inline auto log = logger::get("logging");
}
