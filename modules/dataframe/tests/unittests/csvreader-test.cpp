/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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
#include <inviwo/dataframe/io/csvreader.h>

#include <sstream>

namespace inviwo {

TEST(CSVnoData, stream) {
    std::istringstream ss("");

    CSVReader reader;
    EXPECT_THROW(reader.readData(ss), inviwo::CSVDataReaderException);
}

TEST(CSVnoData, file) {
    util::TempFileHandle tmpFile("", ".csv");

    CSVReader reader;
    EXPECT_THROW(reader.readData(tmpFile.getFileName()), inviwo::CSVDataReaderException);
}

TEST(CSVdata, numRows) {
    // test for correct row count
    std::istringstream ss("1\n2\n3\n4\n\5\n6");

    CSVReader reader;
    reader.setFirstRowHeader(false);

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(2, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(6, dataframe->getNumberOfRows()) << "row count does not match";
}

TEST(CSVdata, numCols) {
    // test for correct column count
    std::istringstream ss("1,a\n2,b\n3,c");

    CSVReader reader;
    reader.setFirstRowHeader(false);

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(3, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(3, dataframe->getNumberOfRows()) << "row count does not match";
}

TEST(CSVdata, emptyRows) {
    // test for empty rows in between
    std::istringstream ss("1\n2\n\n\n3\n4\n\n\5\n6");

    CSVReader reader;
    reader.setFirstRowHeader(false);

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(2, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(6, dataframe->getNumberOfRows()) << "row count does not match";
}

TEST(CSVdata, emptyRowAtEnd) {
    // test for empty rows at the end
    std::istringstream ss("1\n2\n");

    CSVReader reader;
    reader.setFirstRowHeader(false);

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(2, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(2, dataframe->getNumberOfRows()) << "row count does not match";
}

TEST(CSVdata, emptyRowsAtEnd) {
    // test for empty rows at the end
    std::istringstream ss("1\n2\n\n\n\n");

    CSVReader reader;
    reader.setFirstRowHeader(false);

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(2, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(2, dataframe->getNumberOfRows()) << "row count does not match";
}

TEST(CSVdata, emptyFields) {
    // test for empty fields in both categorical and numeric columns
    std::istringstream ss(
        "Number,Name,Category\n"
        "1,Apple,Fruit\n"
        "2,Banana,\n"    // -> 2, "Banana", ""
        ",,Vegetable");  // -> nan, "", "Vegetable"

    CSVReader reader;
    reader.setFirstRowHeader(true);

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(4, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(3, dataframe->getNumberOfRows()) << "row count does not match";

    auto value = dataframe->getColumn(3)->get(1, true)->toString();
    EXPECT_EQ("", value) << "empty field at end of row";
    value = dataframe->getColumn(1)->get(2, false)->toString();
    EXPECT_EQ("nan", value) << "empty numeric field at begin of row";
    value = dataframe->getColumn(2)->get(2, true)->toString();
    EXPECT_EQ("", value) << "empty field in middle of row";
}

TEST(CSVdata, columnCountMismatch) {
    // test for rows with varying column counts
    std::istringstream ss("1,2,3\n4,5\n7,8,9");

    CSVReader reader;
    reader.setFirstRowHeader(false);

    EXPECT_THROW(reader.readData(ss), CSVDataReaderException);
}

TEST(CSVdata, columnCountMismatchWithHeader) {
    // test for rows with varying column counts
    std::istringstream ss("A,B,C\n1,2,3\n4,5\n7,8,9");

    CSVReader reader;
    reader.setFirstRowHeader(true);

    EXPECT_THROW(reader.readData(ss), CSVDataReaderException);
}

TEST(CSVdata, delimiterBeforeLineBreak) {
    // test whether delimiters before the end of line are ignored with no header
    std::istringstream ss("1,a,\n2,b,");
    std::istringstream ssEmptyLine("1,a,\n2,b,\n");

    CSVReader reader;
    reader.setFirstRowHeader(false);

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(4, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(2, dataframe->getNumberOfRows()) << "row count does not match";

    dataframe = reader.readData(ssEmptyLine);
    ASSERT_EQ(4, dataframe->getNumberOfColumns()) << "column count does not match (empty line)";
    ASSERT_EQ(2, dataframe->getNumberOfRows()) << "row count does not match (empty line)";
}

TEST(CSVdata, delimiterBeforeLineBreakWithHeader) {
    // test whether delimiters before the end of line are ignored with header
    std::istringstream ss("A,B\n1,a,\n2,b,");
    std::istringstream ssEmptyLine("A,B\n1,a,\n2,b,\n");

    CSVReader reader;
    reader.setFirstRowHeader(true);

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(3, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(2, dataframe->getNumberOfRows()) << "row count does not match";

    dataframe = reader.readData(ssEmptyLine);
    ASSERT_EQ(3, dataframe->getNumberOfColumns()) << "column count does not match (empty line)";
    ASSERT_EQ(2, dataframe->getNumberOfRows()) << "row count does not match (empty line)";
}

TEST(CSVdata, ignoreEmptyLine) {
    // test whether rows with only delimiters, i.e. ",,,,," are ignored
    std::istringstream ss("1,a,apple,fruit\n,,,\n,,,\n2,b,banana,fruit\n");

    CSVReader reader;
    reader.setFirstRowHeader(false);

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(5, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(2, dataframe->getNumberOfRows()) << "row count does not match";
}

TEST(CSVdata, byteOrderMark) {
    // Byte order mark should be detected and not treated as a value
    std::stringstream ss;
    ss << '\xef' << '\xbb' << '\xbf' << "1,2,3";

    CSVReader reader;
    reader.setFirstRowHeader(false);
    auto dataframe = reader.readData(ss);

    ASSERT_EQ(4, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(1, dataframe->getNumberOfRows()) << "row count does not match";
    auto value = dataframe->getColumn(1)->get(0, true)->toString();
    EXPECT_EQ("1", value) << "Column 1";
}

TEST(CSVheader, withHeader) {
    const std::string data = "1,2,3\n4,5,6";
    std::istringstream ss("First Col,Second Col,Third Col\n" + data);

    CSVReader reader;
    reader.setFirstRowHeader(true);
    auto dataframe = reader.readData(ss);

    // dataFrame should have 4 columns, three for data + 1 index
    ASSERT_EQ(4, dataframe->getNumberOfColumns()) << "column count does not match";
    EXPECT_EQ("First Col", dataframe->getColumn(1)->getHeader());
    EXPECT_EQ("Second Col", dataframe->getColumn(2)->getHeader());
    EXPECT_EQ("Third Col", dataframe->getColumn(3)->getHeader());
}

TEST(CSVheader, noHeader) {
    std::istringstream ss("1,2,3\n4,5,6");

    // test if default header names are created properly, i.e. "Column i"
    CSVReader reader;
    reader.setFirstRowHeader(false);
    auto dataframe = reader.readData(ss);

    ASSERT_EQ(4, dataframe->getNumberOfColumns()) << "column count does not match";
    EXPECT_EQ("Column 1", dataframe->getColumn(1)->getHeader());
    EXPECT_EQ("Column 2", dataframe->getColumn(2)->getHeader());
    EXPECT_EQ("Column 3", dataframe->getColumn(3)->getHeader());
}

TEST(CSVheader, missingHeader) {
    std::istringstream ss("1,2,3\n4,5,6");

    CSVReader reader;
    reader.setFirstRowHeader(true);
    auto dataframe = reader.readData(ss);

    // columns will use first data row as header
    ASSERT_EQ(4, dataframe->getNumberOfColumns()) << "column count does not match";
    EXPECT_EQ("1", dataframe->getColumn(1)->getHeader());
    EXPECT_EQ("2", dataframe->getColumn(2)->getHeader());
    EXPECT_EQ("3", dataframe->getColumn(3)->getHeader());
}

TEST(CSVdelimiter, comma) {
    // test for default delimiter ','
    std::istringstream ssComma("a,b,c");
    std::stringstream ssSemicolon("a;b;c");

    CSVReader reader;
    reader.setFirstRowHeader(false);

    auto dataframe = reader.readData(ssComma);
    ASSERT_EQ(4, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(1, dataframe->getNumberOfRows()) << "row count does not match";
    EXPECT_EQ("a", dataframe->getColumn(1)->get(0, true)->toString());
    EXPECT_EQ("b", dataframe->getColumn(2)->get(0, true)->toString());
    EXPECT_EQ("c", dataframe->getColumn(3)->get(0, true)->toString());

    dataframe = reader.readData(ssSemicolon);
    ASSERT_EQ(2, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(1, dataframe->getNumberOfRows()) << "row count does not match";
    auto value = dataframe->getColumn(1)->get(0, true)->toString();
    EXPECT_EQ("a;b;c", value);
}

TEST(CSVdelimiter, semicolon) {
    // test for ';'
    std::istringstream ssComma("a,b,c");
    std::stringstream ssSemicolon("a;b;c");

    CSVReader reader;
    reader.setDelimiters(";");
    reader.setFirstRowHeader(false);

    auto dataframe = reader.readData(ssSemicolon);
    ASSERT_EQ(4, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(1, dataframe->getNumberOfRows()) << "row count does not match";
    EXPECT_EQ("a", dataframe->getColumn(1)->get(0, true)->toString());
    EXPECT_EQ("b", dataframe->getColumn(2)->get(0, true)->toString());
    EXPECT_EQ("c", dataframe->getColumn(3)->get(0, true)->toString());

    dataframe = reader.readData(ssComma);
    ASSERT_EQ(2, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(1, dataframe->getNumberOfRows()) << "row count does not match";
    auto value = dataframe->getColumn(1)->get(0, true)->toString();
    EXPECT_EQ("a,b,c", value);
}

TEST(CSVdelimiter, multiple) {
    // test for multiple delimiters
    std::istringstream ss("a|b#3#4@5|6");
    std::stringstream ssSemicolon("a;b;c");

    CSVReader reader;
    reader.setDelimiters("|@#");
    reader.setFirstRowHeader(false);

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(7, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(1, dataframe->getNumberOfRows()) << "row count does not match";
    EXPECT_EQ("a", dataframe->getColumn(1)->get(0, true)->toString());
    EXPECT_EQ("b", dataframe->getColumn(2)->get(0, true)->toString());
    EXPECT_EQ("3", dataframe->getColumn(3)->get(0, false)->toString());
    EXPECT_EQ("4", dataframe->getColumn(4)->get(0, false)->toString());
    EXPECT_EQ("5", dataframe->getColumn(5)->get(0, false)->toString());
    EXPECT_EQ("6", dataframe->getColumn(6)->get(0, false)->toString());

    dataframe = reader.readData(ssSemicolon);
    ASSERT_EQ(2, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(1, dataframe->getNumberOfRows()) << "row count does not match";
    EXPECT_EQ("a;b;c", dataframe->getColumn(1)->get(0, true)->toString());
}

TEST(CSVquotes, singlequotes) {
    std::istringstream ss("\"first quote\",not quoted,\"b\"");

    CSVReader reader;
    reader.setFirstRowHeader(false);

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(4, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(1, dataframe->getNumberOfRows()) << "row count does not match";
    auto col1 = dataframe->getColumn(1)->get(0, true)->toString();
    EXPECT_EQ("\"first quote\"", col1);
    auto col2 = dataframe->getColumn(2)->get(0, true)->toString();
    EXPECT_EQ("not quoted", col2);
    auto col3 = dataframe->getColumn(3)->get(0, true)->toString();
    EXPECT_EQ("\"b\"", col3);
}

TEST(CVSquotes, headerquotes) {
    std::istringstream ss("\"first col\",second,\"third\"\n1,2,3");

    CSVReader reader;
    reader.setFirstRowHeader(true);

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(4, dataframe->getNumberOfColumns()) << "column count does not match";
    EXPECT_EQ("\"first col\"", dataframe->getColumn(1)->getHeader()) << "Column Header 1";
    EXPECT_EQ("second", dataframe->getColumn(2)->getHeader()) << "Column Header 2";
    EXPECT_EQ("\"third\"", dataframe->getColumn(3)->getHeader()) << "Column Header 3";
}

TEST(CSVquotes, trailingquote) {
    std::istringstream ss("\"trailing \"quote\",second,\"third\"\n1,2,3");

    CSVReader reader;
    reader.setFirstRowHeader(true);

    EXPECT_THROW(reader.readData(ss), inviwo::CSVDataReaderException) << "unmatched quote";
}

TEST(CSVquotes, trailingquoteAtEnd) {
    std::istringstream ss("\"first col\",second,\"third\n1,2,3");

    CSVReader reader;
    reader.setFirstRowHeader(true);

    EXPECT_THROW(reader.readData(ss), inviwo::CSVDataReaderException) << "unmatched quote";
}

TEST(CSVquotes, nestedquotes) {
    std::istringstream ss("\"first \"col\"\",second,\"third\"\n1,2,3");

    CSVReader reader;
    reader.setFirstRowHeader(true);

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(4, dataframe->getNumberOfColumns()) << "column count does not match";
    EXPECT_EQ("\"first \"col\"\"", dataframe->getColumn(1)->getHeader()) << "Column Header 1";
    EXPECT_EQ("second", dataframe->getColumn(2)->getHeader()) << "Column Header 2";
    EXPECT_EQ("\"third\"", dataframe->getColumn(3)->getHeader()) << "Column Header 3";
}

TEST(CSVquotes, multiline) {
    std::istringstream ss(
        "Col 1,Col 2, Col3\n"
        "\"multiline \n quote\",2,3");

    CSVReader reader;
    reader.setFirstRowHeader(true);

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(4, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(1, dataframe->getNumberOfRows()) << "row count does not match";
    auto value = dataframe->getColumn(1)->get(0, true)->toString();
    EXPECT_EQ("\"multiline \n quote\"", value) << "Column 1";
    value = dataframe->getColumn(2)->get(0, true)->toString();
    EXPECT_EQ("2", value) << "Column 2";
    value = dataframe->getColumn(3)->get(0, true)->toString();
    EXPECT_EQ("3", value) << "Column 3";
}

TEST(CSVlinebreaks, CRonly) {
    std::istringstream ss("1,2,3\r4,5,6\r7,8,9");

    CSVReader reader;
    reader.setFirstRowHeader(false);
    auto dataframe = reader.readData(ss);

    ASSERT_EQ(4, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(3, dataframe->getNumberOfRows()) << "row count does not match";
}

TEST(CSVlinebreaks, LFonly) {
    std::istringstream ss("1,2,3\n4,5,6\n7,8,9");

    CSVReader reader;
    reader.setFirstRowHeader(false);
    auto dataframe = reader.readData(ss);

    ASSERT_EQ(4, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(3, dataframe->getNumberOfRows()) << "row count does not match";
}

TEST(CSVlinebreaks, CRLF) {
    std::istringstream ss("1,2,3\r\n4,5,6\r\n7,8,9");

    CSVReader reader;
    reader.setFirstRowHeader(false);
    auto dataframe = reader.readData(ss);

    ASSERT_EQ(4, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(3, dataframe->getNumberOfRows()) << "row count does not match";
}

TEST(CSVlinebreaks, LFCR) {
    std::istringstream ss("1,2,3\n\r4,5,6\n\r7,8,9");

    CSVReader reader;
    reader.setFirstRowHeader(false);
    auto dataframe = reader.readData(ss);

    ASSERT_EQ(4, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(3, dataframe->getNumberOfRows()) << "row count does not match";
}

TEST(CSVlinebreaks, mixed) {
    std::istringstream ss("1,2,3\r4,5,6\n\r7,8,9\n10,11,12");

    CSVReader reader;
    reader.setFirstRowHeader(false);
    auto dataframe = reader.readData(ss);

    ASSERT_EQ(4, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(4, dataframe->getNumberOfRows()) << "row count does not match";
}

}  // namespace inviwo
