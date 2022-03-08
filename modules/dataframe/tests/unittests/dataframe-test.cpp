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

#include <inviwo/dataframe/datastructures/column.h>
#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/dataframe/util/dataframeutil.h>
#include <inviwo/dataframe/util/filters.h>

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
        EXPECT_EQ(expectedHeader, col->getHeader()) << "Column header does not match";
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
    EXPECT_EQ(colname, col->getHeader()) << "Column header does not match";
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
    EXPECT_EQ(colname, col->getHeader()) << "Column header does not match";

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

    auto col = dataframe.getColumn(1);
    EXPECT_EQ(colname, col->getHeader());
    EXPECT_EQ(3, col->getSize()) << "Column row count incorrect";
}

TEST(DataFrameTests, RowAccess) {
    const size_t rowIndex = 1;

    auto buffer = util::makeBuffer(std::vector<float>{1.0f, 2.0f, 3.0f});

    DataFrame dataframe;
    dataframe.addColumnFromBuffer("FloatCol", buffer);
    dataframe.addColumn<int>("IntCol", std::vector<int>{4, 5, 6});
    dataframe.updateIndexBuffer();

    EXPECT_EQ(3, dataframe.getNumberOfColumns()) << "Incorrect number of columns";
    ASSERT_EQ(3, dataframe.getNumberOfRows()) << "Incorrect number of rows";

    auto indexcol = dataframe.getIndexColumn();
    auto floatcol = std::dynamic_pointer_cast<TemplateColumn<float>>(dataframe.getColumn(1));
    auto intcol = std::dynamic_pointer_cast<const TemplateColumn<int>>(dataframe.getColumn(2));

    ASSERT_TRUE(floatcol) << "Column not found";
    ASSERT_TRUE(intcol) << "Column not found";

    EXPECT_EQ(rowIndex, indexcol->get(rowIndex)) << "Incorrect row index";
    EXPECT_EQ(2.0f, floatcol->get(rowIndex));
    EXPECT_EQ(5, intcol->get(rowIndex));
}

TEST(DataFrameFilter, NoFilter) {
    DataFrame dataframe;
    dataframe.addColumnFromBuffer("FloatCol",
                                  util::makeBuffer(std::vector<float>{1.0f, 2.0f, 3.0f}));
    dataframe.updateIndexBuffer();

    EXPECT_EQ(2, dataframe.getNumberOfColumns()) << "Incorrect number of columns";
    EXPECT_EQ(3, dataframe.getNumberOfRows()) << "Incorrect number of rows";

    const auto result = dataframe::selectRows(dataframe, {});

    EXPECT_EQ(dataframe.getNumberOfRows(), result.size()) << "Incorrect number of filtered rows";
}

TEST(DataFrameFilter, SingleColumn) {
    DataFrame dataframe;
    dataframe.addColumnFromBuffer("FloatCol",
                                  util::makeBuffer(std::vector<float>{1.0f, 2.0f, 3.0f}));
    dataframe.updateIndexBuffer();

    EXPECT_EQ(2, dataframe.getNumberOfColumns()) << "Incorrect number of columns";
    EXPECT_EQ(3, dataframe.getNumberOfRows()) << "Incorrect number of rows";

    dataframefilters::Filters filters;
    filters.include.push_back(
        dataframefilters::doubleMatch(1, filters::NumberComp::LessEqual, 2.0));

    const auto result = dataframe::selectRows(dataframe, filters);
    EXPECT_EQ(2, result.size()) << "Incorrect number of filtered rows";

    const std::vector<uint32_t> expected = {0, 1};
    EXPECT_EQ(expected, result) << "Filter result does not match";
}

TEST(DataFrameFilter, MultiColumn) {
    DataFrame dataframe;
    dataframe.addColumnFromBuffer("FloatCol",
                                  util::makeBuffer(std::vector<float>{1.0f, 2.0f, 3.0f}));
    dataframe.addColumnFromBuffer("FloatCol2",
                                  util::makeBuffer(std::vector<float>{3.2f, 2.2f, 1.2f}));
    dataframe.updateIndexBuffer();

    EXPECT_EQ(3, dataframe.getNumberOfColumns()) << "Incorrect number of columns";
    EXPECT_EQ(3, dataframe.getNumberOfRows()) << "Incorrect number of rows";

    dataframefilters::Filters filters;
    filters.include.push_back(dataframefilters::doubleMatch(2, filters::NumberComp::Less, 2.3));

    const auto result = dataframe::selectRows(dataframe, filters);
    EXPECT_EQ(2, result.size()) << "Incorrect number of filtered rows";

    const std::vector<uint32_t> expected = {1, 2};
    EXPECT_EQ(expected, result) << "Filter result does not match";
}

TEST(DataFrameFilter, MultiColumnFilter) {
    DataFrame dataframe;
    dataframe.addColumnFromBuffer("FloatCol",
                                  util::makeBuffer(std::vector<float>{1.0f, 2.0f, 3.0f}));
    dataframe.addColumnFromBuffer("IntCol", util::makeBuffer(std::vector<int>{6, 2, 3}));
    dataframe.addColumnFromBuffer("FloatCol2",
                                  util::makeBuffer(std::vector<float>{3.2f, 2.2f, 1.2f}));
    dataframe.updateIndexBuffer();

    EXPECT_EQ(4, dataframe.getNumberOfColumns()) << "Incorrect number of columns";
    EXPECT_EQ(3, dataframe.getNumberOfRows()) << "Incorrect number of rows";

    dataframefilters::Filters filters;
    filters.include.push_back(dataframefilters::intMatch(2, filters::NumberComp::Equal, 2));
    filters.include.push_back(dataframefilters::doubleMatch(3, filters::NumberComp::Less, 2.3));

    const auto result = dataframe::selectRows(dataframe, filters);
    EXPECT_EQ(2, result.size()) << "Incorrect number of filtered rows";

    const std::vector<uint32_t> expected = {1, 2};
    EXPECT_EQ(expected, result) << "Filter result does not match";
}

TEST(DataFrameFilter, MultiColumnIncludeExclude) {
    DataFrame dataframe;
    dataframe.addColumnFromBuffer("FloatCol",
                                  util::makeBuffer(std::vector<float>{1.0f, 2.0f, 3.0f}));
    dataframe.addColumnFromBuffer("IntCol", util::makeBuffer(std::vector<int>{6, 2, 3}));
    dataframe.addCategoricalColumn("CatCol", {"a", "a", "b"});
    dataframe.updateIndexBuffer();

    EXPECT_EQ(4, dataframe.getNumberOfColumns()) << "Incorrect number of columns";
    EXPECT_EQ(3, dataframe.getNumberOfRows()) << "Incorrect number of rows";

    dataframefilters::Filters filters;
    filters.include.push_back(dataframefilters::intMatch(2, filters::NumberComp::GreaterEqual, 2));
    filters.exclude.push_back(dataframefilters::stringMatch(3, filters::StringComp::Equal, "a"));

    const auto result = dataframe::selectRows(dataframe, filters);
    EXPECT_EQ(1, result.size()) << "Incorrect number of filtered rows";

    const std::vector<uint32_t> expected = {2};
    EXPECT_EQ(expected, result) << "Filter result does not match";
}

}  // namespace inviwo
