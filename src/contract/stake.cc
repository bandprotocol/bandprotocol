// // Licensed to the Apache Software Foundation (ASF) under one
// // or more contributor license agreements.  See the LICENSE file
// // distributed with this work for additional information
// // regarding copyright ownership.  The ASF licenses this file
// // to you under the Apache License, Version 2.0 (the
// // "License"); you may not use this file except in compliance
// // with the License.  You may obtain a copy of the License at
// //
// //   http://www.apache.org/licenses/LICENSE-2.0
// //
// // Unless required by applicable law or agreed to in writing,
// // software distributed under the License is distributed on an
// // "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// // KIND, either express or implied.  See the License for the
// // specific language governing permissions and limitations
// // under the License.

// #include "stake.h"

// #include <algorithm>

// #include "contract/token.h"
// #include "store/global.h"

// Stake::Stake(const Address& stake_id)
//     : Contract(stake_id, ContractID::Stake)
// {
// }

// void Stake::init(const Address& _base_token)
// {
//   base_token = _base_token;
//   party_nonce = 0;
//   receipt_nonce = 0;
// }

// uint256_t Stake::stake(uint256_t party_id, uint256_t value)
// {
//   receipt_nonce = +receipt_nonce + 1;

//   assert_con(m_parties[party_id].exist(), "Party doesn't exist.");

//   // Add stake and transfer token to contract
//   Party& party = m_parties[party_id];
//   Token& token = Global::get().m_ctx->get<Token>(+base_token);
//   uint256_t old_stake = +party.current_stake;
//   party.current_stake = +party.current_stake + value;
//   token.transfer(m_addr, value);

//   // Create receipt
//   Receipt& receipt = m_receipts[+receipt_nonce];
//   receipt.init(Global::get().block_time, get_sender(), party_id, value);
//   DEBUG(log, "Create new receipt id : {} owner: {}", +receipt_nonce,
//         get_sender());

//   // Update set
//   if (active_party_list.contains({old_stake, party_id})) {
//     active_party_list.erase({old_stake, party_id});
//     active_party_list.insert({+party.current_stake, party_id});
//   }
//   return +receipt_nonce;
// }

// void Stake::withdraw(uint256_t receipt_id)
// {
//   assert_con(m_receipts[receipt_id].exist(), "Receipt not found.");
//   assert_con(get_sender() == +m_receipts[receipt_id].owner,
//              "You aren't owner of this receipt.");

//   Receipt& receipt = m_receipts[receipt_id];
//   assert_con(m_parties[+receipt.party_id].exist(),
//              "Party in this receipt doesn't exist.");

//   // Send token back to stake owner.
//   Token& token = Global::get().m_ctx->get<Token>(+base_token);
//   set_sender();
//   token.transfer(+receipt.owner, +receipt.value);

//   // Decrease total stake
//   Party& party = m_parties[+receipt.party_id];
//   uint256_t old_stake = +party.current_stake;
//   party.current_stake = +party.current_stake - +receipt.value;

//   // Decrease last_checkpoint stake if this stake before last checkpoint
//   if (party.sum_reward.size() > 0 &&
//       +receipt.last_update_time <= party.sum_reward.back().first) {
//     party.last_checkpoint_stake = +party.last_checkpoint_stake -
//     +receipt.value;
//   }

//   // Update set
//   if (active_party_list.contains({old_stake, +receipt.party_id})) {
//     active_party_list.erase({old_stake, +receipt.party_id});
//     active_party_list.insert({+party.current_stake, +receipt.party_id});
//   }
//   // Delete receipt
//   m_receipts[receipt_id].erase();
// }

// uint256_t Stake::create_party(uint256_t value, uint256_t numerator,
//                               uint256_t denominator)
// {
//   party_nonce = +party_nonce + 1;

//   Party& party = m_parties[+party_nonce];
//   party.init(0, 0, get_sender(), numerator, denominator);

//   // Create map leader to party_id
//   leader_to_party[get_sender()] = +party_nonce;

//   // Create receipt for first stake in this party.
//   stake(+party_nonce, value);

//   // Update list.
//   active_party_list.insert({value, +party_nonce});

//   return +party_nonce;
// }

// void Stake::claim_reward(uint256_t receipt_id)
// {
//   assert_con(m_receipts[receipt_id].exist(), "Receipt not found.");
//   assert_con(get_sender() == +m_receipts[receipt_id].owner,
//              "You aren't owner of this receipt.");

//   Receipt& receipt = m_receipts[receipt_id];

//   assert_con(m_parties[+receipt.party_id].exist(), "Party doesn't exist.");

//   Party& party = m_parties[+receipt.party_id];

//   // No reward to claim
//   assert_con(party.sum_reward.size() > 0 &&
//                  +receipt.last_update_time < party.sum_reward.back().first,
//              "No reward to claim.");

//   std::pair<uint64_t, uint256_t> p = {+receipt.last_update_time, 0};
//   // auto it =
//   //     std::lower_bound(party.sum_reward.begin(), party.sum_reward.end(),
//   p);

//   auto idx = party.sum_reward.lower_bound_index(p);
//   if (idx == party.sum_reward.size())
//     throw Error("No reward to claim.");
//   auto last_price = party.sum_reward[idx];
//   // Calculate reward
//   uint256_t total_reward = party.sum_reward.back().second -
//   last_price.second; total_reward *= +receipt.value; total_reward /=
//   magnitude;

//   Token& token = Global::get().m_ctx->get<Token>(+base_token);
//   set_sender();
//   token.transfer(+receipt.owner, total_reward);

//   // Update lastest update time of receipt.
//   receipt.last_update_time = party.sum_reward.back().first;
// }

// void Stake::reactivate_party(uint256_t party_id)
// {
//   assert_con(m_parties[party_id].exist(), "Party doesn't exist.");
//   Party& party = m_parties[party_id];
//   assert_con(!(+party.is_active), "Party has still actived");
//   assert_con(get_sender() == +party.leader,
//              "Sender don't be a leader of this party.");

//   party.is_active = true;
//   active_party_list.insert({+party.current_stake, party_id});
// }

// void Stake::add_reward(const Address& party_leader, uint256_t value)
// {
//   // TODO: How token come to contract?
//   Token& token = Global::get().m_ctx->get<Token>(+base_token);
//   set_sender();
//   token.mint(value);

//   // find party_id from address
//   assert_con(leader_to_party[party_leader].exist(),
//              "This address isn't any party's leader.");

//   uint256_t party_id = +leader_to_party[party_leader];
//   // Send some token direct to leader
//   assert_con(m_parties[party_id].exist(), "Party doesn't exist.");
//   Party& party = m_parties[party_id];

//   // No stake before last reward all new reward go to leader
//   uint256_t leader_reward = 0;
//   uint256_t shared_reward = 0;
//   if (+party.last_checkpoint_stake == 0) {
//     leader_reward = value;
//     shared_reward = 0;
//   } else {
//     leader_reward = value * +party.numerator / +party.denominator;
//     shared_reward = value - +leader_reward;
//     shared_reward *= magnitude;
//     shared_reward /= +party.last_checkpoint_stake;
//   }

//   // transfer reward to leader
//   token.transfer(+party.leader, leader_reward);

//   // Calculate sum of reward
//   uint256_t sum = 0;
//   if (party.sum_reward.size() == 0)
//     sum = shared_reward;
//   else
//     sum = party.sum_reward.back().second + shared_reward;
//   party.sum_reward.push_back({Global::get().block_time, sum});
//   DEBUG(log, "Now reward : {} sum reward :{}", shared_reward, sum);
//   // Save current stake to checkpoint stake
//   party.last_checkpoint_stake = +party.current_stake;
// }

// std::vector<std::pair<Address, uint256_t>> Stake::topx(uint16_t value) const
// {
//   assert_con(value <= active_party_list.size(),
//              "Party size is less than value");
//   std::vector<std::pair<Address, uint256_t>> top_x;
//   auto it = active_party_list.last();
//   for (int i = 0; i < value; i++, --it) {
//     uint256_t party_id = (*it).second;
//     assert_con(m_parties[party_id].exist(), "Party doesn't exist.");
//     const Party& party = m_parties[party_id];
//     top_x.emplace_back(+party.leader, +party.current_stake);
//   }
//   return top_x;
// }

// void Stake::deactivate_party(uint256_t party_id)
// {
//   assert_con(m_parties[party_id].exist(), "Party doesn't exist.");
//   Party& party = m_parties[party_id];
//   assert_con(+party.is_active, "Party doesn't active now");

//   party.is_active = false;
//   assert_con(active_party_list.contains({+party.current_stake, party_id}),
//              "Not found this party in list.");
//   active_party_list.erase({+party.current_stake, party_id});
// }

// void Stake::destroy_party(uint256_t party_id)
// {
//   assert_con(m_parties[party_id].exist(), "Party doesn't exist.");
//   Party& party = m_parties[party_id];
//   Address leader = +party.leader;
//   if (active_party_list.contains({+party.current_stake, party_id}))
//     active_party_list.erase({+party.current_stake, party_id});
//   m_parties[party_id].erase();
//   leader_to_party[leader].erase();
// }
// void Stake::debug_create() const
// {
//   DEBUG(log, "Stake contract created at {}", m_addr);
// }

// void Stake::debug_save() const
// {
//   // DEBUG(log, "Total Parties: {}", m_parties.size());

//   // for (auto& [party_id, party] : m_parties) {
//   //   DEBUG(log, "  PartyID {}", party_id);
//   //   DEBUG(log, "    leader: {}", party.leader);
//   //   DEBUG(log, "    current_stake: {}", party.current_stake);
//   //   DEBUG(log, "    last_stake: {}", party.last_checkpoint_stake);
//   //   DEBUG(log, "    numerator: {}", party.numerator);
//   //   DEBUG(log, "    denominator: {}", party.denominator);
//   //   DEBUG(log, "    Reward History: {}", party.sum_reward.size());
//   //   for (auto& [ts, sum] : party.sum_reward) {
//   //     DEBUG(log, "      At {} get {}", ts, sum);
//   //   }
//   // }
//   // DEBUG(log, "------------------------------------------------------");

//   // DEBUG(log, "Total Receipts: {}", m_receipts.size());
//   // for (auto& [receipt_id, receipt] : m_receipts) {
//   //   DEBUG(log, "  ReceiptID {}", receipt_id);
//   //   DEBUG(log, "    owner: {}", receipt.owner);
//   //   DEBUG(log, "    party_id: {}", receipt.party_id);
//   //   DEBUG(log, "    value: {}", receipt.value);
//   //   DEBUG(log, "    last_update_time: {}", receipt.last_update_time);
//   // }
//   // DEBUG(log, "======================================================");
// }
