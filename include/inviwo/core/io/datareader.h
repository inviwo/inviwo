/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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

#ifndef IVW_DATAREADER_H
#define IVW_DATAREADER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/fileextension.h>

namespace inviwo {

/** \brief A abstract base class for all file readers.
 */
class IVW_CORE_API DataReader {
public:
    DataReader() = default;
    DataReader(const DataReader& rhs) = default;
    DataReader& operator=(const DataReader& that) = default;
    virtual DataReader* clone() const = 0;
    virtual ~DataReader() = default;

    const std::vector<FileExtension>& getExtensions() const;
    void addExtension(FileExtension ext);

private:
    std::vector<FileExtension> extensions_;
};

/** \brief Template base class for file readers designating what type of data
 * object the reader returns.
 */
template <typename T>
class DataReaderType : public DataReader {
public:
    DataReaderType() = default;
    DataReaderType(const DataReaderType& rhs) = default;
    DataReaderType& operator=(const DataReaderType& that) = default;
    virtual DataReaderType* clone() const = 0;
    virtual ~DataReaderType() = default;
    virtual std::shared_ptr<T> readData(const std::string filePath) = 0;
};

}  // namespace

#endif  // IVW_DATAREADER_H
