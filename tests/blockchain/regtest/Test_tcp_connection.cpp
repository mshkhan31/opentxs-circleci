// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Helpers.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>

namespace ottest
{
TEST_F(Regtest_fixture_tcp, init_opentxs) {}

TEST_F(Regtest_fixture_tcp, start_chains) { EXPECT_TRUE(Start()); }

TEST_F(Regtest_fixture_tcp, connect_peers) { EXPECT_TRUE(Connect()); }

TEST_F(Regtest_fixture_tcp, shutdown) { Shutdown(); }
}  // namespace ottest
