/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/profiles/brave_profile_manager.h"

#include <string>
#include <memory>

#include "base/files/file_path.h"
#include "base/files/scoped_temp_dir.h"
#include "base/strings/utf_string_conversions.h"
#include "brave/browser/profiles/tor_unittest_profile_manager.h"
#include "brave/browser/tor/tor_launcher_factory.h"
#include "brave/browser/translate/buildflags/buildflags.h"
#include "brave/common/tor/pref_names.h"
#include "brave/common/tor/tor_constants.h"
#include "brave/components/brave_webtorrent/browser/webtorrent_util.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_avatar_icon_util.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/profiles/profiles_state.h"
#include "chrome/browser/net/proxy_config_monitor.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/common/pref_names.h"
#include "chrome/grit/generated_resources.h"
#include "chrome/test/base/scoped_testing_local_state.h"
#include "chrome/test/base/test_browser_window.h"
#include "chrome/test/base/testing_browser_process.h"
#include "chrome/test/base/testing_profile.h"
#include "components/safe_browsing/common/safe_browsing_prefs.h"
#include "components/translate/core/browser/translate_pref_names.h"
#include "content/public/common/webrtc_ip_handling_policy.h"
#include "content/public/test/browser_task_environment.h"
#include "content/public/test/test_utils.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "net/proxy_resolution/proxy_config_with_annotation.h"
#include "net/proxy_resolution/proxy_resolution_service.h"
#include "ui/base/l10n/l10n_util.h"

namespace {

MATCHER(NotFail, "Profile creation failure status is not reported.") {
  return arg == Profile::CREATE_STATUS_CREATED ||
         arg == Profile::CREATE_STATUS_INITIALIZED;
}

}  // namespace

class BraveProfileManagerTest : public testing::Test {
 protected:
  class MockObserver {
   public:
    MOCK_METHOD0(DidLaunchTorProcess, void());
    MOCK_METHOD2(OnProfileCreated,
        void(Profile* profile, Profile::CreateStatus status));
  };

  BraveProfileManagerTest()
      : local_state_(TestingBrowserProcess::GetGlobal()) {
  }

  void SetUp() override {
    // Create a new temporary directory, and store the path
    ASSERT_TRUE(temp_dir_.CreateUniqueTempDir());
    TestingBrowserProcess::GetGlobal()->SetProfileManager(
        new TorUnittestProfileManager(temp_dir_.GetPath()));
  }

  void TearDown() override {
    TestingBrowserProcess::GetGlobal()->SetProfileManager(nullptr);
    content::RunAllTasksUntilIdle();
  }

  // Helper function to create a profile with |name| for a profile |manager|.
  void CreateProfileAsync(ProfileManager* manager,
                          const std::string& name,
                          MockObserver* mock_observer) {
    if (temp_dir_.GetPath().AppendASCII(name) ==
        BraveProfileManager::GetTorProfilePath())
      mock_observer->DidLaunchTorProcess();
    manager->CreateProfileAsync(temp_dir_.GetPath().AppendASCII(name),
                                base::Bind(&MockObserver::OnProfileCreated,
                                           base::Unretained(mock_observer)),
                                base::UTF8ToUTF16(name),
                                profiles::GetDefaultAvatarIconUrl(0));
  }

  // Helper function to set profile ephemeral at prefs and attributes storage.
  void SetProfileEphemeral(Profile* profile) {
    profile->GetPrefs()->SetBoolean(prefs::kForceEphemeralProfiles, true);

    // Update IsEphemeral in attributes storage, normally it happened via
    // kForceEphemeralProfiles pref change event routed to
    // ProfileImpl::UpdateIsEphemeralInStorage().
    ProfileAttributesEntry* entry;
    ProfileAttributesStorage& storage =
        g_browser_process->profile_manager()->GetProfileAttributesStorage();
    EXPECT_TRUE(
        storage.GetProfileAttributesWithPath(profile->GetPath(), &entry));
    entry->SetIsEphemeral(true);
  }

  content::TestBrowserThreadBundle thread_bundle_;

  // The path to temporary directory used to contain the test operations.
  base::ScopedTempDir temp_dir_;
  ScopedTestingLocalState local_state_;

  DISALLOW_COPY_AND_ASSIGN(BraveProfileManagerTest);
};

TEST_F(BraveProfileManagerTest, GetTorProfilePath) {
  base::FilePath tor_path = BraveProfileManager::GetTorProfilePath();
  base::FilePath expected_path = temp_dir_.GetPath();
  expected_path = expected_path.Append(tor::kTorProfileDir);
  EXPECT_EQ(expected_path, tor_path);
}

TEST_F(BraveProfileManagerTest, InitProfileUserPrefs) {
  base::FilePath dest_path =
    temp_dir_.GetPath().AppendASCII(TestingProfile::kTestUserProfileDir);
  base::FilePath tor_dest_path =
    temp_dir_.GetPath().Append(tor::kTorProfileDir);

  ProfileManager* profile_manager = g_browser_process->profile_manager();

  // Successfully create test profile and tor profile
  Profile* profile = profile_manager->GetProfile(dest_path);
  Profile* tor_profile = profile_manager->GetProfile(tor_dest_path);
  ASSERT_TRUE(profile);
  ASSERT_TRUE(tor_profile);

  // Check that the tor_profile name is non empty
  std::string profile_name =
      tor_profile->GetPrefs()->GetString(prefs::kProfileName);
  EXPECT_EQ(profile_name,
            l10n_util::GetStringUTF8(IDS_PROFILES_TOR_PROFILE_NAME));

  // Check that the tor_profile avatar index is valid
  size_t avatar_index =
      tor_profile->GetPrefs()->GetInteger(prefs::kProfileAvatarIndex);
  EXPECT_EQ(avatar_index, size_t(0));
  EXPECT_FALSE(
      tor_profile->GetPrefs()->GetBoolean(prefs::kProfileUsingDefaultName));
  EXPECT_FALSE(profile->GetPrefs()->GetBoolean(tor::prefs::kProfileUsingTor));
  EXPECT_TRUE(
      tor_profile->GetPrefs()->GetBoolean(tor::prefs::kProfileUsingTor));

  // Check WebRTC IP handling policy.
  EXPECT_EQ(
      tor_profile->GetPrefs()->GetString(prefs::kWebRTCIPHandlingPolicy),
      content::kWebRTCIPHandlingDisableNonProxiedUdp);

  // Check SafeBrowsing status
  EXPECT_FALSE(
      tor_profile->GetPrefs()->GetBoolean(prefs::kSafeBrowsingEnabled));

  // Check translate.enabled for translate bubble.
  EXPECT_FALSE(
      tor_profile->GetPrefs()->GetBoolean(prefs::kOfferTranslateEnabled));
}

// This is for tor guest window, remove it when we have persistent tor profiles
// support
TEST_F(BraveProfileManagerTest, TorProfileDontEndUpAsLastProfile) {
  base::FilePath dest_path = temp_dir_.GetPath();
  dest_path = dest_path.Append(tor::kTorProfileDir);

  ProfileManager* profile_manager = g_browser_process->profile_manager();

  TestingProfile* profile =
      static_cast<TestingProfile*>(profile_manager->GetProfile(dest_path));
  ASSERT_TRUE(profile);

  // Here the last used profile is still the "Default" profile.
  Profile* last_used_profile = profile_manager->GetLastUsedProfile();
  EXPECT_NE(profile, last_used_profile);

  // Create a browser for the profile.
  Browser::CreateParams profile_params(profile, true);
  std::unique_ptr<Browser> browser(
      CreateBrowserWithTestWindowForParams(&profile_params));
  last_used_profile = profile_manager->GetLastUsedProfile();
  EXPECT_NE(profile, last_used_profile);

  // Close the browser.
  browser.reset();
  last_used_profile = profile_manager->GetLastUsedProfile();
  EXPECT_NE(profile, last_used_profile);
}

TEST_F(BraveProfileManagerTest, CreateProfilesAsync) {
  const std::string profile_name1 = "New Profile 1";
  const std::string profile_name2 = "Tor Profile";

  MockObserver mock_observer;
  EXPECT_CALL(mock_observer, OnProfileCreated(
      testing::NotNull(), NotFail())).Times(testing::AtLeast(3));
  EXPECT_CALL(mock_observer, DidLaunchTorProcess()).Times(1);

  ProfileManager* profile_manager = g_browser_process->profile_manager();

  CreateProfileAsync(profile_manager, profile_name1, &mock_observer);
  CreateProfileAsync(profile_manager, profile_name2, &mock_observer);

  content::RunAllTasksUntilIdle();
}

TEST_F(BraveProfileManagerTest, NoWebtorrentInTorProfile) {
  ProfileManager* profile_manager = g_browser_process->profile_manager();
  base::FilePath tor_path = BraveProfileManager::GetTorProfilePath();
  Profile* profile = profile_manager->GetProfile(tor_path);
  ASSERT_TRUE(profile);

  EXPECT_FALSE(webtorrent::IsWebtorrentEnabled(profile));
}

TEST_F(BraveProfileManagerTest, ProxyConfigMonitorInTorProfile) {
  ScopedTorLaunchPreventerForTest prevent_tor_process;
  ProfileManager* profile_manager = g_browser_process->profile_manager();
  base::FilePath tor_path = BraveProfileManager::GetTorProfilePath();
  Profile* profile = profile_manager->GetProfile(tor_path);
  ASSERT_TRUE(profile);

  std::unique_ptr<ProxyConfigMonitor> monitor(new ProxyConfigMonitor(profile));
  auto* proxy_config_service = monitor->GetProxyConfigServiceForTesting();
  net::ProxyConfigWithAnnotation config;
  proxy_config_service->GetLatestProxyConfig(&config);
  const std::string& proxy_uri =
      config.value().proxy_rules().single_proxies.Get().ToURI();
  EXPECT_EQ(proxy_uri, g_browser_process->local_state()
            ->GetString(tor::prefs::kTorProxyString));
}
