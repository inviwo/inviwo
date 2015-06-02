/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_DISKREPRESENTATION_H
#define IVW_DISKREPRESENTATION_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <string>

namespace inviwo {

class DataReader;

class IVW_CORE_API DiskRepresentation {

public:
    DiskRepresentation();
    DiskRepresentation(std::string);
    DiskRepresentation(const DiskRepresentation& rhs);
    DiskRepresentation& operator=(const DiskRepresentation& that);
    virtual DiskRepresentation* clone() const;
    virtual ~DiskRepresentation();

    const std::string& getSourceFile() const;
    bool hasSourceFile() const;

    void setDataReader(DataReader* reader);

    void* readData() const;
    void readDataInto(void* dest) const;

private:
#pragma warning(push)
#pragma warning(disable : 4251)
    std::string sourceFile_;
#pragma warning(pop)
    // DiskRepresentation owns a DataReader to be able to convert it self into RAM.
    DataReader* reader_;
};

} // namespace

#endif // IVW_DISKREPRESENTATION_H
