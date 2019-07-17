/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/services/bat_ledger/bat_ledger_impl.h"

#include <stdint.h>

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "base/containers/flat_map.h"
#include "brave/components/services/bat_ledger/bat_ledger_client_mojo_proxy.h"
#include "mojo/public/cpp/bindings/map.h"

using std::placeholders::_1;
using std::placeholders::_2;

namespace bat_ledger {

namespace {  // TODO(Nejc Zdovc): Move into a util class

ledger::PUBLISHER_EXCLUDE ToLedgerPublisherExclude(int32_t exclude) {
  return (ledger::PUBLISHER_EXCLUDE)exclude;
}

ledger::ACTIVITY_MONTH ToLedgerPublisherMonth(int32_t month) {
  return (ledger::ACTIVITY_MONTH)month;
}

ledger::ReportType ToLedgerReportType(int32_t type) {
  return (ledger::ReportType)type;
}

ledger::REWARDS_CATEGORY ToLedgerPublisherCategory(int32_t category) {
  return (ledger::REWARDS_CATEGORY)category;
}

}  // namespace

BatLedgerImpl::BatLedgerImpl(
    mojom::BatLedgerClientAssociatedPtrInfo client_info)
  : bat_ledger_client_mojo_proxy_(
      new BatLedgerClientMojoProxy(std::move(client_info))),
    ledger_(
      ledger::Ledger::CreateInstance(bat_ledger_client_mojo_proxy_.get())) {
}

BatLedgerImpl::~BatLedgerImpl() {
}

void BatLedgerImpl::Initialize() {
  ledger_->Initialize();
}

void BatLedgerImpl::CreateWallet() {
  ledger_->CreateWallet();
}

// static
void BatLedgerImpl::OnFetchWalletProperties(
    CallbackHolder<FetchWalletPropertiesCallback>* holder,
    ledger::Result result,
    ledger::WalletPropertiesPtr properties) {
  if (holder->is_valid())
    std::move(holder->get()).Run(result, std::move(properties));
  delete holder;
}

void BatLedgerImpl::FetchWalletProperties(
    FetchWalletPropertiesCallback callback) {
  // delete in OnFetchWalletProperties
  auto* holder = new CallbackHolder<FetchWalletPropertiesCallback>(
      AsWeakPtr(), std::move(callback));
  ledger_->FetchWalletProperties(
      std::bind(BatLedgerImpl::OnFetchWalletProperties, holder, _1, _2));
}

void BatLedgerImpl::GetAutoContributeProps(
    GetAutoContributePropsCallback callback) {
  ledger::AutoContributePropsPtr props = ledger_->GetAutoContributeProps();
  std::move(callback).Run(std::move(props));
}

void BatLedgerImpl::GetPublisherMinVisitTime(
    GetPublisherMinVisitTimeCallback callback) {
  std::move(callback).Run(ledger_->GetPublisherMinVisitTime());
}

void BatLedgerImpl::GetPublisherMinVisits(
    GetPublisherMinVisitsCallback callback) {
  std::move(callback).Run(ledger_->GetPublisherMinVisits());
}

void BatLedgerImpl::GetPublisherAllowNonVerified(
    GetPublisherAllowNonVerifiedCallback callback) {
  std::move(callback).Run(ledger_->GetPublisherAllowNonVerified());
}

void BatLedgerImpl::GetPublisherAllowVideos(
    GetPublisherAllowVideosCallback callback) {
  std::move(callback).Run(ledger_->GetPublisherAllowVideos());
}

void BatLedgerImpl::GetAutoContribute(
    GetAutoContributeCallback callback) {
  std::move(callback).Run(ledger_->GetAutoContribute());
}

void BatLedgerImpl::GetReconcileStamp(GetReconcileStampCallback callback) {
  std::move(callback).Run(ledger_->GetReconcileStamp());
}

void BatLedgerImpl::OnLoad(ledger::VisitDataPtr visit_data,
    uint64_t current_time) {
  ledger_->OnLoad(std::move(visit_data), current_time);
}

void BatLedgerImpl::OnUnload(uint32_t tab_id, uint64_t current_time) {
  ledger_->OnUnload(tab_id, current_time);
}

void BatLedgerImpl::OnShow(uint32_t tab_id, uint64_t current_time) {
  ledger_->OnShow(tab_id, current_time);
}

void BatLedgerImpl::OnHide(uint32_t tab_id, uint64_t current_time) {
  ledger_->OnHide(tab_id, current_time);
}

void BatLedgerImpl::OnForeground(uint32_t tab_id, uint64_t current_time) {
  ledger_->OnForeground(tab_id, current_time);
}

void BatLedgerImpl::OnBackground(uint32_t tab_id, uint64_t current_time) {
  ledger_->OnBackground(tab_id, current_time);
}

void BatLedgerImpl::OnPostData(const std::string& url,
    const std::string& first_party_url, const std::string& referrer,
    const std::string& post_data, ledger::VisitDataPtr visit_data) {
  ledger_->OnPostData(
      url, first_party_url, referrer, post_data, std::move(visit_data));
}

void BatLedgerImpl::OnXHRLoad(uint32_t tab_id, const std::string& url,
    const base::flat_map<std::string, std::string>& parts,
    const std::string& first_party_url, const std::string& referrer,
    ledger::VisitDataPtr visit_data) {
    ledger_->OnXHRLoad(tab_id, url, mojo::FlatMapToMap(parts),
        first_party_url, referrer, std::move(visit_data));
}

void BatLedgerImpl::SetPublisherExclude(const std::string& publisher_key,
    int32_t exclude) {
  ledger_->SetPublisherExclude(publisher_key,
      ToLedgerPublisherExclude(exclude));
}

void BatLedgerImpl::RestorePublishers() {
  ledger_->RestorePublishers();
}

void BatLedgerImpl::SetBalanceReportItem(int32_t month,
    int32_t year, int32_t type, const std::string& probi) {
  ledger_->SetBalanceReportItem(
      ToLedgerPublisherMonth(month), year, ToLedgerReportType(type), probi);
}

void BatLedgerImpl::OnReconcileCompleteSuccess(const std::string& viewing_id,
    int32_t category, const std::string& probi, int32_t month,
    int32_t year, uint32_t data) {
  ledger_->OnReconcileCompleteSuccess(viewing_id,
      ToLedgerPublisherCategory(category), probi,
      ToLedgerPublisherMonth(month), year, data);
}

void BatLedgerImpl::FetchGrants(const std::string& lang,
    const std::string& payment_id) {
  ledger_->FetchGrants(lang, payment_id);
}

void BatLedgerImpl::GetGrantCaptcha(
    const std::vector<std::string>& headers) {
  ledger_->GetGrantCaptcha(headers);
}

void BatLedgerImpl::GetWalletPassphrase(GetWalletPassphraseCallback callback) {
  std::move(callback).Run(ledger_->GetWalletPassphrase());
}

void BatLedgerImpl::RecoverWallet(const std::string& passPhrase) {
  ledger_->RecoverWallet(passPhrase);
}

void BatLedgerImpl::SolveGrantCaptcha(const std::string& solution,
                                      const std::string& promotion_id) {
  ledger_->SolveGrantCaptcha(solution, promotion_id);
}

void BatLedgerImpl::OnGetAddresses(
    CallbackHolder<GetAddressesCallback>* holder,
    std::map<std::string, std::string> addresses) {
  if (holder->is_valid()) {
    std::move(holder->get()).Run(mojo::MapToFlatMap(addresses));
  }
  delete holder;
}

void BatLedgerImpl::GetAddresses(
    int32_t current_country_code,
    GetAddressesCallback callback) {
  auto* holder = new CallbackHolder<GetAddressesCallback>(
      AsWeakPtr(), std::move(callback));
  ledger_->GetAddresses(current_country_code,
      std::bind(BatLedgerImpl::OnGetAddresses, holder, _1));
}

void BatLedgerImpl::GetBATAddress(GetBATAddressCallback callback) {
  std::move(callback).Run(ledger_->GetBATAddress());
}

void BatLedgerImpl::GetBTCAddress(GetBTCAddressCallback callback) {
  std::move(callback).Run(ledger_->GetBTCAddress());
}

void BatLedgerImpl::GetETHAddress(GetETHAddressCallback callback) {
  std::move(callback).Run(ledger_->GetETHAddress());
}

void BatLedgerImpl::GetLTCAddress(GetLTCAddressCallback callback) {
  std::move(callback).Run(ledger_->GetLTCAddress());
}

void BatLedgerImpl::SetRewardsMainEnabled(bool enabled) {
  ledger_->SetRewardsMainEnabled(enabled);
}

void BatLedgerImpl::SetPublisherMinVisitTime(uint64_t duration_in_seconds) {
  ledger_->SetPublisherMinVisitTime(duration_in_seconds);
}

void BatLedgerImpl::SetPublisherMinVisits(uint32_t visits) {
  ledger_->SetPublisherMinVisits(visits);
}

void BatLedgerImpl::SetPublisherAllowNonVerified(bool allow) {
  ledger_->SetPublisherAllowNonVerified(allow);
}

void BatLedgerImpl::SetPublisherAllowVideos(bool allow) {
  ledger_->SetPublisherAllowVideos(allow);
}

void BatLedgerImpl::SetUserChangedContribution() {
  ledger_->SetUserChangedContribution();
}

void BatLedgerImpl::SetContributionAmount(double amount) {
  ledger_->SetContributionAmount(amount);
}

void BatLedgerImpl::SetAutoContribute(bool enabled) {
  ledger_->SetAutoContribute(enabled);
}

void BatLedgerImpl::UpdateAdsRewards() {
  ledger_->UpdateAdsRewards();
}

void BatLedgerImpl::OnTimer(uint32_t timer_id) {
  ledger_->OnTimer(timer_id);
}

void BatLedgerImpl::GetAllBalanceReports(
    GetAllBalanceReportsCallback callback) {
  auto reports = ledger_->GetAllBalanceReports();
  base::flat_map<std::string, std::string> out_reports;
  for (auto const& report : reports) {
    out_reports[report.first] = report.second.ToJson();
  }
  std::move(callback).Run(out_reports);
}

void BatLedgerImpl::GetBalanceReport(int32_t month, int32_t year,
    GetBalanceReportCallback callback) {
  ledger::BalanceReportInfo info;
  bool result =
    ledger_->GetBalanceReport(ToLedgerPublisherMonth(month), year, &info);
  std::move(callback).Run(result, info.ToJson());
}

void BatLedgerImpl::IsWalletCreated(IsWalletCreatedCallback callback) {
  std::move(callback).Run(ledger_->IsWalletCreated());
}

void BatLedgerImpl::GetPublisherActivityFromUrl(
    uint64_t window_id,
    ledger::VisitDataPtr visit_data,
    const std::string& publisher_blob) {
  ledger_->GetPublisherActivityFromUrl(
      window_id, std::move(visit_data), publisher_blob);
}

// static
void BatLedgerImpl::OnGetPublisherBanner(
    CallbackHolder<GetPublisherBannerCallback>* holder,
    ledger::PublisherBannerPtr banner) {
  if (holder->is_valid())
    std::move(holder->get()).Run(std::move(banner));
  delete holder;
}

void BatLedgerImpl::GetPublisherBanner(const std::string& publisher_id,
    GetPublisherBannerCallback callback) {
  // delete in OnGetPublisherBanner
  auto* holder = new CallbackHolder<GetPublisherBannerCallback>(
      AsWeakPtr(), std::move(callback));
  ledger_->GetPublisherBanner(publisher_id,
      std::bind(BatLedgerImpl::OnGetPublisherBanner, holder, _1));
}

void BatLedgerImpl::GetContributionAmount(
    GetContributionAmountCallback callback) {
  std::move(callback).Run(ledger_->GetContributionAmount());
}

void BatLedgerImpl::DoDirectTip(const std::string& publisher_id,
                                int32_t amount,
                                const std::string& currency) {
  ledger_->DoDirectTip(publisher_id, amount, currency);
}

void BatLedgerImpl::RemoveRecurringTip(const std::string& publisher_key) {
  ledger_->RemoveRecurringTip(publisher_key);
}


void BatLedgerImpl::GetBootStamp(GetBootStampCallback callback) {
  std::move(callback).Run(ledger_->GetBootStamp());
}

void BatLedgerImpl::GetRewardsMainEnabled(
    GetRewardsMainEnabledCallback callback) {
  std::move(callback).Run(ledger_->GetRewardsMainEnabled());
}

void BatLedgerImpl::OnHasSufficientBalanceToReconcile(
    CallbackHolder<HasSufficientBalanceToReconcileCallback>* holder,
    bool sufficient) {
  if (holder->is_valid()) {
    std::move(holder->get()).Run(sufficient);
  }
  delete holder;
}

void BatLedgerImpl::HasSufficientBalanceToReconcile(
    HasSufficientBalanceToReconcileCallback callback) {
  auto* holder = new CallbackHolder<HasSufficientBalanceToReconcileCallback>(
      AsWeakPtr(), std::move(callback));
  ledger_->HasSufficientBalanceToReconcile(
      std::bind(BatLedgerImpl::OnHasSufficientBalanceToReconcile, holder, _1));
}

// static
void BatLedgerImpl::OnAddressesForPaymentId(
    CallbackHolder<GetAddressesForPaymentIdCallback>* holder,
    std::map<std::string, std::string> addresses) {
  if (holder->is_valid())
    std::move(holder->get()).Run(mojo::MapToFlatMap(addresses));
  delete holder;
}

void BatLedgerImpl::GetAddressesForPaymentId(
    GetAddressesForPaymentIdCallback callback) {
  // delete in OnAddressesForPaymentId
  auto* holder = new CallbackHolder<GetAddressesForPaymentIdCallback>(
      AsWeakPtr(), std::move(callback));
  ledger_->GetAddressesForPaymentId(
      std::bind(BatLedgerImpl::OnAddressesForPaymentId, holder, _1));
}

void BatLedgerImpl::SetCatalogIssuers(const std::string& info) {
  ledger_->SetCatalogIssuers(info);
}

void BatLedgerImpl::ConfirmAd(const std::string& info) {
  ledger_->ConfirmAd(info);
}

// static
void BatLedgerImpl::OnGetTransactionHistory(
    CallbackHolder<GetTransactionHistoryCallback>* holder,
    std::unique_ptr<ledger::TransactionsInfo> history) {
  std::string json_transactions = history.get() ? history->ToJson() : "";
  if (holder->is_valid())
    std::move(holder->get()).Run(json_transactions);
  delete holder;
}

void BatLedgerImpl::GetTransactionHistory(
    GetTransactionHistoryCallback callback) {
  auto* holder = new CallbackHolder<GetTransactionHistoryCallback>(
      AsWeakPtr(), std::move(callback));

  ledger_->GetTransactionHistory(
      std::bind(BatLedgerImpl::OnGetTransactionHistory,
          holder, _1));
}

void BatLedgerImpl::GetRewardsInternalsInfo(
    GetRewardsInternalsInfoCallback callback) {
  ledger::RewardsInternalsInfo info;
  ledger_->GetRewardsInternalsInfo(&info);
  std::move(callback).Run(info.ToJson());
}

// static
void BatLedgerImpl::OnGetRecurringTips(
    CallbackHolder<GetRecurringTipsCallback>* holder,
    ledger::PublisherInfoList list,
    uint32_t num) {
  if (holder->is_valid())
    std::move(holder->get()).Run(std::move(list));

  delete holder;
}

void BatLedgerImpl::GetRecurringTips(GetRecurringTipsCallback callback) {
  auto* holder = new CallbackHolder<GetRecurringTipsCallback>(
      AsWeakPtr(), std::move(callback));

  ledger_->GetRecurringTips(std::bind(
      BatLedgerImpl::OnGetRecurringTips, holder, _1, _2));
}

// static
void BatLedgerImpl::OnGetOneTimeTips(
    CallbackHolder<GetRecurringTipsCallback>* holder,
    ledger::PublisherInfoList list,
    uint32_t num) {
  if (holder->is_valid())
    std::move(holder->get()).Run(std::move(list));

  delete holder;
}

void BatLedgerImpl::GetOneTimeTips(GetOneTimeTipsCallback callback) {
  auto* holder = new CallbackHolder<GetOneTimeTipsCallback>(
      AsWeakPtr(), std::move(callback));

  ledger_->GetOneTimeTips(std::bind(
      BatLedgerImpl::OnGetOneTimeTips, holder, _1, _2));
}

// static
void BatLedgerImpl::OnGetActivityInfoList(
    CallbackHolder<GetActivityInfoListCallback>* holder,
    ledger::PublisherInfoList list,
    uint32_t num) {
  if (holder->is_valid())
    std::move(holder->get()).Run(std::move(list), num);

  delete holder;
}

void BatLedgerImpl::GetActivityInfoList(
    uint32_t start,
    uint32_t limit,
    const std::string& json_filter,
    GetActivityInfoListCallback callback) {
  auto* holder = new CallbackHolder<GetActivityInfoListCallback>(
      AsWeakPtr(), std::move(callback));

  ledger::ActivityInfoFilter filter;
  if (filter.loadFromJson(json_filter)) {
    ledger_->GetActivityInfoList(
        start,
        limit,
        filter,
        std::bind(BatLedgerImpl::OnGetActivityInfoList, holder, _1, _2));
  }
}

// static
void BatLedgerImpl::OnLoadPublisherInfo(
    CallbackHolder<LoadPublisherInfoCallback>* holder,
    ledger::Result result,
    ledger::PublisherInfoPtr publisher_info) {
  if (holder->is_valid())
    std::move(holder->get()).Run(result, std::move(publisher_info));

  delete holder;
}

void BatLedgerImpl::LoadPublisherInfo(
    const std::string& publisher_key,
    LoadPublisherInfoCallback callback) {
  auto* holder = new CallbackHolder<LoadPublisherInfoCallback>(
      AsWeakPtr(), std::move(callback));

  ledger_->GetPublisherInfo(
      publisher_key,
      std::bind(BatLedgerImpl::OnLoadPublisherInfo, holder, _1, _2));
}

// static
void BatLedgerImpl::OnSaveMediaInfoCallback(
    CallbackHolder<SaveMediaInfoCallback>* holder,
    ledger::Result result,
    ledger::PublisherInfoPtr publisher_info) {
  if (holder->is_valid()) {
    std::move(holder->get()).Run(result, std::move(publisher_info));
  }

  delete holder;
}

void BatLedgerImpl::SaveMediaInfo(
    const std::string& type,
    const base::flat_map<std::string, std::string>& args,
    SaveMediaInfoCallback callback) {
  auto* holder = new CallbackHolder<SaveMediaInfoCallback>(
      AsWeakPtr(), std::move(callback));

  ledger_->SaveMediaInfo(
      type,
      mojo::FlatMapToMap(args),
      std::bind(BatLedgerImpl::OnSaveMediaInfoCallback, holder, _1, _2));
}

void BatLedgerImpl::OnRefreshPublisher(
    CallbackHolder<RefreshPublisherCallback>* holder,
    bool verified) {
  if (holder->is_valid())
    std::move(holder->get()).Run(verified);

  delete holder;
}

void BatLedgerImpl::RefreshPublisher(
    const std::string& publisher_key,
    RefreshPublisherCallback callback) {
  auto* holder = new CallbackHolder<RefreshPublisherCallback>(
      AsWeakPtr(), std::move(callback));
  ledger_->RefreshPublisher(
      publisher_key,
      std::bind(BatLedgerImpl::OnRefreshPublisher, holder, _1));
}

void BatLedgerImpl::StartMonthlyContribution() {
  ledger_->StartMonthlyContribution();
}

void BatLedgerImpl::SetInlineTipSetting(const std::string& key, bool enabled) {
  ledger_->SetInlineTipSetting(key, enabled);
}

void BatLedgerImpl::GetInlineTipSetting(
    const std::string& key,
    GetInlineTipSettingCallback callback) {
  std::move(callback).Run(ledger_->GetInlineTipSetting(key));
}

void BatLedgerImpl::GetShareURL(
    const std::string& type,
    const base::flat_map<std::string, std::string>& args,
    GetShareURLCallback callback) {
  std::move(callback).Run(ledger_->GetShareURL(type, mojo::FlatMapToMap(args)));
}

// static
void BatLedgerImpl::OnGetPendingContributions(
    CallbackHolder<GetPendingContributionsCallback>* holder,
    ledger::PendingContributionInfoList list) {
  if (holder->is_valid()) {
    std::move(holder->get()).Run(std::move(list));
  }
  delete holder;
}

void BatLedgerImpl::GetPendingContributions(
    GetPendingContributionsCallback callback) {
  auto* holder = new CallbackHolder<GetPendingContributionsCallback>(
      AsWeakPtr(), std::move(callback));

  ledger_->GetPendingContributions(std::bind(
      BatLedgerImpl::OnGetPendingContributions, holder, _1));
}

// static
void BatLedgerImpl::OnRemovePendingContribution(
    CallbackHolder<RemovePendingContributionCallback>* holder,
    ledger::Result result) {
  if (holder->is_valid()) {
    std::move(holder->get()).Run(result);
  }
  delete holder;
}

void BatLedgerImpl::RemovePendingContribution(
    const std::string& publisher_key,
    const std::string& viewing_id,
    uint64_t added_date,
    RemovePendingContributionCallback callback) {
  auto* holder = new CallbackHolder<RemovePendingContributionCallback>(
      AsWeakPtr(), std::move(callback));

  ledger_->RemovePendingContribution(
      publisher_key,
      viewing_id,
      added_date,
      std::bind(BatLedgerImpl::OnRemovePendingContribution,
                holder,
                _1));
}

// static
void BatLedgerImpl::OnRemoveAllPendingContributions(
    CallbackHolder<RemovePendingContributionCallback>* holder,
    ledger::Result result) {
  if (holder->is_valid()) {
    std::move(holder->get()).Run(result);
  }
  delete holder;
}

void BatLedgerImpl::RemoveAllPendingContributions(
    RemovePendingContributionCallback callback) {
  auto* holder = new CallbackHolder<RemovePendingContributionCallback>(
      AsWeakPtr(), std::move(callback));

  ledger_->RemoveAllPendingContributions(
      std::bind(BatLedgerImpl::OnRemoveAllPendingContributions,
                holder,
                _1));
}

// static
void BatLedgerImpl::OnGetPendingContributionsTotal(
    CallbackHolder<GetPendingContributionsTotalCallback>* holder,
    double amount) {
  if (holder->is_valid()) {
    std::move(holder->get()).Run(amount);
  }
  delete holder;
}

void BatLedgerImpl::GetPendingContributionsTotal(
    GetPendingContributionsTotalCallback callback) {
  auto* holder = new CallbackHolder<GetPendingContributionsTotalCallback>(
      AsWeakPtr(), std::move(callback));

  ledger_->GetPendingContributionsTotal(
      std::bind(BatLedgerImpl::OnGetPendingContributionsTotal,
                holder,
                _1));
}

// static
void BatLedgerImpl::OnFetchBalance(
    CallbackHolder<FetchBalanceCallback>* holder,
    ledger::Result result,
    ledger::BalancePtr balance) {
  if (holder->is_valid())
    std::move(holder->get()).Run(result, std::move(balance));
  delete holder;
}

void BatLedgerImpl::FetchBalance(
    FetchBalanceCallback callback) {
  auto* holder = new CallbackHolder<FetchBalanceCallback>(
      AsWeakPtr(), std::move(callback));

  ledger_->FetchBalance(
      std::bind(BatLedgerImpl::OnFetchBalance,
                holder,
                _1,
                _2));
}

}  // namespace bat_ledger