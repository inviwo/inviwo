/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2022 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <inviwo/core/datastructures/unitsystem.h>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <iostream>

namespace inviwo {

TEST(Unitsystem, unitsystem) {

    Unit m = units::unit_from_string("m");
    Unit km = units::unit_from_string("km");

    EXPECT_EQ(fmt::format("{:si}", m), "m");
    EXPECT_EQ(fmt::format("{: si}", m), " m");
    EXPECT_EQ(fmt::format("{:(si}", m), "(m)");
    EXPECT_EQ(fmt::format("{:[si}", m), "[m]");
    EXPECT_EQ(fmt::format("{: (si}", m), " (m)");
    EXPECT_EQ(fmt::format("{: [si}", m), " [m]");
    EXPECT_EQ(fmt::format("{: [psi}", km), " [km]");
    EXPECT_EQ(fmt::format("{: [Psi}", km), " [1e+03 m]");

    EXPECT_EQ(fmt::format("{: [si}", m.pow(3)), " [m³]");
    EXPECT_EQ(fmt::format("{: [si}", m.pow(-1)), " [1/m]");
    EXPECT_EQ(fmt::format("{: [si}", m.pow(-3)), " [1/m³]");

    EXPECT_EQ(fmt::format("{:[si:<7}", m), "[m]    ");
    EXPECT_EQ(fmt::format("{:[si:^7}", m), "  [m]  ");
    EXPECT_EQ(fmt::format("{:[si:>7}", m), "    [m]");
    EXPECT_EQ(fmt::format("{:[si:*^7}", m), "**[m]**");

    Unit N = units::unit_from_string("N");

    EXPECT_EQ(fmt::format("{:si}", N), "kgm/s²");
    EXPECT_EQ(fmt::format("{:ext}", N), "N");

    Unit chargeDensity = units::unit_from_string("e/Å^3");
    Unit length = units::unit_from_string("Å");

    EXPECT_EQ(fmt::format("{:all}", chargeDensity), "e/Å³");
    EXPECT_EQ(fmt::format("{:all}", chargeDensity * length.pow(3)), "e");
}

}  // namespace inviwo
