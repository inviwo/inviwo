/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2022 Inviwo Foundation
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
#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/core/io/datareaderexception.h>

#include <sstream>

namespace inviwo {

TEST(CSVnoData, stream) {
    std::istringstream ss("");

    CSVReader reader;
    EXPECT_THROW(reader.readData(ss), DataReaderException);
}

TEST(CSVnoData, file) {
    util::TempFileHandle tmpFile("", ".csv");

    CSVReader reader;
    EXPECT_THROW(reader.readData(tmpFile.getFileName()), DataReaderException);
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

    EXPECT_THROW(reader.readData(ss), DataReaderException);
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
    std::istringstream ss("1\n2\n  \n\n  \n");

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

    auto value = dataframe->getColumn(3)->getAsString(1);
    EXPECT_EQ("", value) << "empty field at end of row";
    value = dataframe->getColumn(1)->getAsString(2);
    EXPECT_EQ("0", value) << "empty numeric field at begin of row";
    value = dataframe->getColumn(2)->getAsString(2);
    EXPECT_EQ("", value) << "empty field in middle of row";
}

TEST(CSVdata, columnCountMismatch) {
    // test for rows with varying column counts
    std::istringstream ss("1,2,3\n4,5\n7,8,9");

    CSVReader reader;
    reader.setFirstRowHeader(false);

    EXPECT_THROW(reader.readData(ss), DataReaderException);
}

TEST(CSVdata, columnCountMismatchWithHeader) {
    // test for rows with varying column counts
    std::istringstream ss("A,B,C\n1,2,3\n4,5\n7,8,9");

    CSVReader reader;
    reader.setFirstRowHeader(true);

    EXPECT_THROW(reader.readData(ss), DataReaderException);
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

TEST(CSVdata, allEmptyElementLine) {
    // test whether rows with only delimiters, i.e. ",,,,," are kept
    std::istringstream ss("1,a,apple,fruit\n,,,\n,,,\n2,b,banana,fruit\n");

    CSVReader reader;
    reader.setFirstRowHeader(false);

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(5, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(4, dataframe->getNumberOfRows()) << "row count does not match";
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
    auto value = dataframe->getColumn(1)->getAsString(0);
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
    EXPECT_EQ("a", dataframe->getColumn(1)->getAsString(0));
    EXPECT_EQ("b", dataframe->getColumn(2)->getAsString(0));
    EXPECT_EQ("c", dataframe->getColumn(3)->getAsString(0));

    dataframe = reader.readData(ssSemicolon);
    ASSERT_EQ(2, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(1, dataframe->getNumberOfRows()) << "row count does not match";
    auto value = dataframe->getColumn(1)->getAsString(0);
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
    EXPECT_EQ("a", dataframe->getColumn(1)->getAsString(0));
    EXPECT_EQ("b", dataframe->getColumn(2)->getAsString(0));
    EXPECT_EQ("c", dataframe->getColumn(3)->getAsString(0));

    dataframe = reader.readData(ssComma);
    ASSERT_EQ(2, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(1, dataframe->getNumberOfRows()) << "row count does not match";
    auto value = dataframe->getColumn(1)->getAsString(0);
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
    EXPECT_EQ("a", dataframe->getColumn(1)->getAsString(0));
    EXPECT_EQ("b", dataframe->getColumn(2)->getAsString(0));
    EXPECT_EQ("3", dataframe->getColumn(3)->getAsString(0));
    EXPECT_EQ("4", dataframe->getColumn(4)->getAsString(0));
    EXPECT_EQ("5", dataframe->getColumn(5)->getAsString(0));
    EXPECT_EQ("6", dataframe->getColumn(6)->getAsString(0));

    dataframe = reader.readData(ssSemicolon);
    ASSERT_EQ(2, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(1, dataframe->getNumberOfRows()) << "row count does not match";
    EXPECT_EQ("a;b;c", dataframe->getColumn(1)->getAsString(0));
}

TEST(CSVquotes, singlequotes) {
    std::istringstream ss("\"first quote\",not quoted,\"b\"");

    CSVReader reader;
    reader.setFirstRowHeader(false);
    reader.setStripQuotes(false);

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(4, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(1, dataframe->getNumberOfRows()) << "row count does not match";
    auto col1 = dataframe->getColumn(1)->getAsString(0);
    EXPECT_EQ("\"first quote\"", col1);
    auto col2 = dataframe->getColumn(2)->getAsString(0);
    EXPECT_EQ("not quoted", col2);
    auto col3 = dataframe->getColumn(3)->getAsString(0);
    EXPECT_EQ("\"b\"", col3);
}

TEST(CSVquotes, stripSingleQuotes) {
    std::istringstream ss("\"first quote\",not quoted,\"b\"");

    CSVReader reader;
    reader.setFirstRowHeader(false);

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(4, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(1, dataframe->getNumberOfRows()) << "row count does not match";
    auto col1 = dataframe->getColumn(1)->getAsString(0);
    EXPECT_EQ("first quote", col1);
    auto col2 = dataframe->getColumn(2)->getAsString(0);
    EXPECT_EQ("not quoted", col2);
    auto col3 = dataframe->getColumn(3)->getAsString(0);
    EXPECT_EQ("b", col3);
}

TEST(CSVquotes, headerQuotes) {
    std::istringstream ss("\"first col\",second,\"third\"\n1,2,3");

    CSVReader reader;
    reader.setFirstRowHeader(true);
    reader.setStripQuotes(false);

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(4, dataframe->getNumberOfColumns()) << "column count does not match";
    EXPECT_EQ("\"first col\"", dataframe->getColumn(1)->getHeader()) << "Column Header 1";
    EXPECT_EQ("second", dataframe->getColumn(2)->getHeader()) << "Column Header 2";
    EXPECT_EQ("\"third\"", dataframe->getColumn(3)->getHeader()) << "Column Header 3";
}

TEST(CSVquotes, stripHeaderQuotes) {
    std::istringstream ss("\"first col\",second,\"third\"\n1,2,3");

    CSVReader reader;
    reader.setFirstRowHeader(true);

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(4, dataframe->getNumberOfColumns()) << "column count does not match";
    EXPECT_EQ("first col", dataframe->getColumn(1)->getHeader()) << "Column Header 1";
    EXPECT_EQ("second", dataframe->getColumn(2)->getHeader()) << "Column Header 2";
    EXPECT_EQ("third", dataframe->getColumn(3)->getHeader()) << "Column Header 3";
}

TEST(CSVquotes, trailingquote) {
    std::istringstream ss("\"trailing \"quote\",second,\"third\"\n1,2,3");

    CSVReader reader;
    reader.setFirstRowHeader(true);

    EXPECT_THROW(reader.readData(ss), DataReaderException) << "unmatched quote";
}

TEST(CSVquotes, trailingquoteAtEnd) {
    std::istringstream ss("\"first col\",second,\"third\n1,2,3");

    CSVReader reader;
    reader.setFirstRowHeader(true);

    EXPECT_THROW(reader.readData(ss), DataReaderException) << "unmatched quote";
}

TEST(CSVquotes, nestedquotes) {
    std::istringstream ss("\"first \"col\"\",second,\"third\"\n1,2,3");

    CSVReader reader;
    reader.setFirstRowHeader(true);
    reader.setStripQuotes(false);

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
    reader.setStripQuotes(false);

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(4, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(1, dataframe->getNumberOfRows()) << "row count does not match";
    auto value = dataframe->getColumn(1)->getAsString(0);
    EXPECT_EQ("\"multiline \n quote\"", value) << "Column 1";
    value = dataframe->getColumn(2)->getAsString(0);
    EXPECT_EQ("2", value) << "Column 2";
    value = dataframe->getColumn(3)->getAsString(0);
    EXPECT_EQ("3", value) << "Column 3";
}

TEST(CSVquotes, stripMultiline) {
    std::istringstream ss(
        "Col 1,Col 2, Col3\n"
        "\"multiline \n quote\",2,3");

    CSVReader reader;
    reader.setFirstRowHeader(true);

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(4, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(1, dataframe->getNumberOfRows()) << "row count does not match";
    auto value = dataframe->getColumn(1)->getAsString(0);
    EXPECT_EQ("multiline \n quote", value) << "Column 1";
    value = dataframe->getColumn(2)->getAsString(0);
    EXPECT_EQ("2", value) << "Column 2";
    value = dataframe->getColumn(3)->getAsString(0);
    EXPECT_EQ("3", value) << "Column 3";
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
    std::istringstream ss("1,2,3\r\n4,5,6\n\r7,8,9\n10,11,12");

    CSVReader reader;
    reader.setFirstRowHeader(false);
    auto dataframe = reader.readData(ss);

    ASSERT_EQ(4, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(4, dataframe->getNumberOfRows()) << "row count does not match";
}

TEST(CSVFilters, comments) {
    std::istringstream ss("# comment\n1,2,3\n# 4,5,6\n7,8,9\n# foo");

    const std::string comment = "#";
    csvfilters::Filters filters;
    filters.excludeRows.push_back(csvfilters::rowBegin(comment, false));

    CSVReader reader;
    reader.setFirstRowHeader(false);
    reader.setFilters(filters);
    auto dataframe = reader.readData(ss);

    // columns will use first data row as header
    ASSERT_EQ(4, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(2, dataframe->getNumberOfRows()) << "row count does not match";
    EXPECT_EQ("1", dataframe->getColumn(1)->getAsString(0));
    EXPECT_EQ("7", dataframe->getColumn(1)->getAsString(1));
}

TEST(CSVFilters, commentsBeforeHeaders) {
    std::istringstream ss("#,header,comment\n1,2,3\n# 4,5,6\n7,8,9\n# foo");

    const std::string comment = "#";
    csvfilters::Filters filters;
    filters.excludeRows.push_back(csvfilters::rowBegin(comment, true));

    CSVReader reader;
    reader.setFirstRowHeader(true);
    reader.setFilters(filters);
    auto dataframe = reader.readData(ss);

    // columns will use first data row as header
    ASSERT_EQ(4, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(1, dataframe->getNumberOfRows()) << "row count does not match";
    EXPECT_EQ("1", dataframe->getColumn(1)->getHeader());
    EXPECT_EQ("7", dataframe->getColumn(1)->getAsString(0));
}

TEST(CSVFilters, keepLines) {
    std::istringstream ss("1,2,3\n4,5,6\n7,8,9\n");

    csvfilters::Filters filters;
    filters.includeRows.push_back(
        {[](std::string_view, size_t line) { return (line < 2) || (line > 2); }, true});

    CSVReader reader;
    reader.setFirstRowHeader(false);
    reader.setFilters(filters);
    auto dataframe = reader.readData(ss);

    // columns will use first data row as header
    ASSERT_EQ(4, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(2, dataframe->getNumberOfRows()) << "row count does not match";
    EXPECT_EQ("1", dataframe->getColumn(1)->getAsString(0));
    EXPECT_EQ("7", dataframe->getColumn(1)->getAsString(1));
}
TEST(CSVFilters, emptyRows) {
    // test for empty rows in between
    std::istringstream ss("1\n2\n\n\n3\n4\n\n\5\n6");

    csvfilters::Filters filters;
    filters.excludeRows.push_back(csvfilters::emptyLines());

    CSVReader reader;
    reader.setFilters(filters);
    reader.setFirstRowHeader(false);

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(2, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(6, dataframe->getNumberOfRows()) << "row count does not match";
}

TEST(CSVFilters, emptyRowsAtEnd) {
    // test for empty rows at the end
    std::istringstream ss("1\n2\n\n\n\n");

    csvfilters::Filters filters;
    filters.excludeRows.push_back(csvfilters::emptyLines());

    CSVReader reader;
    reader.setFilters(filters);
    reader.setFirstRowHeader(false);

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(2, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(2, dataframe->getNumberOfRows()) << "row count does not match";
}

TEST(CSVItemFilters, stringMatchEqual) {
    std::istringstream ss("a,1\nb,2\nc,3\na,4");

    csvfilters::Filters filters;
    filters.includeItems.push_back(csvfilters::stringMatch(0, filters::StringComp::Equal, "a"));

    CSVReader reader;
    reader.setFilters(filters);
    reader.setFirstRowHeader(false);

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(3, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(2, dataframe->getNumberOfRows()) << "row count does not match";

    EXPECT_EQ("a", dataframe->getColumn(1)->getAsString(0));
    EXPECT_EQ("a", dataframe->getColumn(1)->getAsString(1));

    const std::vector<int> expected = {1, 4};

    auto bufferram = static_cast<const BufferRAMPrecision<int>*>(
        dataframe->getColumn(2)->getBuffer()->getRepresentation<BufferRAM>());
    EXPECT_EQ(expected, bufferram->getDataContainer()) << "Row contents incorrect";
}

TEST(CSVItemFilters, stringMatchRegex) {
    std::istringstream ss("apple\napples\nbanana\napple");

    csvfilters::Filters filters;
    filters.includeItems.push_back(csvfilters::stringMatch(0, filters::StringComp::Regex, ".p+l."));

    CSVReader reader;
    reader.setFilters(filters);
    reader.setFirstRowHeader(false);

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(2, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(2, dataframe->getNumberOfRows()) << "row count does not match";

    EXPECT_EQ("apple", dataframe->getColumn(1)->getAsString(0));
    EXPECT_EQ("apple", dataframe->getColumn(1)->getAsString(1));
}

TEST(CSVItemFilters, stringMatchPartialRegex) {
    std::istringstream ss("apple\napples\nbanana\napple");

    csvfilters::Filters filters;
    filters.includeItems.push_back(
        csvfilters::stringMatch(0, filters::StringComp::RegexPartial, "p+l"));

    CSVReader reader;
    reader.setFilters(filters);
    reader.setFirstRowHeader(false);

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(2, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(3, dataframe->getNumberOfRows()) << "row count does not match";

    EXPECT_EQ("apple", dataframe->getColumn(1)->getAsString(0));
    EXPECT_EQ("apples", dataframe->getColumn(1)->getAsString(1));
    EXPECT_EQ("apple", dataframe->getColumn(1)->getAsString(2));
}

TEST(CSVItemFilters, intMatch) {
    std::istringstream ss("1\n2\n3\n2\n4");

    csvfilters::Filters filters;
    filters.includeItems.push_back(csvfilters::intMatch(0, filters::NumberComp::Less, 3));

    CSVReader reader;
    reader.setFilters(filters);
    reader.setFirstRowHeader(false);

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(2, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(3, dataframe->getNumberOfRows()) << "row count does not match";

    const std::vector<int> expected = {1, 2, 2};

    auto bufferram = static_cast<const BufferRAMPrecision<int>*>(
        dataframe->getColumn(1)->getBuffer()->getRepresentation<BufferRAM>());
    EXPECT_EQ(expected, bufferram->getDataContainer()) << "Row contents incorrect";
}

TEST(CSVItemFilters, intRange) {
    std::istringstream ss("10\n2\n3\n2\n4\n20\n8");

    csvfilters::Filters filters;
    filters.includeItems.push_back(csvfilters::intRange(0, 3, 9));

    CSVReader reader;
    reader.setFilters(filters);
    reader.setFirstRowHeader(false);

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(2, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(3, dataframe->getNumberOfRows()) << "row count does not match";

    const std::vector<int> expected = {3, 4, 8};

    auto bufferram = static_cast<const BufferRAMPrecision<int>*>(
        dataframe->getColumn(1)->getBuffer()->getRepresentation<BufferRAM>());
    EXPECT_EQ(expected, bufferram->getDataContainer()) << "Row contents incorrect";
}

TEST(CSVItemFilters, doubleMatch) {
    std::istringstream ss("3.2\n3.3\n1.5\n4.5\n10.0");

    csvfilters::Filters filters;
    filters.includeItems.push_back(
        csvfilters::doubleMatch(0, filters::NumberComp::Equal, 3.25, 0.06));

    CSVReader reader;
    reader.setFilters(filters);
    reader.setFirstRowHeader(false);
    reader.setEnableDoublePrecision(true);

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(2, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(2, dataframe->getNumberOfRows()) << "row count does not match";

    const std::vector<double> expected = {3.2, 3.3};

    auto bufferram = static_cast<const BufferRAMPrecision<double>*>(
        dataframe->getColumn(1)->getBuffer()->getRepresentation<BufferRAM>());
    EXPECT_EQ(expected, bufferram->getDataContainer()) << "Row contents incorrect";
}

TEST(CSVItemFilters, doubleRange) {
    std::istringstream ss("3.2\n3.3\n1.5\n4.5\n10.0");

    csvfilters::Filters filters;
    filters.includeItems.push_back(csvfilters::doubleRange(0, 3.3, 5.0));

    CSVReader reader;
    reader.setFilters(filters);
    reader.setFirstRowHeader(false);
    reader.setEnableDoublePrecision(true);

    auto dataframe = reader.readData(ss);
    ASSERT_EQ(2, dataframe->getNumberOfColumns()) << "column count does not match";
    ASSERT_EQ(2, dataframe->getNumberOfRows()) << "row count does not match";

    const std::vector<double> expected = {3.3, 4.5};

    auto bufferram = static_cast<const BufferRAMPrecision<double>*>(
        dataframe->getColumn(1)->getBuffer()->getRepresentation<BufferRAM>());
    EXPECT_EQ(expected, bufferram->getDataContainer()) << "Row contents incorrect";
}

}  // namespace inviwo
