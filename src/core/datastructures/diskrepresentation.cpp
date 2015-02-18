/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include <inviwo/core/datastructures/diskrepresentation.h>
#include <inviwo/core/io/datareader.h>

namespace inviwo {

DiskRepresentation::DiskRepresentation() : sourceFile_(""), reader_(NULL) {}

DiskRepresentation::DiskRepresentation(std::string srcFile) : sourceFile_(srcFile), reader_(NULL) {}

DiskRepresentation::DiskRepresentation(const DiskRepresentation& rhs)
    : sourceFile_(rhs.sourceFile_)
    , reader_(NULL) {
    setDataReader(rhs.reader_!=NULL?rhs.reader_->clone():NULL);
}

DiskRepresentation& DiskRepresentation::operator=(const DiskRepresentation& that) {
    if (this != &that) {
        sourceFile_ = that.sourceFile_;

        if (reader_) {
            delete reader_;
            reader_ = NULL;
        }

        setDataReader(that.reader_!=NULL?that.reader_->clone():NULL);
    }

    return *this;
}

DiskRepresentation* DiskRepresentation::clone() const {
    return new DiskRepresentation(*this);
}

DiskRepresentation::~DiskRepresentation() {
    if (reader_) {
        delete reader_;
        reader_ = NULL;
    }
}

const std::string& DiskRepresentation::getSourceFile() const {
    return sourceFile_;
}

bool DiskRepresentation::hasSourceFile() const {
    return !sourceFile_.empty();
}

void DiskRepresentation::setDataReader(DataReader* reader) {
    if(!reader)
        return;
    
    if (reader_)
        delete reader_;

    reader_ = reader;
    reader->setOwner(this);
}

void* DiskRepresentation::readData() const {
    if (reader_)
        return reader_->readData();

    return NULL;
}

void DiskRepresentation::readDataInto(void* dest) const {
    if (reader_)
        reader_->readDataInto(dest);
}

} // namespace
