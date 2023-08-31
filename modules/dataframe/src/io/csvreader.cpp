/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2023 Inviwo Foundation
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

#include <inviwo/dataframe/io/csvreader.h>

#include <inviwo/core/datastructures/buffer/buffer.h>                   // for Buffer
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/unitsystem.h>                      // for Unit
#include <inviwo/core/io/datareader.h>                                  // for DataReaderType
#include <inviwo/core/io/datareaderexception.h>                         // for DataReaderException
#include <inviwo/core/util/detected.h>                                  // for alwaysFalse
#include <inviwo/core/util/fileextension.h>                             // for FileExtension
#include <inviwo/core/util/filesystem.h>                                // for skipByteOrderMark
#include <inviwo/core/util/logcentral.h>                                // for LogCentral, LogWarn
#include <inviwo/core/util/raiiutils.h>                                 // for OnScopeExit, OnSc...
#include <inviwo/core/util/safecstr.h>                                  // for SafeCStr
#include <inviwo/core/util/sourcecontext.h>                             // for IVW_CONTEXT_CUSTOM
#include <inviwo/core/util/stdextensions.h>                             // for overloaded
#include <inviwo/core/util/stringconversion.h>                          // for trim
#include <inviwo/core/util/zip.h>                                       // for zipIterator, zipper
#include <inviwo/dataframe/datastructures/column.h>                     // for CategoricalColumn...
#include <inviwo/dataframe/datastructures/dataframe.h>                  // for DataFrame
#include <inviwo/dataframe/util/filters.h>                              // for Filters, ItemFilter

#include <algorithm>      // for any_of, none_of
#include <charconv>       // for from_chars
#include <clocale>        // for setlocale, LC_ALL
#include <cstdint>        // for int64_t
#include <cstdlib>        // for size_t, strtod
#include <fstream>        // for char_traits, basi...
#include <functional>     // for function, __base
#include <iterator>       // for istreambuf_iterator
#include <limits>         // for numeric_limits
#include <optional>       // for optional, nullopt
#include <regex>          // for regex_match, smatch
#include <sstream>        // for basic_stringbuf<>...
#include <system_error>   // for errc
#include <type_traits>    // for remove_reference<...
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set
#include <variant>        // for visit
#include <cerrno>         // for errno

#include <fmt/core.h>  // for format
#include <fmt/std.h>
#include <glm/gtc/type_ptr.hpp>  // for value_ptr
#include <llnl-units/units.hpp>       // for unit_from_string

namespace inviwo {

namespace config {
#if defined(__cpp_lib_to_chars) && __cpp_lib_to_chars >= 201611L
constexpr bool charconv = true;
#else
constexpr bool charconv = false;
#endif
}  // namespace config

CSVReader::CSVReader(std::string_view delim, bool hasHeader, bool doublePrecision)
    : DataReaderType<DataFrame>()
    , delimiters_(delim)
    , stripQuotes_{defaultStripQuotes}
    , firstRowHeader_(hasHeader)
    , firstColIndices_{defaultFirstColIndices}
    , unitsInHeaders_(defaultUnitInHeaders)
    , unitRegexp_(defaultUnitRegexp)
    , doublePrecision_(doublePrecision)
    , exampleRows_{defaultNumberOfExampleRows}
    , locale_{defaultLocale}
    , emptyField_{defaultEmptyField} {
    addExtension(FileExtension("csv", "Comma Separated Values"));
}

CSVReader* CSVReader::clone() const { return new CSVReader(*this); }

CSVReader& CSVReader::setDelimiters(const std::string& delim) {
    delimiters_ = delim;
    return *this;
}

const std::string& CSVReader::getDelimiters() const { return delimiters_; }

CSVReader& CSVReader::setStripQuotes(bool stripQuotes) {
    stripQuotes_ = stripQuotes;
    return *this;
}
bool CSVReader::getStripQuotes() const { return stripQuotes_; }

CSVReader& CSVReader::setFirstRowHeader(bool hasHeader) {
    firstRowHeader_ = hasHeader;
    return *this;
}
bool CSVReader::hasFirstRowHeader() const { return firstRowHeader_; }

CSVReader& CSVReader::setFirstColIndices(bool hasIndices) {
    firstColIndices_ = hasIndices;
    return *this;
}
bool CSVReader::hasFirstColIndices() const { return firstColIndices_; }

CSVReader& CSVReader::setUnitsInHeaders(bool unitsInHeaders) {
    unitsInHeaders_ = unitsInHeaders;
    return *this;
}
bool CSVReader::hasUnitsInHeaders() const { return unitsInHeaders_; }

CSVReader& CSVReader::setUnitRegexp(std::string_view regexp) {
    unitRegexp_ = regexp;
    return *this;
}
const std::string& CSVReader::getUnitRegexp() const { return unitRegexp_; }

CSVReader& CSVReader::setEnableDoublePrecision(bool doubleprec) {
    doublePrecision_ = doubleprec;
    return *this;
}
bool CSVReader::hasDoublePrecision() const { return doublePrecision_; }

CSVReader& CSVReader::setNumberOfExampleRows(size_t rows) {
    exampleRows_ = rows;
    return *this;
}
size_t CSVReader::getNumberOfExamplesRows() const { return exampleRows_; }

CSVReader& CSVReader::setLocale(std::string_view loc) {
    locale_ = loc;
    return *this;
}
const std::string& CSVReader::getLocale() const { return locale_; }

CSVReader& CSVReader::setFilters(const csvfilters::Filters& filters) {
    filters_ = filters;
    return *this;
}
const csvfilters::Filters& CSVReader::getFilters() const { return filters_; }

CSVReader& CSVReader::setHandleEmptyFields(EmptyField emptyField) {
    emptyField_ = emptyField;
    return *this;
}
CSVReader::EmptyField CSVReader::getHandleEmptyFields() const { return emptyField_; }

bool CSVReader::setOption(std::string_view key, std::any value) {
    if (auto* delimiters = std::any_cast<std::string>(&value); delimiters && key == "Delimiters") {
        setDelimiters(*delimiters);
        return true;
    } else if (auto* stripQuotes = std::any_cast<bool>(&value);
               stripQuotes && key == "StripQuotes") {
        setStripQuotes(*stripQuotes);
        return true;
    } else if (auto* firstRowHeader = std::any_cast<bool>(&value);
               firstRowHeader && key == "FirstRowHeader") {
        setFirstRowHeader(*firstRowHeader);
        return true;
    } else if (auto* firstColIndices = std::any_cast<bool>(&value);
               firstColIndices && key == "FirstColIndices") {
        setFirstColIndices(*firstColIndices);
        return true;
    } else if (auto* unitsInHeaders = std::any_cast<bool>(&value);
               unitsInHeaders && key == "UnitsInHeaders") {
        setUnitsInHeaders(*unitsInHeaders);
        return true;
    } else if (auto* unitRegexp = std::any_cast<std::string>(&value);
               unitRegexp && key == "UnitRegexp") {
        setUnitRegexp(*unitRegexp);
        return true;
    } else if (auto* doublePrecision = std::any_cast<bool>(&value);
               doublePrecision && key == "DoublePrecision") {
        setEnableDoublePrecision(*doublePrecision);
        return true;
    } else if (auto* rows = std::any_cast<size_t>(&value); rows && key == "NumberOfExampleRows") {
        setNumberOfExampleRows(*rows);
        return true;
    } else if (auto* locale = std::any_cast<std::string>(&value); locale && key == "Locale") {
        setLocale(*locale);
        return true;
    } else if (auto* emptyField = std::any_cast<EmptyField>(&value);
               emptyField && key == "HandleEmptyFields") {
        setHandleEmptyFields(*emptyField);
        return true;
    } else if (auto* filters = std::any_cast<csvfilters::Filters>(&value);
               filters && key == "Filters") {
        setFilters(*filters);
        return true;
    }

    return false;
}

std::any CSVReader::getOption(std::string_view key) {
    if (key == "Delimiters") {
        return getDelimiters();
    } else if (key == "StripQuotes") {
        return getStripQuotes();
    } else if (key == "FirstRowHeader") {
        return hasFirstRowHeader();
    } else if (key == "FirstColIndices") {
        return hasFirstColIndices();
    } else if (key == "UnitsInHeaders") {
        return hasUnitsInHeaders();
    } else if (key == "UnitRegexp") {
        return getUnitRegexp();
    } else if (key == "DoublePrecision") {
        return hasDoublePrecision();
    } else if (key == "NumberOfExampleRows") {
        return getNumberOfExamplesRows();
    } else if (key == "Locale") {
        return getLocale();
    } else if (key == "HandleEmptyFields") {
        return getHandleEmptyFields();
    } else if (key == "Filters") {
        return getFilters();
    }
    return std::any{};
}

std::shared_ptr<DataFrame> CSVReader::readData(const std::filesystem::path& fileName) {
    auto file = open(fileName);

    file.seekg(0, std::ios::end);
    std::streampos len = file.tellg();
    file.seekg(0, std::ios::beg);

    if (len == std::streampos(0)) {
        throw DataReaderException(IVW_CONTEXT, "Emtpy file: {}", fileName);
    }

    return readData(file);
}

namespace util {

template <typename Func>
size_t parse(std::string_view str, std::string_view delimiters, std::optional<size_t> expectedParts,
             std::optional<size_t> refLine, Func&& func) {

    std::string lookFor{delimiters};
    lookFor.push_back('"');  // We also have to handle quotes
    size_t part = 1;
    size_t index = 0;

    for (size_t first = 0; first < str.size();) {
        size_t second = first;
        size_t startPart = part;
        bool quoted = false;
        while (true) {
            second = str.find_first_of(lookFor, second);
            if (second != std::string_view::npos && str[second] == '"') {
                quoted = !quoted;
                ++second;
            } else if (quoted && second != std::string_view::npos) {
                ++second;
                ++part;
            } else if (!quoted) {
                ++part;
                break;
            } else {
                const auto ref = refLine ? *refLine : startPart;
                throw DataReaderException(IVW_CONTEXT_CUSTOM("CSVReader"),
                                          "Detected unmatched quote starting on line: {}", ref);
            }
        }

        if (expectedParts && index >= *expectedParts) {
            const auto ref = refLine ? *refLine : startPart;
            throw DataReaderException(IVW_CONTEXT_CUSTOM("CSVReader"),
                                      "Extra columns on line {}, expected {} found {}", ref,
                                      *expectedParts, index + 1);
        }

        const auto cell = util::trim(str.substr(first, second - first));
        std::invoke(func, cell, index, startPart);
        ++index;

        if (second == std::string_view::npos) break;
        first = second + 1;
    }

    // Handle the trailing delimiter with empty data
    if (!str.empty() && delimiters.find(str.back()) != std::string_view::npos) {
        // Ignore them if we don't expect them.
        if ((expectedParts && index < *expectedParts) || !expectedParts) {
            std::invoke(func, std::string_view{}, index, part);
            ++index;
        }
    }

    if (expectedParts && index < *expectedParts) {
        const auto ref = refLine ? *refLine : 0;
        throw DataReaderException(IVW_CONTEXT_CUSTOM("CSVReader"),
                                  "Missing columns on line {}, expected {} found {}", ref,
                                  *expectedParts, index);
    }

    return index;
}

namespace config {
#if defined(__cpp_lib_to_chars) && __cpp_lib_to_chars >= 201611L
constexpr bool charconv = true;
#else
constexpr bool charconv = false;
#endif
}  // namespace config

template <typename T>
std::optional<T> toNumberLocale(std::string_view str) {
    SafeCStr cstr{str};
    if constexpr (std::is_same_v<T, double>) {
        char* end = nullptr;
        typename std::remove_reference<decltype(errno)>::type errno_save = errno;
        errno = 0;
        double val = strtod(cstr.c_str(), &end);
        std::swap(errno, errno_save);
        if (errno_save == 0 && static_cast<size_t>(end - cstr.c_str()) == str.size()) {
            return val;
        } else {
            return std::nullopt;
        }
    } else if constexpr (std::is_same_v<T, float>) {
        char* end = nullptr;
        typename std::remove_reference<decltype(errno)>::type errno_save = errno;
        errno = 0;
        float val = strtof(cstr.c_str(), &end);
        std::swap(errno, errno_save);

        if (errno_save == 0 && static_cast<size_t>(end - cstr.c_str()) == str.size()) {
            return val;
        } else {
            return std::nullopt;
        }
    } else if constexpr (std::is_same_v<T, int>) {
        char* end = nullptr;
        typename std::remove_reference<decltype(errno)>::type errno_save = errno;
        errno = 0;
        long val = strtol(cstr.c_str(), &end, 10);
        std::swap(errno, errno_save);

        if (errno_save == 0 && static_cast<size_t>(end - cstr.c_str()) == str.size()) {
            if (val >= std::numeric_limits<int>::min() && val <= std::numeric_limits<int>::max()) {
                return static_cast<int>(val);
            } else {
                return std::nullopt;
            }
        } else {
            return std::nullopt;
        }
    } else if constexpr (std::is_same_v<T, std::uint32_t>) {
        char* end = nullptr;
        typename std::remove_reference<decltype(errno)>::type errno_save = errno;
        errno = 0;
        unsigned long val = strtoul(cstr.c_str(), &end, 10);
        std::swap(errno, errno_save);

        if (errno_save == 0 && static_cast<size_t>(end - cstr.c_str()) == str.size()) {
            if (val >= std::numeric_limits<std::uint32_t>::min() &&
                val <= std::numeric_limits<std::uint32_t>::max()) {
                return static_cast<std::uint32_t>(val);
            } else {
                return std::nullopt;
            }
        } else {
            return std::nullopt;
        }
    } else if constexpr (std::is_same_v<T, std::int64_t>) {
        char* end = nullptr;
        typename std::remove_reference<decltype(errno)>::type errno_save = errno;
        errno = 0;
        std::int64_t val = strtoll(cstr.c_str(), &end, 10);
        std::swap(errno, errno_save);

        if (errno_save == 0 && static_cast<size_t>(end - cstr.c_str()) == str.size()) {
            return val;
        } else {
            return std::nullopt;
        }
    } else {
        static_assert(util::alwaysFalse<T>(), "Unsupported type");
    }
}

template <typename T>
std::optional<T> toNumber(std::string_view str, bool cLocale) {
    if constexpr (config::charconv || !std::is_floating_point_v<T>) {
        if (!cLocale) return toNumberLocale<T>(str);

        T val;
        auto res = std::from_chars(str.data(), str.data() + str.size(), val);
        if (res.ec == std::errc() && res.ptr == (str.data() + str.size())) {
            return val;
        } else {
            return std::nullopt;
        }
    } else {
        return toNumberLocale<T>(str);
    }
}

template <typename T>
bool isNumber(std::string_view str, bool cLocale) {
    return util::toNumber<T>(str, cLocale).has_value();
}

std::string_view stripQuotes(std::string_view str) {
    // remove surrounding quotes
    if (str.size() >= 2 && str.front() == '"' && str.back() == '"') {
        str = str.substr(1, str.size() - 2);
    }
    return str;
};

}  // namespace util

std::vector<CSVReader::TypeCounts> CSVReader::findCellTypes(
    size_t nCol, const std::vector<std::pair<std::string_view, size_t>>& rows,
    size_t sampleRows) const {
    size_t sampledRows = sampleRows;
    std::vector<TypeCounts> counts(nCol);

    const bool cLocale = locale_ == "C";

    for (auto [i, row] : util::enumerate(rows)) {
        util::parse(row.first, delimiters_, nCol, row.second,
                    [&](std::string_view cell, size_t index, [[maybe_unused]] size_t part) {
                        if (cell.empty()) {
                            // Ignore empty cells.
                        } else if (util::isNumber<int>(cell, cLocale)) {
                            ++counts[index].integer;
                        } else if (doublePrecision_ ? util::isNumber<double>(cell, cLocale)
                                                    : util::isNumber<float>(cell, cLocale)) {
                            ++counts[index].real;
                        } else {
                            ++counts[index].string;
                        }
                    });
        if (i > sampledRows) {
            if (std::any_of(counts.begin(), counts.end(), [](const TypeCounts& type) {
                    return type.integer == 0 && type.real == 0 && type.string == 0;
                })) {
                sampledRows *= 2;
            } else {
                break;
            }
        }
    }
    if (sampledRows != sampleRows) {
        LogWarn(
            "Could not find any data for some columns, sampled more rows to determine column "
            "types "
            << sampledRows);
    }

    return counts;
}

template <typename T, bool index = false>
std::function<void(std::string_view, size_t, size_t)> addColumn(DataFrame& df,
                                                                std::string_view header, Unit unit,
                                                                CSVReader::EmptyField emptyField,
                                                                bool cLocale) {
    auto& data = [&]() -> decltype(auto) {
        if constexpr (index && std::is_same_v<T, std::uint32_t>) {
            df.getIndexColumn()->setHeader(header);
            df.getIndexColumn()->setUnit(unit);
            return df.getIndexColumn()
                ->getTypedBuffer()
                ->getEditableRAMRepresentation()
                ->getDataContainer();
        } else {
            return df.addColumn<T>(header, 0, unit)
                ->getTypedBuffer()
                ->getEditableRAMRepresentation()
                ->getDataContainer();
        }
    }();
    return [&data, cLocale, emptyField](std::string_view str, size_t line, size_t col) {
        if (str.empty()) {
            switch (emptyField) {
                case CSVReader::EmptyField::Throw:
                    throw DataReaderException(IVW_CONTEXT_CUSTOM("CSVReader"),
                                              "Empty field on line {}, column {}", line, col);
                case CSVReader::EmptyField::NanOrZero:
                    if constexpr (std::is_floating_point_v<T>) {
                        data.push_back(std::numeric_limits<T>::quiet_NaN());
                    } else {
                        data.emplace_back();
                    }
                    break;
                case CSVReader::EmptyField::EmptyOrZero:
                    data.emplace_back();
                    break;
                default:
                    data.emplace_back();
                    break;
            }
        } else if (auto val = util::toNumber<T>(str, cLocale)) {
            data.push_back(*val);
        } else {
            throw DataReaderException(IVW_CONTEXT_CUSTOM("CSVReader"),
                                      "Invalid format on line {}, column {}", line, col);
        }
    };
}

std::vector<std::function<void(std::string_view, size_t, size_t)>> CSVReader::addColumns(
    DataFrame& df, const std::vector<TypeCounts>& typeCounts,
    const std::vector<std::string>& headers) const {

    const bool cLocale = locale_ == "C";
    std::regex re{unitRegexp_};
    std::smatch m;

    std::vector<std::function<void(std::string_view, size_t, size_t)>> appenders;
    for (auto&& [counts, header] : util::zip(typeCounts, headers)) {
        auto headerCopy = header;
        Unit unit{};

        if (unitsInHeaders_ && counts.string == 0) {
            if (std::regex_match(header, m, re)) {
                headerCopy = util::trim(m.str(1));
                unit = units::unit_from_string(m.str(2));
            }
        }

        if (counts.index) {
            appenders.push_back(addColumn<std::uint32_t, true>(
                df, headerCopy, unit, CSVReader::EmptyField::Throw, cLocale));
        } else if (stripQuotes_ && counts.string > 0) {
            auto col = df.addCategoricalColumn(header);
            appenders.emplace_back([f = col->addMany()](std::string_view str, size_t, size_t) {
                f(util::stripQuotes(str));
            });
        } else if (counts.string > 0) {
            auto col = df.addCategoricalColumn(header);
            appenders.emplace_back(
                [f = col->addMany()](std::string_view str, size_t, size_t) { f(str); });
        } else if (doublePrecision_ && counts.real > 0) {
            appenders.push_back(addColumn<double>(df, headerCopy, unit, emptyField_, cLocale));
        } else if (!doublePrecision_ && counts.real > 0) {
            appenders.push_back(addColumn<float>(df, headerCopy, unit, emptyField_, cLocale));
        } else if (counts.integer > 0) {
            appenders.push_back(addColumn<int>(df, headerCopy, unit, emptyField_, cLocale));
        } else if (stripQuotes_) {
            auto col = df.addCategoricalColumn(header);
            appenders.emplace_back([f = col->addMany()](std::string_view str, size_t, size_t) {
                f(util::stripQuotes(str));
            });
        } else {
            auto col = df.addCategoricalColumn(header);
            appenders.emplace_back(
                [f = col->addMany()](std::string_view str, size_t, size_t) { f(str); });
        }
    }

    return appenders;
}

bool CSVReader::skipRow(std::string_view row, size_t lineNumber, bool filterOnHeader) const {

    auto filterRow = [&](bool neutralValue) {
        return [&, neutral = neutralValue](const auto& f) {
            if (!firstRowHeader_ || (f.filterOnHeader == filterOnHeader)) {
                return f.filter(row, lineNumber);
            }
            return neutral;
        };
    };

    auto filterItems = [&](const std::vector<csvfilters::ItemFilter>& filters, bool neutralValue) {
        const bool cLocale = locale_ == "C";
        bool retval = false;
        util::parse(
            row, delimiters_, std::nullopt, lineNumber,
            [&](std::string_view cell, size_t colIndex, [[maybe_unused]] size_t part) {
                auto test = util::overloaded{
                    [&](const std::function<bool(std::string_view)>& func) { return func(cell); },
                    [&](const std::function<bool(std::int64_t)>& func) {
                        if (auto val = util::toNumber<std::int64_t>(cell, cLocale)) {
                            return func(*val);
                        }
                        return neutralValue;
                    },
                    [&](const std::function<bool(double)>& func) {
                        if (auto val = util::toNumber<double>(cell, cLocale)) {
                            return func(*val);
                        }
                        return neutralValue;
                    }};
                for (const auto& f : filters) {
                    if ((!firstRowHeader_ || (f.filterOnHeader == filterOnHeader)) &&
                        f.column == static_cast<int>(colIndex)) {
                        retval |= std::visit(test, f.filter);
                    }
                }
            });
        return retval;
    };

    auto filterApplicable = [&](auto& filters) {
        return !filters.empty() &&
               (!firstRowHeader_ || std::any_of(filters.begin(), filters.end(), [&](auto& f) {
                   return (f.filterOnHeader == filterOnHeader);
               }));
    };

    if (filterApplicable(filters_.includeRows) &&
        std::none_of(filters_.includeRows.begin(), filters_.includeRows.end(), filterRow(true))) {
        return true;
    }
    if (std::any_of(filters_.excludeRows.begin(), filters_.excludeRows.end(), filterRow(false))) {
        return true;
    }

    if (filterApplicable(filters_.includeItems) && !filterItems(filters_.includeItems, true)) {
        return true;
    }
    if (filterItems(filters_.excludeItems, false)) {
        return true;
    }

    return false;
}

std::shared_ptr<DataFrame> CSVReader::readData(std::istream& stream) const {
    filesystem::skipByteOrderMark(stream);

    util::OnScopeExit cleanup{nullptr};
    if (!config::charconv || locale_ != "C") {
        // We need to use the C locale here to force use of decimal "."
        auto prevLocale = std::setlocale(LC_ALL, nullptr);
        std::string prev{prevLocale ? prevLocale : ""};
        if (!std::setlocale(LC_ALL, locale_.c_str())) {
            LogWarn("Failed to set locale " << locale_);
        }
        cleanup.setAction([prev]() { std::setlocale(LC_ALL, prev.c_str()); });
    }

    std::string content{std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
    std::string_view trimmed{content};
    if (auto pos = trimmed.find_last_not_of(" \f\n\r\t\v"); pos != std::string_view::npos) {
        trimmed = trimmed.substr(0, pos + 1);
    }

    std::vector<std::pair<std::string_view, size_t>> rows;
    util::parse(trimmed, "\n", std::nullopt, std::nullopt,
                [&](std::string_view line, [[maybe_unused]] size_t index, size_t lineNumber) {
                    if (!skipRow(line, lineNumber, true)) {
                        rows.emplace_back(line, lineNumber);
                    }
                });

    if (rows.empty()) {
        throw DataReaderException("No data", IVW_CONTEXT);
    }

    // extract first row
    std::vector<std::string> headers;
    util::parse(rows.front().first, delimiters_, std::nullopt, rows.front().second,
                [&](std::string_view cell, [[maybe_unused]] size_t index,
                    [[maybe_unused]] size_t partNumber) {
                    headers.emplace_back(stripQuotes_ ? util::stripQuotes(cell) : cell);
                    return true;
                });

    if (firstRowHeader_) {
        rows.erase(rows.begin());
    } else {
        for (auto&& [i, header] : util::enumerate(headers)) {
            header = fmt::format("Column {}", i + 1);
        }
    }

    // Construct Data Frame
    auto df = std::make_shared<DataFrame>();
    auto types = findCellTypes(headers.size(), rows, exampleRows_);
    if (firstColIndices_) {
        if (!types.empty() && types.front().string == 0 && types.front().real == 0) {
            types.front().index = true;
        } else {
            throw Exception("Unable to use first column as index, invalid data found");
        }
    }
    const auto appenders = addColumns(*df, types, headers);

    for (const auto& [row, lineNumber] : rows) {
        if (!skipRow(row, lineNumber, false)) {
            util::parse(row, delimiters_, headers.size(), lineNumber,
                        [&, l = lineNumber](std::string_view cell, size_t index,
                                            [[maybe_unused]] size_t part) {
                            appenders[index](cell, l, index + 1);
                        });
        }
    }

    if (!firstColIndices_) {
        df->updateIndexBuffer();
    }

    return df;
}

}  // namespace inviwo
