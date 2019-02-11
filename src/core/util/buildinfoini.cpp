/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#include <inviwo/core/util/buildinfo.h>

#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/filesystem.h>

#include <fstream>
#include <sstream>
#include <locale>

namespace inviwo {

namespace util {

// helper struct to define arbitrary separator characters in a locale
// usage:
//   std::stringstream stream;
//   stream.imbue(std::locale(stream.getloc(), new CsvWhitespace));
struct IniSeparator : std::ctype<char> {
    static const mask* makeTable() {
        // copy table of C locale
        static std::vector<mask> m(classic_table(), classic_table() + table_size);
        m[' '] &= ~space;  // remove space as whitespace
        m['='] |= space;
        return &m[0];
    }
    IniSeparator(std::size_t refs = 0) : ctype(makeTable(), false, refs) {}
};

BuildInfo getBuildInfo() {
    auto dir =
        filesystem::getFileDirectory(filesystem::getExecutablePath()) + "/inviwo_buildinfo.ini";
    auto in = filesystem::ifstream(dir.c_str(), std::ios::in);
    if (!in.is_open()) {
        return {};
    }

    BuildInfo buildInfo;

    std::istringstream iss;
    iss.imbue(std::locale(iss.getloc(), new IniSeparator()));
    enum class Section { Unknown, Date, Hashes };

    std::string line;
    Section currentSection = Section::Unknown;
    while (std::getline(in, line)) {
        line = trim(line);
        // ignore comment, i.e. line starts with ';'
        if (line.empty() || line[0] == ';') {
            continue;
        }
        if (line == "[date]") {
            currentSection = Section::Date;
        } else if (line == "[hashes]") {
            currentSection = Section::Hashes;
        } else if (line[0] == '[') {
            currentSection = Section::Unknown;
        } else {
            // read in key value pairs
            iss.clear();
            iss.str(line);
            std::string key;
            std::string value;
            if (!(iss >> key >> value)) {
                // invalid key-value pair, ignore it
                continue;
            }
            switch (currentSection) {
                case Section::Date: {
                    int valuei = std::stoi(value);
                    if (key == "year") {
                        buildInfo.year = valuei;
                    } else if (key == "month") {
                        buildInfo.month = valuei;
                    } else if (key == "day") {
                        buildInfo.day = valuei;
                    } else if (key == "hour") {
                        buildInfo.hour = valuei;
                    } else if (key == "minute") {
                        buildInfo.minute = valuei;
                    } else if (key == "second") {
                        buildInfo.second = valuei;
                    }
                    break;
                }
                case Section::Hashes:
                    buildInfo.githashes.push_back({key, value});
                    break;
                case Section::Unknown:
                default:
                    break;
            }
        }
    }
    return buildInfo;
}

}  // namespace util

}  // namespace inviwo
