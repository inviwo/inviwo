/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

namespace inviwo {
DataWriterException::DataWriterException(const std::string& message)
    : Exception(message) {}


DataWriter::DataWriter() : overwrite_(false), extensions_() {}

DataWriter::DataWriter(const DataWriter& rhs)
    : overwrite_(rhs.overwrite_)
    , extensions_(rhs.extensions_) {
}

DataWriter& DataWriter::operator=(const DataWriter& that) {
    if (this != &that) {
        overwrite_ = that.overwrite_;
        extensions_.clear();

        for (std::vector<FileExtension>::const_iterator it = that.getExtensions().begin();
             it != that.getExtensions().end(); ++it)
            extensions_.push_back(*it);
    }

    return *this;
}

const std::vector<FileExtension>& DataWriter::getExtensions() const {
    return extensions_;
}
void DataWriter::addExtension(FileExtension ext) {
    extensions_.push_back(ext);
}

bool DataWriter::getOverwrite() const {
    return overwrite_;
}
void DataWriter::setOverwrite(bool val) {
    overwrite_ = val;
}

} // namespace
