/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2021 Inviwo Foundation
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

#include <inviwo/core/io/datawriter.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/datawriterexception.h>
#include <fmt/format.h>

namespace inviwo {

DataWriter::DataWriter() : overwrite_(Overwrite::No), extensions_() {}

DataWriter::DataWriter(const DataWriter& rhs)
    : overwrite_(rhs.overwrite_), extensions_(rhs.extensions_) {}

DataWriter& DataWriter::operator=(const DataWriter& that) {
    if (this != &that) {
        overwrite_ = that.overwrite_;
        extensions_.clear();

        for (const auto& elem : that.getExtensions()) extensions_.push_back(elem);
    }

    return *this;
}

const std::vector<FileExtension>& DataWriter::getExtensions() const { return extensions_; }
void DataWriter::addExtension(FileExtension ext) { extensions_.push_back(ext); }

Overwrite DataWriter::getOverwrite() const { return overwrite_; }
void DataWriter::setOverwrite(Overwrite val) { overwrite_ = val; }

void DataWriter::checkOverwrite(std::string_view path, Overwrite overwrite) {
    if (filesystem::fileExists(path) && overwrite == Overwrite::No)
        throw DataWriterException(IVW_CONTEXT_CUSTOM("DataWriter"),
                                  "Output file: {} already exists", path);
}

std::ofstream DataWriter::open(std::string_view path, std::ios_base::openmode mode) const {
    checkOverwrite(path);
    auto f = filesystem::ofstream(path, mode);
    if (!f) {
        throw FileException(IVW_CONTEXT, "Could not open file '{}'", path);
    }
    return f;
}

void DataWriter::checkOverwrite(std::string_view path) const { checkOverwrite(path, overwrite_); }

}  // namespace inviwo
