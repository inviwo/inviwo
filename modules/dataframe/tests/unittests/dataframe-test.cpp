/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2021 Inviwo Foundation
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

#include <inviwo/dataframe/datastructures/datapoint.h>
#include <inviwo/dataframe/datastructures/column.h>
#include <inviwo/dataframe/datastructures/dataframe.h>

#include <inviwo/core/datastructures/buffer/buffer.h>

#include <fmt/format.h>

namespace inviwo {

namespace {

struct AddColumnScript {
    template <typename T>
    auto operator()() {
        auto classname = Defaultvalues<T>::getName();

        DataFrame dataframe;
        dataframe.addColumn<T>(classname + "Col", 0u);

        ASSERT_EQ(2, dataframe.getNumberOfColumns()) << "Incorrect number of columns";

        auto col = dataframe.getColumn(1);
        auto expectedFormat = DataFormat<T>::get();
        EXPECT_EQ(expectedFormat->getId(), col->getBuffer()->getDataFormat()->getId())
            << "Dataformat mismatch";
        const std::string expectedHeader = fmt::format("{}Col", classname).c_str();
        EXPECT_STREQ(expectedHeader.c_str(), col->getHeader().c_str())
            << "Column header does not match";
    }
};

}  // namespace

TEST(DataFrameTests, Create) {
    DataFrame dataframe;
    DataFrame dataframe2(10);

    EXPECT_EQ(1, dataframe.getNumberOfColumns()) << "DataFrame contains more than index column";
    EXPECT_EQ(0, dataframe.getNumberOfRows()) << "DataFrame contains rows";

    EXPECT_EQ(1, dataframe2.getNumberOfColumns()) << "DataFrame contains more than index column";
    EXPECT_EQ(0, dataframe2.getNumberOfRows()) << "DataFrame contains rows";
    EXPECT_EQ(10, dataframe2.getIndexColumn()->getSize())
        << "Incorrect size of Index column buffer";
}

TEST(DataFrameTests, AddColumn) {
    using Scalars = std::tuple<float, double, int, glm::i64, std::uint32_t>;

    util::for_each_type<Scalars>{}(AddColumnScript{});
}

TEST(DataFrameTests, AddCategoricalColumn) {
    const std::string colname = "CatColumn";

    DataFrame dataframe;
    dataframe.addCategoricalColumn(colname);

    ASSERT_EQ(2, dataframe.getNumberOfColumns()) << "Incorrect number of columns";

    auto col = dataframe.getColumn(1);
    auto expectedFormat = DataFormat<uint32_t>::get();
    EXPECT_EQ(expectedFormat->getId(), col->getBuffer()->getDataFormat()->getId())
        << "Dataformat mismatch";
    EXPECT_STREQ(colname.c_str(), col->getHeader().c_str()) << "Column header does not match";
}

TEST(DataFrameTests, AddCategoricalColumnData) {
    const std::string colname = "CatColumn";

    DataFrame dataframe;
    dataframe.addCategoricalColumn(colname, {"a", "c", "b"});
    dataframe.updateIndexBuffer();

    ASSERT_EQ(2, dataframe.getNumberOfColumns()) << "Incorrect number of columns";
    ASSERT_EQ(3, dataframe.getNumberOfRows()) << "Incorrect number of rows";

    auto col = dataframe.getColumn(1);
    auto expectedFormat = DataFormat<uint32_t>::get();
    EXPECT_EQ(expectedFormat->getId(), col->getBuffer()->getDataFormat()->getId())
        << "Dataformat mismatch";
    EXPECT_STREQ(colname.c_str(), col->getHeader().c_str()) << "Column header does not match";

    auto catcol = dynamic_cast<CategoricalColumn*>(col.get());
    ASSERT_TRUE(catcol) << "Column is not categorical";

    const auto& result = catcol->getCategories();
    const std::vector<std::string> expected = {"a", "c", "b"};

    EXPECT_EQ(expected, result) << "Categories after append are not correct";
}

TEST(DataFrameTests, AddColumnFromBuffer) {
    const std::string colname = "FloatCol";

    auto buffer = util::makeBuffer(std::vector<float>{1.0f, 2.0f, 3.0f});

    DataFrame dataframe;
    dataframe.addColumnFromBuffer(colname, buffer);
    dataframe.updateIndexBuffer();

    EXPECT_EQ(2, dataframe.getNumberOfColumns()) << "Incorrect number of columns";
    EXPECT_EQ(3, dataframe.getNumberOfRows()) << "Incorrect number of rows";

    auto columnHeader = dataframe.getHeader(1);
    EXPECT_STREQ(colname.c_str(), columnHeader.c_str());
    auto col = dataframe.getColumn(1);
    EXPECT_EQ(3, col->getSize()) << "Column row count incorrect";
}

TEST(DataFrameTests, RowAccess) {
    const size_t rowIndex = 1;

    auto buffer = util::makeBuffer(std::vector<float>{1.0f, 2.0f, 3.0f});

    DataFrame dataframe;
    dataframe.addColumnFromBuffer("FloatCol", buffer);
    auto col = dataframe.addColumn<int>("IntCol", 3);
    col->set(0, 4);
    col->set(1, 5);
    col->set(2, 6);
    dataframe.updateIndexBuffer();

    const size_t numCols = dataframe.getNumberOfColumns();
    ASSERT_EQ(3, dataframe.getNumberOfRows()) << "Incorrect number of rows";

    auto rowValues = dataframe.getDataItem(rowIndex);
    EXPECT_EQ(numCols, rowValues.size()) << "DataFrame::DataItem size incorrect";

    auto indexcol = dataframe.getIndexColumn();
    auto floatcol = std::dynamic_pointer_cast<TemplateColumn<float>>(dataframe.getColumn(1));
    auto intcol = std::dynamic_pointer_cast<const TemplateColumn<int>>(dataframe.getColumn(2));

    ASSERT_TRUE(floatcol) << "column not found";
    ASSERT_TRUE(intcol) << "column not found";

    {
        uint32_t retval = std::static_pointer_cast<DataPoint<uint32_t>>(rowValues[0])->getData();
        const uint32_t expected = indexcol->get(rowIndex);
        EXPECT_EQ(expected, retval);
    }
    {
        float retval = std::static_pointer_cast<DataPoint<float>>(rowValues[1])->getData();
        const float expected = floatcol->get(rowIndex);
        EXPECT_EQ(expected, retval);
    }
    {
        int retval = std::static_pointer_cast<DataPoint<int>>(rowValues[2])->getData();
        const int expected = intcol->get(rowIndex);
        EXPECT_EQ(expected, retval);
    }
}

}  // namespace inviwo
