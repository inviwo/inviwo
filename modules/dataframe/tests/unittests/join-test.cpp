/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2022 Inviwo Foundation
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

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/util/exception.h>

#include <fmt/format.h>

namespace inviwo {

namespace {

template <typename T>
void checkColumnContents(const Column& col, const std::vector<T>& expected) {
    EXPECT_EQ(expected.size(), col.getSize())
        << fmt::format("Column should have {} rows", expected.size());
    auto buffer = col.getBuffer();
    auto bufferram =
        static_cast<const BufferRAMPrecision<T>*>(buffer->getRepresentation<BufferRAM>());
    EXPECT_EQ(expected, bufferram->getDataContainer())
        << fmt::format("Column contents for '{}' differ", col.getHeader());
}

}  // namespace

TEST(AppendRows, ByOrder) {
    DataFrame top;
    top.addColumnFromBuffer("col", util::makeBuffer(std::vector<int>{1, 2}));
    top.updateIndexBuffer();

    DataFrame bottom;
    bottom.addColumnFromBuffer("col", util::makeBuffer(std::vector<int>{3, 4}));
    bottom.updateIndexBuffer();

    auto dataframe = dataframe::appendRows(top, bottom, false);

    EXPECT_EQ(2, dataframe->getNumberOfColumns())
        << "DataFrame should have 2 columns (index + 'col')";
    EXPECT_EQ(4, dataframe->getNumberOfRows()) << "Row count after appendRows not correct";

    checkColumnContents<int>(*dataframe->getColumn("col"), {1, 2, 3, 4});
}

TEST(AppendRows, ByName) {
    DataFrame top;
    top.addColumnFromBuffer("int", util::makeBuffer(std::vector<int>{1, 2}));
    top.addColumnFromBuffer("float", util::makeBuffer(std::vector<float>{1.0f, 2.0f}));
    top.updateIndexBuffer();

    DataFrame bottom;
    bottom.addColumnFromBuffer("float", util::makeBuffer(std::vector<float>{3.0f, 4.0f}));
    bottom.addColumnFromBuffer("int", util::makeBuffer(std::vector<int>{3, 4}));
    bottom.updateIndexBuffer();

    auto dataframe = dataframe::appendRows(top, bottom, true);

    EXPECT_EQ(3, dataframe->getNumberOfColumns()) << "DataFrame should have 3 columns";
    EXPECT_EQ(4, dataframe->getNumberOfRows()) << "Row count after appendRows not correct";

    checkColumnContents<int>(*dataframe->getColumn("int"), {1, 2, 3, 4});

    checkColumnContents<float>(*dataframe->getColumn("float"), {1.0f, 2.0f, 3.0f, 4.0f});
}

TEST(AppendRows, Exceptions) {
    DataFrame top;
    top.addColumn(std::make_shared<TemplateColumn<int>>("int"));
    DataFrame bottom;
    bottom.addColumn(std::make_shared<TemplateColumn<float>>("float"));

    EXPECT_THROW(dataframe::appendRows(top, bottom, false), Exception) << "Column type mismatch";

    top.addColumn(std::make_shared<TemplateColumn<float>>("float"));
    EXPECT_THROW(dataframe::appendRows(top, bottom, false), Exception) << "Column count differs";

    bottom.addColumn(std::make_shared<TemplateColumn<int>>("int"));
    EXPECT_NO_THROW(dataframe::appendRows(top, bottom, true)) << "Could not match columns by name";

    top.addColumn(std::make_shared<TemplateColumn<int>>("int2"));
    bottom.addColumn(std::make_shared<TemplateColumn<int>>("float2"));
    EXPECT_THROW(dataframe::appendRows(top, bottom, true), Exception)
        << "Columns matched despite different names";
}

TEST(AppendColumns, DuplicateColumns) {
    DataFrame left;
    left.addColumn(std::make_shared<TemplateColumn<int>>("int"));
    left.addColumn(std::make_shared<TemplateColumn<float>>("float"));
    left.updateIndexBuffer();

    DataFrame right;
    right.addColumn(std::make_shared<TemplateColumn<int>>("int2"));
    right.addColumn(std::make_shared<TemplateColumn<int>>("float"));
    right.updateIndexBuffer();

    {
        auto dataframe = dataframe::appendColumns(left, right, false, false);
        EXPECT_EQ(5, dataframe->getNumberOfColumns()) << "DataFrame should have 5 columns";
    }

    {
        auto dataframe = dataframe::appendColumns(left, right, true, false);
        EXPECT_EQ(4, dataframe->getNumberOfColumns())
            << "Column duplicates detected (expected: 3 data columns + index)";
    }
}

TEST(AppendColumns, Contents) {
    DataFrame left;
    left.addColumnFromBuffer("int", util::makeBuffer(std::vector<int>{1, 2}));
    left.updateIndexBuffer();

    DataFrame right;
    right.addColumnFromBuffer("float", util::makeBuffer(std::vector<float>{3.0f, 4.0f}));
    right.updateIndexBuffer();

    auto dataframe = dataframe::appendColumns(left, right, false, false);

    checkColumnContents<int>(*dataframe->getColumn("int"), {1, 2});
    checkColumnContents<float>(*dataframe->getColumn("float"), {3.0f, 4.0f});
}

TEST(AppendColumns, MissingRows) {
    DataFrame left;
    left.addColumnFromBuffer("int", util::makeBuffer(std::vector<int>{1, 2, 3}));
    left.updateIndexBuffer();

    DataFrame right;
    right.addColumnFromBuffer("float", util::makeBuffer(std::vector<float>{3.0f}));
    right.updateIndexBuffer();

    EXPECT_THROW(dataframe::appendColumns(left, right, false, false), Exception);

    auto dataframe = dataframe::appendColumns(left, right, false, true);
    EXPECT_EQ(3, dataframe->getNumberOfRows()) << "Append was not successful";
    EXPECT_EQ(3, dataframe->getNumberOfColumns()) << "DataFrame should have 3 columns";

    checkColumnContents<float>(*dataframe->getColumn("float"), {3.0f, 0.0f, 0.0f});
}

TEST(AppendColumns, MissingRowsCategorical) {
    DataFrame left;
    left.addColumnFromBuffer("int", util::makeBuffer(std::vector<int>{1, 2, 3}));
    left.updateIndexBuffer();

    DataFrame right;
    auto col = right.addCategoricalColumn("cat");
    col->add("foo");
    col->add("bar");
    right.updateIndexBuffer();

    EXPECT_THROW(dataframe::appendColumns(left, right, false, false), Exception);

    auto dataframe = dataframe::appendColumns(left, right, false, true);
    EXPECT_EQ(3, dataframe->getNumberOfRows()) << "Append was not successful";
    EXPECT_EQ(3, dataframe->getNumberOfColumns()) << "DataFrame should have 3 columns";

    auto catCol = dynamic_cast<CategoricalColumn*>(dataframe->getColumn("cat").get());

    const std::vector<std::string> expected = {"foo", "bar", "undefined"};
    const auto result = catCol->getCategories();

    EXPECT_EQ(expected, result) << "Categories after append are not correct";
}

TEST(InnerJoin, ByIndexColumn) {
    DataFrame left;
    left.addColumnFromBuffer("int", util::makeBuffer(std::vector<int>{1, 2, 3}));
    left.updateIndexBuffer();

    DataFrame right;
    right.addColumnFromBuffer("float", util::makeBuffer(std::vector<float>{3.0f, 4.0f}));
    right.updateIndexBuffer();

    auto dataframe = dataframe::innerJoin(left, right);
    EXPECT_EQ(2, dataframe->getNumberOfRows()) << "inner join should result in 2 rows";
    EXPECT_EQ(3, dataframe->getNumberOfColumns()) << "inner join should result in 3 columns";

    checkColumnContents<int>(*dataframe->getColumn("int"), {1, 2});
    checkColumnContents<float>(*dataframe->getColumn("float"), {3.0f, 4.0f});
}

TEST(InnerJoin, ByCustomColumn) {
    DataFrame left;
    left.addColumnFromBuffer("int", util::makeBuffer(std::vector<int>{8, 42}));
    left.updateIndexBuffer();

    DataFrame right;
    right.addColumnFromBuffer("float", util::makeBuffer(std::vector<float>{3.0f, 4.0f, 5.0f}));
    right.addColumnFromBuffer("int2", util::makeBuffer(std::vector<int>{42, 3, 8}));
    right.updateIndexBuffer();

    EXPECT_THROW(dataframe::innerJoin(left, right, "int"), Exception);

    right.getColumn("int2")->setHeader("int");

    auto dataframe = dataframe::innerJoin(left, right, "int");
    EXPECT_EQ(2, dataframe->getNumberOfRows()) << "inner join should result in 2 rows";
    EXPECT_EQ(3, dataframe->getNumberOfColumns()) << "inner join should result in 3 columns";
    EXPECT_TRUE(dataframe->getColumn("float"));

    checkColumnContents<int>(*dataframe->getColumn("int"), {8, 42});
    checkColumnContents<float>(*dataframe->getColumn("float"), {5.0f, 3.0f});
}

TEST(InnerJoin, Categorical) {
    DataFrame left;
    left.addColumnFromBuffer("int", util::makeBuffer(std::vector<int>{1, 2, 3}));
    left.updateIndexBuffer();

    DataFrame right;
    auto col = right.addCategoricalColumn("cat");
    col->add("foo");
    col->add("bar");
    right.updateIndexBuffer();

    auto dataframe = dataframe::innerJoin(left, right);
    EXPECT_EQ(2, dataframe->getNumberOfRows()) << "inner join should result in 2 rows";
    EXPECT_EQ(3, dataframe->getNumberOfColumns()) << "inner join should result in 3 columns";

    checkColumnContents<int>(*dataframe->getColumn("int"), {1, 2});

    auto catCol = dynamic_cast<CategoricalColumn*>(dataframe->getColumn("cat").get());
    ASSERT_TRUE(catCol != nullptr) << "column 'cat' is not categorical after join";

    const std::vector<std::string> expected = {"foo", "bar"};
    const auto result = catCol->getCategories();

    EXPECT_EQ(expected, result) << "categories after join are not correct";
}

TEST(InnerJoin, ByCategoricalColumn) {
    DataFrame left;
    left.addCategoricalColumn("cat", {"a", "b", "c"});
    left.updateIndexBuffer();

    DataFrame right;
    right.addCategoricalColumn("cat", {"a", "c", "b"});
    right.addColumnFromBuffer("int", util::makeBuffer(std::vector<int>{1, 2, 3}));
    right.updateIndexBuffer();

    auto dataframe = dataframe::innerJoin(left, right, "cat");
    EXPECT_EQ(3, dataframe->getNumberOfRows()) << "inner join should result in 5 rows";
    EXPECT_EQ(3, dataframe->getNumberOfColumns()) << "inner join should result in 3 columns";
    EXPECT_TRUE(dataframe->getColumn("int"));

    auto catCol = dynamic_cast<CategoricalColumn*>(dataframe->getColumn("cat").get());
    ASSERT_TRUE(catCol != nullptr) << "column 'cat' is not categorical after join";

    const std::vector<std::string> expected = {"a", "b", "c"};
    const std::vector<std::string> result{catCol->begin(), catCol->end()};
    EXPECT_EQ(expected, result) << "column contents of categorical column differ";

    checkColumnContents<int>(*dataframe->getColumn("int"), {1, 3, 2});
}

TEST(LeftJoin, ByIndexColumn) {
    DataFrame left;
    left.addColumnFromBuffer("int", util::makeBuffer(std::vector<int>{1, 2, 3}));
    left.updateIndexBuffer();

    DataFrame right;
    right.addColumnFromBuffer("float", util::makeBuffer(std::vector<float>{3.0f, 4.0f}));
    right.updateIndexBuffer();

    auto dataframe = dataframe::leftJoin(left, right);
    EXPECT_EQ(3, dataframe->getNumberOfRows()) << "left join should result in 3 rows";
    EXPECT_EQ(3, dataframe->getNumberOfColumns()) << "left join should result in 3 columns";

    checkColumnContents<int>(*dataframe->getColumn("int"), {1, 2, 3});
    checkColumnContents<float>(*dataframe->getColumn("float"), {3.0f, 4.0f, 0.0f});
}

TEST(LeftJoin, ByCustomColumn) {
    DataFrame left;
    left.addColumnFromBuffer("int", util::makeBuffer(std::vector<int>{8, 42, 1, 3, 42}));
    left.updateIndexBuffer();

    DataFrame right;
    right.addColumnFromBuffer("float", util::makeBuffer(std::vector<float>{3.0f, 4.0f, 5.0f}));
    right.addColumnFromBuffer("int2", util::makeBuffer(std::vector<int>{42, 3, 8}));
    right.updateIndexBuffer();

    EXPECT_THROW(dataframe::leftJoin(left, right, "int"), Exception);

    right.getColumn("int2")->setHeader("int");

    auto dataframe = dataframe::leftJoin(left, right, "int");
    EXPECT_EQ(5, dataframe->getNumberOfRows()) << "left join should result in 5 rows";
    EXPECT_EQ(3, dataframe->getNumberOfColumns()) << "left join should result in 3 columns";
    EXPECT_TRUE(dataframe->getColumn("float"));

    checkColumnContents<int>(*dataframe->getColumn("int"), {8, 42, 1, 3, 42});
    checkColumnContents<float>(*dataframe->getColumn("float"), {5.0f, 3.0f, 0.0f, 4.0f, 3.0f});
}

TEST(LeftJoin, Categorical) {
    DataFrame left;
    left.addColumnFromBuffer("int", util::makeBuffer(std::vector<int>{1, 2, 3}));
    left.updateIndexBuffer();

    DataFrame right;
    right.addCategoricalColumn("cat", {"foo", "bar"});
    right.updateIndexBuffer();

    auto dataframe = dataframe::leftJoin(left, right);
    EXPECT_EQ(3, dataframe->getNumberOfRows()) << "inner join should result in 3 rows";
    EXPECT_EQ(3, dataframe->getNumberOfColumns()) << "inner join should result in 3 columns";

    checkColumnContents<int>(*dataframe->getColumn("int"), {1, 2, 3});

    auto catCol = dynamic_cast<CategoricalColumn*>(dataframe->getColumn("cat").get());
    ASSERT_TRUE(catCol != nullptr) << "column 'cat' is not categorical after join";

    const std::vector<std::string> expected = {"foo", "bar", "undefined"};
    const auto result = catCol->getCategories();

    EXPECT_EQ(expected, result) << "categories after join are not correct";
}

TEST(LeftJoin, MultipleKeyColumns) {
    DataFrame left;
    left.addColumnFromBuffer("int", util::makeBuffer(std::vector<int>{1, 1, 1, 2, 2, 4, 3, 1}));
    left.addCategoricalColumn("cat", {"b", "a", "c", "b", "c", "a", "a", "d"});
    left.updateIndexBuffer();

    DataFrame right;
    right.addColumnFromBuffer("float",
                              util::makeBuffer(std::vector<float>{3.0f, 4.0f, 5.0f, 6.0f, 7.0f}));
    right.addCategoricalColumn("cat", {"a", "b", "c", "a", "d"});
    right.addColumnFromBuffer("int", util::makeBuffer(std::vector<int>{1, 1, 2, 3, 1}));
    right.addCategoricalColumn("cat2", {"hello", "world", "test", "cat", "dog"});
    right.updateIndexBuffer();

    auto dataframe = dataframe::leftJoin(left, right, std::vector<std::string>{"int", "cat"});
    EXPECT_EQ(8, dataframe->getNumberOfRows()) << "left join should result in 8 rows";
    EXPECT_EQ(5, dataframe->getNumberOfColumns()) << "left join should result in 5 columns";
    EXPECT_TRUE(dataframe->getColumn("float"));

    auto catCol = dynamic_cast<CategoricalColumn*>(dataframe->getColumn("cat2").get());
    ASSERT_TRUE(catCol != nullptr) << "column 'cat' is not categorical after join";

    const std::vector<std::string> expected = {"world", "hello",     "undefined", "undefined",
                                               "test",  "undefined", "cat",       "dog"};
    const std::vector<std::string> result{catCol->begin(), catCol->end()};
    EXPECT_EQ(expected, result) << "contents of categorical column 'cat2' differ";

    checkColumnContents<float>(*dataframe->getColumn("float"),
                               {4.0f, 3.0f, 0.0f, 0.0f, 5.0f, 0.0f, 6.0f, 7.0f});
}

}  // namespace inviwo
