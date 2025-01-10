/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2025 Inviwo Foundation
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
#include <inviwo/dataframe/util/dataframeutil.h>

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>

#include <inviwo/core/util/defaultvalues.h>

#include <fmt/format.h>

namespace inviwo {

namespace {

struct CreateColumn {
    template <typename T>
    auto operator()() {
        auto classname = Defaultvalues<T>::getName();
        TemplateColumn<T> col(classname + "Col", std::vector<T>{});

        auto expectedFormat = DataFormat<T>::get();
        EXPECT_EQ(expectedFormat->getId(), col.getBuffer()->getDataFormat()->getId())
            << "Dataformat mismatch";

        const std::string expectedHeader = fmt::format("{}Col", classname).c_str();
        EXPECT_EQ(expectedHeader, col.getHeader()) << "Column header does not match";
    }
};

}  // namespace

TEST(ColumnTests, Create) {
    using Scalars = std::tuple<float, double, int, glm::i64, size_t, std::uint32_t>;

    util::for_each_type<Scalars>{}(CreateColumn{});
}

TEST(ColumnTests, IndexColumn) {
    IndexColumn col("index", {0, 1, 2, 3});

    EXPECT_EQ(ColumnType::Index, col.getColumnType()) << "Incorrect column type";
    EXPECT_EQ(4, col.getSize()) << "Row count differs";
}

TEST(ColumnTests, CategoricalColumn) {
    CategoricalColumn col("Column");
    col.add("a");
    col.add("c");
    col.add("b");
    col.add("a");

    const std::vector<std::string> expected = {"a", "c", "b"};
    const auto result = col.getCategories();

    const auto rows = col.getSize();

    EXPECT_EQ(ColumnType::Categorical, col.getColumnType()) << "Incorrect column type";
    EXPECT_EQ(4, rows) << "Row count differs";
    EXPECT_EQ(expected, result) << "Categories in column are not correct";
}

TEST(ColumnTests, FloatColumn) {
    TemplateColumn<float> col("Column", {0.0f, 0.1f, 0.5f, 1.0f, 2.0f, 10.0f});

    auto buffer = col.getBuffer();
    const auto rows = col.getSize();

    EXPECT_EQ(ColumnType::Ordinal, col.getColumnType()) << "Incorrect column type";
    ASSERT_EQ(DataFormat<float>::id(), buffer->getDataFormat()->getId())
        << "column buffer has incorrect data format";
    EXPECT_EQ(6, rows) << "Row count differs";

    auto bufferram =
        static_cast<const BufferRAMPrecision<float>*>(buffer->getRepresentation<BufferRAM>());
    const std::vector<float> expected = {0.0f, 0.1f, 0.5f, 1.0f, 2.0f, 10.0f};

    EXPECT_EQ(expected, bufferram->getDataContainer()) << "Column contents differ";
}

TEST(ColumnTests, DataAccess) {
    TemplateColumn<float> col("Column", {0.0f, 0.1f, 0.5f, 1.0f, 2.0f, 10.0f});

    const auto value = col.get(2);
    col.set(2, 5.0f);

    auto buffer = col.getBuffer();
    const auto rows = col.getSize();

    ASSERT_EQ(DataFormat<float>::id(), buffer->getDataFormat()->getId())
        << "column buffer has incorrect data format";
    EXPECT_EQ(6, rows) << "Row count differs";

    EXPECT_EQ(0.5f, value) << "get() returned wrong value";

    auto bufferram =
        static_cast<const BufferRAMPrecision<float>*>(buffer->getRepresentation<BufferRAM>());
    const std::vector<float> expected = {0.0f, 0.1f, 5.0f, 1.0f, 2.0f, 10.0f};

    EXPECT_EQ(expected, bufferram->getDataContainer()) << "Column contents differ";
}

TEST(ColumnAppend, IntColumn) {
    TemplateColumn<int> col("IntCol", {0, 1, 2, 3});
    TemplateColumn<int> col2("IntCol2", {4, 5});

    EXPECT_EQ(4, col.getSize());

    col.append(col2);

    ASSERT_EQ(6, col.getSize()) << "Incorrect number of rows after append";

    const std::vector<int> expected = {0, 1, 2, 3, 4, 5};

    auto bufferram = static_cast<const BufferRAMPrecision<int>*>(
        col.getBuffer()->getRepresentation<BufferRAM>());
    EXPECT_EQ(expected, bufferram->getDataContainer()) << "Row contents incorrect";
}

TEST(ColumnAppend, Categorical) {
    CategoricalColumn col("Column");
    col.add("a");
    col.add("c");
    col.add("b");

    CategoricalColumn col2("Column 2");
    col2.add("d");
    col2.add("e");
    col2.add("a");
    col2.add("b");

    col.append(col2);

    const std::vector<std::string> expected = {"a", "c", "b", "d", "e"};
    const auto result = col.getCategories();

    const auto rows = col.getSize();

    EXPECT_EQ(7, rows) << "Incorrect number of rows after append";
    EXPECT_EQ(expected, result) << "Categories after append are not correct";
}

TEST(ColumnAppend, CategoricalThrow) {
    CategoricalColumn col("Column");
    col.add("a");
    CategoricalColumn catCol("Column");
    catCol.add("b");

    TemplateColumn<int> intCol("IntCol", std::vector<int>{0, 1});

    EXPECT_NO_THROW(col.append(catCol));
    EXPECT_THROW(col.append(intCol), Exception);
}

TEST(ColumnAppend, TypeMismatch) {
    TemplateColumn<int> intCol("IntCol", {0, 1, 2, 3});
    TemplateColumn<float> floatCol("FloatCol", {0.1f, 1.1f, 2.1f, 3.1f});

    EXPECT_THROW(intCol.append(floatCol), Exception);
    EXPECT_THROW(floatCol.append(intCol), Exception);

    EXPECT_NO_THROW(intCol.append(intCol)) << "Cannot append rows of same column type";
}

TEST(ColumnFilter, NoFilter) {
    TemplateColumn<int> intCol("IntCol", {0, 1, 2, 2, 4, 7, 10, 9, 5, 3});

    const auto result = dataframe::selectRows(intCol, {});
    EXPECT_EQ(0, result.size()) << "Incorrect number of filtered rows";
}
TEST(ColumnFilter, Categorical) {
    CategoricalColumn col("Column");
    col.add("a");
    col.add("c");
    col.add("a");
    col.add("b");

    std::vector<dataframefilters::ItemFilter> filters;
    filters.push_back(dataframefilters::stringMatch(0, filters::StringComp::Equal, "a"));

    const auto result = dataframe::selectRows(col, filters);
    EXPECT_EQ(2, result.size()) << "Incorrect number of filtered rows";

    const std::vector<uint32_t> expected = {0, 2};
    EXPECT_EQ(expected, result) << "Filter result does not match";
}

TEST(ColumnFilter, CategoricalMulti) {
    CategoricalColumn col("Column");
    col.add("a");
    col.add("c");
    col.add("a");
    col.add("b");

    std::vector<dataframefilters::ItemFilter> filters;
    filters.push_back(dataframefilters::stringMatch(0, filters::StringComp::Equal, "a"));
    filters.push_back(dataframefilters::stringMatch(0, filters::StringComp::Equal, "b"));

    const auto result = dataframe::selectRows(col, filters);
    EXPECT_EQ(3, result.size()) << "Incorrect number of filtered rows";

    const std::vector<uint32_t> expected = {0, 2, 3};
    EXPECT_EQ(expected, result) << "Filter result does not match";
}

TEST(ColumnFilter, IntLess) {
    TemplateColumn<int> intCol("IntCol", {0, 1, 2, 2, 4, 7, 10, 9, 5, 3});

    std::vector<dataframefilters::ItemFilter> filters;
    filters.push_back(dataframefilters::intMatch(0, filters::NumberComp::Less, 4));

    const auto result = dataframe::selectRows(intCol, filters);
    EXPECT_EQ(5, result.size()) << "Incorrect number of filtered rows";

    const std::vector<uint32_t> expected = {0, 1, 2, 3, 9};
    EXPECT_EQ(expected, result) << "Filter result does not match";
}

TEST(ColumnFilter, IntRange) {
    TemplateColumn<int> intCol("IntCol", {0, 1, 2, 2, 4, 7, 10, 9, 5, 3});

    std::vector<dataframefilters::ItemFilter> filters;
    filters.push_back(dataframefilters::intRange(0, 2, 5));

    const auto result = dataframe::selectRows(intCol, filters);
    EXPECT_EQ(5, result.size()) << "Incorrect number of filtered rows";

    const std::vector<uint32_t> expected = {2, 3, 4, 8, 9};
    EXPECT_EQ(expected, result) << "Filter result does not match";
}

}  // namespace inviwo
