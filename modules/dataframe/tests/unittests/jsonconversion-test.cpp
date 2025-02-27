/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <inviwo/dataframe/jsondataframeconversion.h>
#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/core/util/unindent.h>
#include <inviwo/core/util/zip.h>

#include <algorithm>
#include <ranges>

namespace inviwo {

TEST(JSONConversion, JSONtoDataFrame) {
    constexpr std::array<std::string_view, 6> headers = {"index",       "sepalLength", "sepalWidth",
                                                         "petalLength", "petalWidth",  "species"};
    const std::string source{IVW_UNINDENT(R"(
        {
            "columns": [
                "sepalLength",
                "sepalWidth",
                "petalLength",
                "petalWidth",
                "species"
            ],
            "data": [
                [
                    5.1,
                    3.5,
                    1.4,
                    0.2,
                    "setosa"
                ],
                [
                    4.9,
                    3.0,
                    1.4,
                    0.2,
                    "setosa"
                ]
            ],
            "index": [
                0,
                1
            ],
            "types": [
                "FLOAT64",
                "FLOAT64",
                "FLOAT64",
                "FLOAT64",
                "CATEGORICAL"
            ]
        }
    )")};

    DataFrame dataframe;
    dataframe = json::parse(source);

    ASSERT_EQ(6, dataframe.getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(2, dataframe.getNumberOfRows()) << "row count does not match";

    // check column order
    for (auto&& [col, header] : util::zip(dataframe, headers)) {
        EXPECT_EQ(col->getHeader(), header) << "column header does not match";
    }
}

TEST(JSONConversion, DataFrameToJSON) {
    constexpr std::string_view refJson(
        R"({"columns":["sepalLength","petalLength","petalWidth","species"],"data":[[5.1,1.4,0.2,"setosa"],[4.9,1.4,0.2,"setosa"]],"index":[0,1],"types":["FLOAT64","FLOAT64","FLOAT64","CATEGORICAL"]})");

    DataFrame dataframe;
    dataframe.addColumn("sepalLength", std::vector<double>{5.1, 4.9});
    dataframe.addColumn("petalLength", std::vector<double>{1.4, 1.4});
    dataframe.addColumn("petalWidth", std::vector<double>{0.2, 0.2});
    dataframe.addCategoricalColumn("species", std::vector<std::string>{"setosa", "setosa"});
    dataframe.updateIndexBuffer();

    json j;
    j = dataframe;

    EXPECT_EQ(refJson, j.dump()) << "JSON dump not matching";
}

TEST(JSONConversion, DataFrameToDataFrame) {
    // convert a DataFrame to JSON and back
    const std::vector<double> sepalLength{5.1, 4.9};
    const std::vector<double> petalLength{1.4, 1.4};
    const std::vector<float> petalWidth{0.2f, 0.2f};
    const std::vector<std::string> species{"setosa", "setosa"};

    DataFrame dataframe;
    dataframe.addColumn("sepalLength", sepalLength);
    dataframe.addColumn("petalLength", petalLength);
    dataframe.addColumn("petalWidth", petalWidth);
    dataframe.addCategoricalColumn("species", species);
    dataframe.updateIndexBuffer();

    json j;
    j = dataframe;

    DataFrame result = j;

    ASSERT_EQ(dataframe.getNumberOfColumns(), result.getNumberOfColumns())
        << "column count does not match";
    ASSERT_EQ(dataframe.getNumberOfRows(), result.getNumberOfRows()) << "row count does not match";

    constexpr std::array<DataFormatId, 5> expectedFormat = {
        DataFormatId::UInt32, DataFormatId::Float64, DataFormatId::Float64, DataFormatId::Float32,
        DataFormatId::UInt32};

    for (auto&& [index, col] : util::enumerate(dataframe)) {
        EXPECT_EQ(expectedFormat[index], col->getBuffer()->getDataFormat()->getId())
            << "format mismatch";
    }
    EXPECT_EQ(ColumnType::Categorical, dataframe.getColumn("species")->getColumnType());

    auto sepalLengthCol = result.getColumn("sepalLength");
    auto petalLengthCol = result.getColumn("petalLength");
    auto petalWidthCol = result.getColumn("petalWidth");
    auto speciesCol = result.getColumn("species");

    bool isMatching = true;
    for (auto row : std::ranges::iota_view{size_t{0}, result.getNumberOfRows()}) {
        isMatching &= util::almostEqual(sepalLength[row], sepalLengthCol->getAsDouble(row));
        isMatching &= util::almostEqual(petalLength[row], petalLengthCol->getAsDouble(row));
        isMatching &=
            util::almostEqual(petalWidth[row], static_cast<float>(petalWidthCol->getAsDouble(row)));
        isMatching &= species[row] == speciesCol->getAsString(row);
    }
    EXPECT_TRUE(isMatching) << "Converted DataFrame does not match source DataFrame";
}

}  // namespace inviwo
