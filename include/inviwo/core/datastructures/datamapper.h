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

#ifndef IVW_DATAMAPPER_H
#define IVW_DATAMAPPER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

namespace inviwo {

class IVW_CORE_API DataMapper {
public:
    DataMapper();
    DataMapper(const DataFormatBase* format);
    DataMapper(const DataMapper& rhs);
    DataMapper& operator=(const DataMapper& that);
    virtual DataMapper* clone() const;
    virtual ~DataMapper() {}

    dvec2 dataRange;
    dvec2 valueRange;
    std::string valueUnit;

    void initWithFormat(const DataFormatBase* format);

    template <typename T>
    T mapFromDataToValue(T val) {
        return static_cast<T>((static_cast<double>(val) - dataRange.x) /
                                  (dataRange.y - dataRange.x) * (valueRange.y - valueRange.x) +
                              valueRange.x);
    }
    
    template <typename T>
    T mapFromValueToData(T val) {
        return static_cast<T>((static_cast<double>(val) - valueRange.x) /
                                  (valueRange.y - valueRange.x) * (dataRange.y - dataRange.x) +
                              dataRange.x);
    }
};

}  // namespace

#endif  // IVW_DATAMAPPER_H
