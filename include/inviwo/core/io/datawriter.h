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

#ifndef IVW_DATAWRITER_H
#define IVW_DATAWRITER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/data.h>
#include <inviwo/core/util/fileextension.h>
#include <inviwo/core/util/exception.h>

namespace inviwo {

class IVW_CORE_API DataWriterException : public Exception {
public:
    DataWriterException(const std::string& message = "",
                        ExceptionContext context = ExceptionContext());
    virtual ~DataWriterException() throw(){};
};

class IVW_CORE_API DataWriter {

public:
    DataWriter();
    DataWriter(const DataWriter& rhs);
    DataWriter& operator=(const DataWriter& that);
    virtual DataWriter* clone() const = 0;
    virtual ~DataWriter() {};

    const std::vector<FileExtension>& getExtensions() const;
    void addExtension(FileExtension ext);

    bool getOverwrite() const;
    void setOverwrite(bool val);

protected:
    bool overwrite_;

private:
    std::vector<FileExtension> extensions_;

};


template <typename T>
class DataWriterType : public DataWriter {
public:
    DataWriterType() : DataWriter() {};
    DataWriterType(const DataWriterType& rhs) : DataWriter(rhs) {};
    DataWriterType& operator=(const DataWriterType& that) {
        if (this != &that)
            DataWriter::operator=(that);

        return *this;
    };
    virtual DataWriterType* clone() const = 0;
    virtual ~DataWriterType() {};

    virtual void writeData(const T* data, const std::string filePath) const = 0;
    virtual std::vector<unsigned char>* writeDataToBuffer(const T* data, std::string& fileType) const {
        return nullptr;
    }
    virtual bool writeDataToRepresentation(const DataRepresentation* src, DataRepresentation* dst) const {
        return false;
    }
};


} // namespace

#endif // IVW_DATAWRITER_H
