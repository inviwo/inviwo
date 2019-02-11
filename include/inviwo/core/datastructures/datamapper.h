/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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
/**
 * \brief Map values into data or value ranges.
 * Data range refer to the range of the data type, i.e. [0 4095] for 12-bit unsigned integer data.
 * Value range refer to the physical meaning of the value, i.e. Hounsfield value range is from
 * [-1000 3000].
 * It is important that the data range is specified for data types with a large range (for example
 * 32/64-bit float and integer) since the data is often normalized to [0 1], when for example
 * performing color mapping, i.e. applying a transfer function.
 */
class IVW_CORE_API DataMapper {
public:
    DataMapper();
    DataMapper(const DataFormatBase* format);
    DataMapper(const DataMapper& rhs);
    DataMapper& operator=(const DataMapper& that);

    dvec2 dataRange;        ///< Minimum and maximum data range
    dvec2 valueRange;       ///< Minimum and maximum value range
    std::string valueUnit;  ///< Unit, i.e. Hounsfield/absorption/W.

    void initWithFormat(const DataFormatBase* format);

    template <typename T>
    T mapFromDataToValue(T val) const {
        return static_cast<T>((static_cast<double>(val) - dataRange.x) /
                                  (dataRange.y - dataRange.x) * (valueRange.y - valueRange.x) +
                              valueRange.x);
    }

    template <typename T>
    T mapFromValueToData(T val) const {
        return static_cast<T>((static_cast<double>(val) - valueRange.x) /
                                  (valueRange.y - valueRange.x) * (dataRange.y - dataRange.x) +
                              dataRange.x);
    }

    template <typename T>
    T mapFromValueToNormalized(T val) const {
        return static_cast<T>((static_cast<double>(val) - valueRange.x) /
                              (valueRange.y - valueRange.x));
    }

    template <typename T>
    T mapFromNormalizedToValue(T val) const {
        return static_cast<T>(valueRange.x +
                              static_cast<double>(val) * (valueRange.y - valueRange.x));
    }
};

}  // namespace inviwo

#endif  // IVW_DATAMAPPER_H
