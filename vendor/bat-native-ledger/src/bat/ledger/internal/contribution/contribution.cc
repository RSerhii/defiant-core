/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <stdint.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <utility>
#include <vector>

#include "base/time/time.h"
#include "bat/ledger/internal/common/bind_util.h"
#include "bat/ledger/internal/contribution/contribution.h"
#include "bat/ledger/internal/contribution/contribution_util.h"
#include "bat/ledger/internal/contribution/phase_one.h"
#include "bat/ledger/internal/contribution/phase_two.h"
#include "bat/ledger/internal/contribution/unverified.h"
#include "bat/ledger/internal/uphold/uphold.h"
#include "bat/ledger/internal/uphold/uphold_util.h"
#include "bat/ledger/internal/wallet/balance.h"
#include "bat/ledger/internal/ledger_impl.h"
#include "brave_base/random.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

namespace braveledger_contribution {

Contribution::Contribution(bat_ledger::LedgerImpl* ledger) :
    ledger_(ledger),
    phase_one_(std::make_unique<PhaseOne>(ledger, this)),
    phase_two_(std::make_unique<PhaseTwo>(ledger, this)),
    unverified_(std::make_unique<Unverified>(ledger, this)),
    uphold_(std::make_unique<braveledger_uphold::Uphold>(ledger)),
    last_reconcile_timer_id_(0u),
    queue_timer_id_(0u) {
}

Contribution::~Contribution() {
}

void Contribution::Initialize() {
  phase_two_->Initialize();
  uphold_->Initialize();

  // Resume in progress contributions
  braveledger_bat_helper::CurrentReconciles currentReconciles =
      ledger_->GetCurrentReconciles();

  for (const auto& value : currentReconciles) {
    braveledger_bat_helper::CURRENT_RECONCILE reconcile = value.second;

    if (reconcile.retry_step_ == ledger::ContributionRetry::STEP_FINAL) {
      ledger_->RemoveReconcileById(reconcile.viewingId_);
    } else {
      DoRetry(reconcile.viewingId_);
    }
  }

  // Process contribution queue
  CheckContributionQueue();
}

void Contribution::CheckContributionQueue() {
  const auto start_timer_in = ledger::is_testing
      ? 1
      : brave_base::random::Geometric(15);

  SetTimer(&queue_timer_id_, start_timer_in);
}

void Contribution::ProcessContributionQueue() {
  const auto callback = std::bind(&Contribution::OnProcessContributionQueue,
      this,
      _1);
  ledger_->GetFirstContributionQueue(callback);
}

void Contribution::OnProcessContributionQueue(
    ledger::ContributionQueuePtr info) {
  if (!info) {
    return;
  }

  InitReconcile(std::move(info));
  CheckContributionQueue();
}

void Contribution::HasSufficientBalance(
    ledger::HasSufficientBalanceToReconcileCallback callback) {
  ledger_->FetchBalance(
      std::bind(&Contribution::OnSufficientBalanceWallet,
                this,
                _1,
                _2,
                callback));
}

void Contribution::OnSufficientBalanceWallet(
    const ledger::Result result,
    ledger::BalancePtr properties,
    ledger::HasSufficientBalanceToReconcileCallback callback) {
  if (result != ledger::Result::LEDGER_OK || !properties) {
    return;
  }

  auto tips_callback =
      std::bind(&Contribution::OnHasSufficientBalance,
          this,
          _1,
          _2,
          properties->total,
          callback);

  ledger_->GetRecurringTips(tips_callback);
}

void Contribution::OnHasSufficientBalance(
    const ledger::PublisherInfoList& publisher_list,
    const uint32_t record,
    const double balance,
    ledger::HasSufficientBalanceToReconcileCallback callback) {
  if (publisher_list.empty()) {
    callback(true);
    return;
  }

  const auto total = GetTotalFromRecurringVerified(publisher_list);
  callback(balance >= total);
}

// static
double Contribution::GetTotalFromRecurringVerified(
    const ledger::PublisherInfoList& publisher_list) {
  double total_recurring_amount = 0.0;
  for (const auto& publisher : publisher_list) {
    if (publisher->id.empty()) {
      continue;
    }

    if (publisher->status == ledger::PublisherStatus::VERIFIED) {
      total_recurring_amount += publisher->weight;
    }
  }
  return total_recurring_amount;
}

ledger::PublisherInfoList Contribution::GetVerifiedListRecurring(
    const ledger::PublisherInfoList& list) {
  ledger::PublisherInfoList verified;
  ledger::PendingContributionList non_verified;

  for (const auto& publisher : list) {
    if (publisher->id.empty() || publisher->weight == 0.0) {
      continue;
    }

    if (publisher->status != ledger::PublisherStatus::NOT_VERIFIED) {
      verified.push_back(publisher->Clone());
    } else {
      auto contribution = ledger::PendingContribution::New();
      contribution->amount = publisher->weight;
      contribution->publisher_key = publisher->id;
      contribution->viewing_id = "";
      contribution->type = ledger::RewardsType::RECURRING_TIP;

      non_verified.push_back(std::move(contribution));
    }
  }

  if (non_verified.size() > 0) {
    ledger_->SaveUnverifiedContribution(
      std::move(non_verified),
      [](const ledger::Result _){});
  }

  return verified;
}

void Contribution::StartRecurringTips(ledger::ResultCallback callback) {
  ledger_->GetRecurringTips(
      std::bind(&Contribution::PrepareRecurringList,
                this,
                _1,
                _2,
                callback));
}

void Contribution::PrepareRecurringList(
    ledger::PublisherInfoList list,
    uint32_t next_record,
    ledger::ResultCallback callback) {
  auto verified_list = GetVerifiedListRecurring(list);

  for (const auto &item : verified_list) {
    ledger::ContributionQueuePublisherList queue_list;
    auto publisher = ledger::ContributionQueuePublisher::New();
    publisher->publisher_key = item->id;
    publisher->amount_percent = 100.0;
    queue_list.push_back(std::move(publisher));

    auto queue = ledger::ContributionQueue::New();
    queue->type = ledger::RewardsType::RECURRING_TIP;
    queue->amount = item->weight;
    queue->partial = false;
    queue->publishers = std::move(queue_list);

    ledger_->InsertOrUpdateContributionQueue(
        std::move(queue),
        [](const ledger::Result _){});
  }
  CheckContributionQueue();
  callback(ledger::Result::LEDGER_OK);
}

void Contribution::ResetReconcileStamp() {
  ledger_->ResetReconcileStamp();
  SetReconcileTimer();
}

void Contribution::StartMonthlyContribution() {
  BLOG(ledger_, ledger::LogLevel::LOG_INFO) << "Staring monthly contribution";
  if (!ledger_->GetRewardsMainEnabled()) {
    ResetReconcileStamp();
    return;
  }

  auto callback = std::bind(&Contribution::OnStartRecurringTips,
      this,
      _1);

  StartRecurringTips(callback);
}

void Contribution::OnStartRecurringTips(const ledger::Result result) {
  StartAutoContribute(ledger_->GetReconcileStamp());
  ResetReconcileStamp();
}

bool Contribution::ShouldStartAutoContribute() {
  if (!ledger_->GetRewardsMainEnabled()) {
    return false;
  }

  return ledger_->GetAutoContribute();
}

void Contribution::StartAutoContribute(uint64_t reconcile_stamp) {
  auto filter = ledger_->CreateActivityFilter(
      "",
      ledger::ExcludeFilter::FILTER_ALL_EXCEPT_EXCLUDED,
      true,
      reconcile_stamp,
      false,
      ledger_->GetPublisherMinVisits());

  ledger_->GetActivityInfoList(
      0,
      0,
      std::move(filter),
      std::bind(&Contribution::PrepareACList,
                this,
                _1,
                _2));
}

void Contribution::PrepareACList(
    ledger::PublisherInfoList list,
    uint32_t next_record) {
  ledger::PublisherInfoList normalized_list;

  ledger_->NormalizeContributeWinners(&normalized_list, &list, 0);

  if (normalized_list.empty()) {
    return;
  }

  ledger::ContributionQueuePublisherList queue_list;
  for (const auto &item : normalized_list) {
    if (item->percent == 0) {
      continue;
    }

    auto publisher = ledger::ContributionQueuePublisher::New();
    publisher->publisher_key = item->id;
    publisher->amount_percent =  item->weight;
    queue_list.push_back(std::move(publisher));
  }

  auto queue = ledger::ContributionQueue::New();
  queue->type = ledger::RewardsType::AUTO_CONTRIBUTE;
  queue->amount = ledger_->GetContributionAmount();
  queue->partial = true;
  queue->publishers = std::move(queue_list);
  ledger_->InsertOrUpdateContributionQueue(
      std::move(queue),
      [](const ledger::Result _){});
  CheckContributionQueue();
}

void Contribution::OnBalanceForReconcile(
    const std::string& contribution_queue,
    const ledger::Result result,
    ledger::BalancePtr info) {
  auto const contribution =
      braveledger_bind_util::FromStringToContributionQueue(contribution_queue);
  if (result != ledger::Result::LEDGER_OK || !info) {
    BLOG(ledger_, ledger::LogLevel::LOG_ERROR) <<
         "We couldn't get balance from the server.";
    phase_one_->Complete(ledger::Result::LEDGER_ERROR,
                         "",
                         contribution->type);
    return;
  }

  ProcessReconcile(contribution->Clone(), std::move(info));
}


void Contribution::InitReconcile(ledger::ContributionQueuePtr info) {
  const auto info_converted =
      braveledger_bind_util::FromContributionQueueToString(std::move(info));
  ledger_->FetchBalance(
      std::bind(&Contribution::OnBalanceForReconcile,
                this,
                info_converted,
                _1,
                _2));
}

void Contribution::OnTimer(uint32_t timer_id) {
  phase_two_->OnTimer(timer_id);
  unverified_->OnTimer(timer_id);
  uphold_->OnTimer(timer_id);

  if (timer_id == last_reconcile_timer_id_) {
    last_reconcile_timer_id_ = 0;
    StartMonthlyContribution();
    return;
  }

  if (timer_id == queue_timer_id_) {
    ProcessContributionQueue();
  }

  for (std::pair<std::string, uint32_t> const& value : retry_timers_) {
    if (value.second == timer_id) {
      std::string viewing_id = value.first;
      DoRetry(viewing_id);
      retry_timers_[viewing_id] = 0u;
    }
  }
}

void Contribution::SetReconcileTimer() {
  if (last_reconcile_timer_id_ != 0) {
    return;
  }

  uint64_t now = std::time(nullptr);
  uint64_t next_reconcile_stamp = ledger_->GetReconcileStamp();

  uint64_t time_to_next_reconcile =
      (next_reconcile_stamp == 0 || next_reconcile_stamp < now) ?
        0 : next_reconcile_stamp - now;

  SetTimer(&last_reconcile_timer_id_, time_to_next_reconcile);
}

void Contribution::SetTimer(uint32_t* timer_id, uint64_t start_timer_in) {
  if (start_timer_in == 0) {
    start_timer_in = brave_base::random::Geometric(45);
  }

  BLOG(ledger_, ledger::LogLevel::LOG_INFO)
    << "Starts in "
    << start_timer_in;

  ledger_->SetTimer(start_timer_in, timer_id);
}

void Contribution::OnReconcileCompleteSuccess(
    const std::string& viewing_id,
    const ledger::RewardsType type,
    const std::string& probi,
    ledger::ACTIVITY_MONTH month,
    int year,
    uint32_t date) {
  if (type == ledger::RewardsType::AUTO_CONTRIBUTE) {
    ledger_->SetBalanceReportItem(
        month,
        year,
        GetReportTypeFromRewardsType(type),
        probi);
    ledger_->SaveContributionInfo(probi, month, year, date, "", type);
    return;
  }

  if (type == ledger::RewardsType::ONE_TIME_TIP ||
      type == ledger::RewardsType::RECURRING_TIP) {
    ledger_->SetBalanceReportItem(
        month,
        year,
        GetReportTypeFromRewardsType(type),
        probi);
    const auto reconcile = ledger_->GetReconcileById(viewing_id);
    const auto donations = reconcile.directions_;
    if (donations.size() > 0) {
      std::string publisher_key = donations[0].publisher_key_;
      ledger_->SaveContributionInfo(probi,
                                    month,
                                    year,
                                    date,
                                    publisher_key,
                                    type);
    }
    return;
  }
}

void Contribution::AddRetry(
    ledger::ContributionRetry step,
    const std::string& viewing_id,
    braveledger_bat_helper::CURRENT_RECONCILE reconcile) {
  BLOG(ledger_, ledger::LogLevel::LOG_WARNING)
      << "Re-trying contribution for step"
      << std::to_string(static_cast<int32_t>(step))
      << "for" << viewing_id;

  if (reconcile.viewingId_.empty()) {
    reconcile = ledger_->GetReconcileById(viewing_id);
  }

  // Don't retry one-time tip if in phase 1
  if (GetRetryPhase(step) == 1 &&
      reconcile.type_ == ledger::RewardsType::ONE_TIME_TIP) {
    phase_one_->Complete(ledger::Result::TIP_ERROR,
                         viewing_id,
                         reconcile.type_);
    return;
  }

  uint64_t start_timer_in = GetRetryTimer(step, viewing_id, &reconcile);
  bool success = ledger_->AddReconcileStep(viewing_id,
                                           reconcile.retry_step_,
                                           reconcile.retry_level_);
  if (!success || start_timer_in == 0) {
    phase_one_->Complete(ledger::Result::LEDGER_ERROR,
                         viewing_id,
                         reconcile.type_);
    return;
  }

  retry_timers_[viewing_id] = 0u;
  SetTimer(&retry_timers_[viewing_id], start_timer_in);
}

uint64_t Contribution::GetRetryTimer(
    ledger::ContributionRetry step,
    const std::string& viewing_id,
    braveledger_bat_helper::CURRENT_RECONCILE* reconcile) {
  ledger::ContributionRetry old_step = reconcile->retry_step_;

  int phase = GetRetryPhase(step);
  if (phase > GetRetryPhase(old_step)) {
    reconcile->retry_level_ = 0;
  } else {
    reconcile->retry_level_++;
  }

  reconcile->retry_step_ = step;

  if (phase == 1) {
    // TODO(nejczdovc) get size from the list
    if (reconcile->retry_level_ < 5) {
      if (ledger::short_retries) {
        return phase_one_debug_timers[reconcile->retry_level_];
      } else {
        return phase_one_timers[reconcile->retry_level_];
      }

    } else {
      return 0;
    }
  }

  if (phase == 2) {
    // TODO(nejczdovc) get size from the list
    if (reconcile->retry_level_ > 2) {
      if (ledger::short_retries) {
        return phase_two_debug_timers[2];
      } else {
        return phase_two_timers[2];
      }
    } else {
      if (ledger::short_retries) {
        return phase_two_debug_timers[reconcile->retry_level_];
      } else {
        return phase_two_timers[reconcile->retry_level_];
      }
    }
  }

  return 0;
}

int Contribution::GetRetryPhase(ledger::ContributionRetry step) {
  int phase = 0;

  switch (step) {
    case ledger::ContributionRetry::STEP_RECONCILE:
    case ledger::ContributionRetry::STEP_CURRENT:
    case ledger::ContributionRetry::STEP_PAYLOAD:
    case ledger::ContributionRetry::STEP_REGISTER:
    case ledger::ContributionRetry::STEP_VIEWING: {
      phase = 1;
      break;
    }
    case ledger::ContributionRetry::STEP_PREPARE:
    case ledger::ContributionRetry::STEP_VOTE:
    case ledger::ContributionRetry::STEP_PROOF:
    case ledger::ContributionRetry::STEP_WINNERS:
    case ledger::ContributionRetry::STEP_FINAL: {
      phase = 2;
      break;
    }
    case ledger::ContributionRetry::STEP_NO:
      break;
  }

  return phase;
}

void Contribution::DoRetry(const std::string& viewing_id) {
  auto reconcile = ledger_->GetReconcileById(viewing_id);

  switch (reconcile.retry_step_) {
    case ledger::ContributionRetry::STEP_RECONCILE: {
      phase_one_->Start(viewing_id);
      break;
    }
    case ledger::ContributionRetry::STEP_CURRENT: {
      phase_one_->CurrentReconcile(viewing_id);
      break;
    }
    case ledger::ContributionRetry::STEP_PAYLOAD: {
      phase_one_->ReconcilePayload(viewing_id);
      break;
    }
    case ledger::ContributionRetry::STEP_REGISTER: {
      phase_one_->RegisterViewing(viewing_id);
      break;
    }
    case ledger::ContributionRetry::STEP_VIEWING: {
      phase_one_->ViewingCredentials(viewing_id);
      break;
    }
    case ledger::ContributionRetry::STEP_PREPARE: {
      phase_two_->PrepareBallots();
      break;
    }
    case ledger::ContributionRetry::STEP_PROOF: {
      phase_two_->Proof();
      break;
    }
    case ledger::ContributionRetry::STEP_VOTE: {
      phase_two_->VoteBatch();
      break;
    }
    case ledger::ContributionRetry::STEP_WINNERS: {
      phase_two_->Start(viewing_id);
      break;
    }
    case ledger::ContributionRetry::STEP_FINAL:
    case ledger::ContributionRetry::STEP_NO:
      break;
  }
}

void Contribution::ContributeUnverifiedPublishers() {
  unverified_->Contribute();
}

void Contribution::StartPhaseTwo(const std::string& viewing_id) {
  phase_two_->Start(viewing_id);
}

void Contribution::DoDirectTip(
    const std::string& publisher_key,
    int amount,
    const std::string& currency,
    ledger::DoDirectTipCallback callback) {
  if (publisher_key.empty()) {
    BLOG(ledger_, ledger::LogLevel::LOG_ERROR) <<
      "Failed direct donation due to missing publisher id";
    callback(ledger::Result::NOT_FOUND);
    return;
  }

  const auto server_callback =
    std::bind(&Contribution::OnDoDirectTipServerPublisher,
              this,
              _1,
              publisher_key,
              amount,
              currency,
              callback);

  ledger_->GetServerPublisherInfo(publisher_key, server_callback);
}

void Contribution::SavePendingContribution(
    const std::string& publisher_key,
    double amount,
    const ledger::RewardsType type,
    ledger::SavePendingContributionCallback callback) {
  auto contribution = ledger::PendingContribution::New();
  contribution->publisher_key = publisher_key;
  contribution->amount = amount;
  contribution->type = type;

  ledger::PendingContributionList list;
  list.push_back(std::move(contribution));

  ledger_->SaveUnverifiedContribution(
      std::move(list),
      callback);
}

void Contribution::OnDoDirectTipServerPublisher(
    ledger::ServerPublisherInfoPtr server_info,
    const std::string& publisher_key,
    int amount,
    const std::string& currency,
    ledger::DoDirectTipCallback callback) {
  auto status = ledger::PublisherStatus::NOT_VERIFIED;
  if (server_info) {
    status =  server_info->status;
  }

  // Save to the pending list if not verified
  if (status == ledger::PublisherStatus::NOT_VERIFIED) {
    SavePendingContribution(
        publisher_key,
        static_cast<double>(amount),
        ledger::RewardsType::ONE_TIME_TIP,
        callback);
    return;
  }

  ledger::ContributionQueuePublisherList queue_list;
  auto publisher = ledger::ContributionQueuePublisher::New();
  publisher->publisher_key = publisher_key;
  publisher->amount_percent = 100.0;
  queue_list.push_back(std::move(publisher));

  auto queue = ledger::ContributionQueue::New();
  queue->type = ledger::RewardsType::ONE_TIME_TIP;
  queue->amount = amount;
  queue->partial = false;
  queue->publishers = std::move(queue_list);

  InitReconcile(std::move(queue));
  callback(ledger::Result::LEDGER_OK);
}

bool Contribution::HaveReconcileEnoughFunds(
    ledger::ContributionQueuePtr contribution,
    double* fee,
    const double balance) {
  if (contribution->type == ledger::RewardsType::AUTO_CONTRIBUTE) {
    if (balance == 0) {
      BLOG(ledger_, ledger::LogLevel::LOG_WARNING) <<
          "You do not have enough funds for auto contribution";
       phase_one_->Complete(ledger::Result::NOT_ENOUGH_FUNDS,
                            "",
                            contribution->type);
      return false;
    }

    if (contribution->amount > balance) {
      contribution->amount = balance;
    }

    *fee = contribution->amount;
    return true;
  }

  if (contribution->amount > balance) {
    BLOG(ledger_, ledger::LogLevel::LOG_WARNING) <<
      "You do not have enough funds to do a contribution";
      phase_one_->Complete(ledger::Result::NOT_ENOUGH_FUNDS,
                           "",
                           contribution->type);
    return false;
  }

  *fee = contribution->amount;
  return true;
}

void Contribution::DeleteContributionQueue(
    ledger::ContributionQueuePtr contribution) {
  if (!contribution || contribution->id == 0) {
    return;
  }

  ledger_->DeleteContributionQueue(
      contribution->id,
      [](const ledger::Result _){});
}

void Contribution::ProcessReconcile(
    ledger::ContributionQueuePtr contribution,
    ledger::BalancePtr info) {
  double fee = .0;
  const auto have_enough_balance = HaveReconcileEnoughFunds(
      contribution->Clone(),
      &fee,
      info->total);

  if (!have_enough_balance) {
    DeleteContributionQueue(contribution->Clone());
    return;
  }

  if (contribution->amount == 0 || contribution->publishers.empty()) {
    DeleteContributionQueue(contribution->Clone());
    return;
  }

  const auto directions = FromContributionQueuePublishersToReconcileDirections(
      std::move(contribution->publishers));

  auto anon_reconcile = braveledger_bat_helper::CURRENT_RECONCILE();
  anon_reconcile.viewingId_ = ledger_->GenerateGUID();
  anon_reconcile.fee_ = fee;
  // TODO(tmancey): Temp conversion as we are moving everything into DB
  // so this can be remove at that point
  anon_reconcile.directions_ = directions;
  anon_reconcile.type_ = contribution->type;

  if (ledger_->ReconcileExists(anon_reconcile.viewingId_)) {
    BLOG(ledger_, ledger::LogLevel::LOG_ERROR)
      << "Unable to reconcile with the same viewing id: "
      << anon_reconcile.viewingId_;
    DeleteContributionQueue(contribution->Clone());
    return;
  }

  // Check if we can process contribution with anon wallet only
  const double anon_balance = braveledger_wallet::Balance::GetPerWalletBalance(
      ledger::kWalletAnonymous,
      info->wallets);
  if (anon_balance >= fee) {
    ledger_->AddReconcile(anon_reconcile.viewingId_, anon_reconcile);
    DeleteContributionQueue(contribution->Clone());
    phase_one_->Start(anon_reconcile.viewingId_);
    return;
  }

  // We need to first use all anon balance and what is left should
  // go through connected wallet
  braveledger_bat_helper::Directions wallet_directions;
  if (anon_balance > 0) {
    fee = fee - anon_balance;
    anon_reconcile.fee_ = anon_balance;

    if (contribution->type == ledger::RewardsType::RECURRING_TIP ||
        contribution->type == ledger::RewardsType::ONE_TIME_TIP) {
      braveledger_bat_helper::Directions anon_directions;
      AdjustTipsAmounts(directions,
                        &wallet_directions,
                        &anon_directions,
                        anon_balance);
      anon_reconcile.directions_ = anon_directions;
    }

    ledger_->AddReconcile(anon_reconcile.viewingId_, anon_reconcile);
    phase_one_->Start(anon_reconcile.viewingId_);
  } else {
    wallet_directions = directions;
  }

  auto wallet_reconcile = braveledger_bat_helper::CURRENT_RECONCILE();
  wallet_reconcile.viewingId_ = ledger_->GenerateGUID();
  wallet_reconcile.fee_ = fee;
  wallet_reconcile.directions_ = wallet_directions;
  wallet_reconcile.type_ = contribution->type;
  ledger_->AddReconcile(wallet_reconcile.viewingId_, wallet_reconcile);

  auto tokens_callback = std::bind(&Contribution::OnExternalWallets,
                                   this,
                                   wallet_reconcile.viewingId_,
                                   info->wallets,
                                   _1);

  // Check if we have token
  ledger_->GetExternalWallets(tokens_callback);
  DeleteContributionQueue(contribution->Clone());
}

void Contribution::AdjustTipsAmounts(
    braveledger_bat_helper::Directions directions,
    braveledger_bat_helper::Directions* wallet_directions,
    braveledger_bat_helper::Directions* anon_directions,
    double reduce_fee_for) {
  for (auto item : directions) {
    if (reduce_fee_for == 0) {
      wallet_directions->push_back(item);
      continue;
    }

    if (item.amount_percent_ <= reduce_fee_for) {
      anon_directions->push_back(item);
      reduce_fee_for -= item.amount_percent_;
      continue;
    }

    if (item.amount_percent_ > reduce_fee_for) {
      // anon wallet
      const auto original_weight = item.amount_percent_;
      item.amount_percent_ = reduce_fee_for;
      anon_directions->push_back(item);

      // rest to normal wallet
      item.amount_percent_ = original_weight - reduce_fee_for;
      wallet_directions->push_back(item);

      reduce_fee_for = 0;
    }
  }
}

void Contribution::OnExternalWallets(
    const std::string& viewing_id,
    base::flat_map<std::string, double> wallet_balances,
    std::map<std::string, ledger::ExternalWalletPtr> wallets) {
  // In this phase we only support one wallet
  // so we will just always pick uphold.
  // In the future we will allow user to pick which wallet to use via UI
  // and then we will extend this function
  const double uphold_balance =
      braveledger_wallet::Balance::GetPerWalletBalance(ledger::kWalletUphold,
                                                       wallet_balances);
  const auto reconcile = ledger_->GetReconcileById(viewing_id);

  if (wallets.size() == 0 || uphold_balance < reconcile.fee_) {
    phase_one_->Complete(ledger::Result::NOT_ENOUGH_FUNDS,
                         viewing_id,
                         reconcile.type_);
    return;
  }

  ledger::ExternalWalletPtr wallet =
      braveledger_uphold::GetWallet(std::move(wallets));
  if (!wallet || wallet->token.empty()) {
    phase_one_->Complete(ledger::Result::LEDGER_ERROR,
                         viewing_id,
                         reconcile.type_);
    return;
  }

  if (reconcile.type_ == ledger::RewardsType::AUTO_CONTRIBUTE) {
    auto callback = std::bind(&Contribution::OnUpholdAC,
                              this,
                              _1,
                              _2,
                              viewing_id);
    uphold_->TransferFunds(reconcile.fee_,
                           ledger_->GetCardIdAddress(),
                           std::move(wallet),
                           callback);
    return;
  }

  for (const auto& item : reconcile.directions_) {
    const auto amount = (item.amount_percent_ * reconcile.fee_) / 100;
    auto callback =
        std::bind(&Contribution::OnExternalWalletServerPublisherInfo,
          this,
          _1,
          viewing_id,
          static_cast<int>(amount),
          *wallet);

    ledger_->GetServerPublisherInfo(item.publisher_key_, callback);
  }
}

void Contribution::OnExternalWalletServerPublisherInfo(
    ledger::ServerPublisherInfoPtr info,
    const std::string& viewing_id,
    int amount,
    const ledger::ExternalWallet& wallet) {
  const auto reconcile = ledger_->GetReconcileById(viewing_id);
  if (!info) {
    const auto probi =
        braveledger_uphold::ConvertToProbi(std::to_string(amount));
    ledger_->OnReconcileComplete(
        ledger::Result::LEDGER_ERROR,
        viewing_id,
        probi,
        reconcile.type_);

    if (!viewing_id.empty()) {
      ledger_->RemoveReconcileById(viewing_id);
    }
    return;
  }

  if (info->status != ledger::PublisherStatus::VERIFIED) {
    SavePendingContribution(
        info->publisher_key,
        static_cast<double>(amount),
        static_cast<ledger::RewardsType>(reconcile.type_),
        [](const ledger::Result _){});
    return;
  }

  uphold_->StartContribution(
      viewing_id,
      info->address,
      static_cast<double>(amount),
      ledger::ExternalWallet::New(wallet));
}

void Contribution::OnUpholdAC(ledger::Result result,
                              bool created,
                              const std::string& viewing_id) {
  if (result != ledger::Result::LEDGER_OK) {
    // TODO(nejczdovc): add retries
    return;
  }

  phase_one_->Start(viewing_id);
}

}  // namespace braveledger_contribution
