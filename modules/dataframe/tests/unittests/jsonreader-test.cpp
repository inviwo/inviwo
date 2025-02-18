/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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

#include <inviwo/core/io/tempfilehandle.h>
#include <inviwo/dataframe/io/jsondataframereader.h>
#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/core/util/unindent.h>

#include <sstream>

namespace inviwo {
// Tests for confirming Inviwo integration. The json library is heavily tested.

TEST(JSONnoData, file) {
    util::TempFileHandle tmpFile("", ".json");

    JSONDataFrameReader reader;
    EXPECT_THROW(reader.readData(tmpFile.getFileName()), inviwo::FileException);
}

TEST(JSONdata, numRows) {
    // test for correct row count
    std::istringstream ss(IVW_UNINDENT(R"(
        [ 
            {"sepalLength" : 5.1, "sepalWidth" : 3.5, "petalLength" : 1.4, "petalWidth" : 0.2,"species" : "setosa"},
            {"sepalLength" : 4.9, "sepalWidth" : 3.0, "petalLength" : 1.4, "petalWidth" : 0.2,"species" : "setosa"}
        ]
    )"));

    JSONDataFrameReader reader;

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(6, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(2, dataframe->getNumberOfRows()) << "row count does not match";
}

TEST(JSONdata, nullItem) {
    // test for null values, must have at least one were value is not null
    std::istringstream ss(IVW_UNINDENT(R"(
        [
            {"sepalLength" : null, "sepalWidth" : 3.5, "petalLength" : 1.4, "petalWidth" : 0.2,"species" : "setosa"},
            {"sepalLength" : 4.9, "sepalWidth" : 3.0, "petalLength" : 1.4, "petalWidth" : 0.2,"species" : "setosa"}
        ]
    )"));

    JSONDataFrameReader reader;

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(6, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(2, dataframe->getNumberOfRows()) << "row count does not match";

    // Note that json reorders columns...
    EXPECT_EQ("nan", dataframe->getColumn(1)->getAsString(0)) << "first item is not nan";
    EXPECT_EQ(NumericType::Float,
              dataframe->getColumn(3)->getBuffer()->getDataFormat()->getNumericType())
        << "incorrect guessing of data type";
}

TEST(JSONdata, nullColumn) {
    // test for column holding only null values, assumed column datatype should be float
    std::istringstream ss(IVW_UNINDENT(R"(
        [
            {"sepalLength" : null, "sepalWidth" : 3.5, "petalLength" : 1.4, "petalWidth" : 0.2,"species" : "setosa"},
            {"sepalLength" : null, "sepalWidth" : 3.0, "petalLength" : 1.4, "petalWidth" : 0.2,"species" : "setosa"}
        ]
    )"));

    JSONDataFrameReader reader;

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(6, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(2, dataframe->getNumberOfRows()) << "row count does not match";

    // Note that json reorders columns...
    EXPECT_EQ("nan", dataframe->getColumn(1)->getAsString(0)) << "first item is not nan";
    EXPECT_EQ(NumericType::Float,
              dataframe->getColumn(3)->getBuffer()->getDataFormat()->getNumericType())
        << "incorrect guessing of data type";
}

}  // namespace inviwo
