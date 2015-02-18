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
#include <inviwo/core/datastructures/data.h>
#include <inviwo/core/datastructures/diskrepresentation.h>
#include <inviwo/core/util/fileextension.h>
#include <inviwo/core/util/exception.h>

namespace inviwo {

class IVW_CORE_API DataReaderException : public Exception {
public:
    DataReaderException(const std::string& message = "");
    virtual ~DataReaderException() throw() {};
};


/** \brief A abstract base class for all file readers.
 *
 */
class IVW_CORE_API DataReader {

friend class DiskRepresentation;

public:
    DataReader();
    DataReader(const DataReader& rhs);
    DataReader& operator=(const DataReader& that);
    virtual DataReader* clone() const = 0;
    virtual ~DataReader() {};

    virtual void* readData() const = 0;
    virtual void readDataInto(void* dest) const = 0;

    const std::vector<FileExtension>& getExtensions() const;
    void addExtension(FileExtension ext);

protected:
    void setOwner(DiskRepresentation* owner) { owner_ = owner; };

    DiskRepresentation* owner_;

private:
    std::vector<FileExtension> extensions_;
};

/** \brief Template base class for file readers designating what type of data object the reader returns.
 *
 */
template <typename T>
class DataReaderType : public DataReader {
public:
    DataReaderType() : DataReader() {};
    DataReaderType(const DataReaderType& rhs) : DataReader(rhs) {};
    DataReaderType& operator=(const DataReaderType& that) {
        if (this != &that)
            DataReader::operator=(that);

        return *this;
    };
    virtual DataReaderType* clone() const = 0;
    virtual ~DataReaderType() {};

    virtual T* readMetaData(const std::string filePath) = 0;
    virtual void* readData() const = 0;
    virtual void readDataInto(void* dest) const = 0;
};

} // namespace

#endif // IVW_DATAREADER_H

