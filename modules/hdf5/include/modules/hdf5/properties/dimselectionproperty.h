/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#pragma once

#include <modules/hdf5/hdf5moduledefine.h>
#include <modules/hdf5/datastructures/hdf5selection.h>

#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/propertysemantics.h>

#include <string_view>

namespace inviwo::hdf5 {

/**
 * @ingroup properties
 * A selection along a single HDF5 dataset dimension expressed as a @p start offset, a @p count of
 * elements to read (0 means all remaining elements) and a @p stride. The resulting hyperslab
 * @see Selection maps to `{start, start + count * stride, stride}` (clamped to the dimension size),
 * or `{start, dimSize, stride}` when the count is 0.
 */
class IVW_MODULE_HDF5_API DimSelectionProperty : public CompositeProperty {
public:
    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{"org.inviwo.hdf5.DimSelectionProperty"};

    DimSelectionProperty(std::string_view identifier, std::string_view displayName,
                         InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput);
    DimSelectionProperty(const DimSelectionProperty& rhs);
    DimSelectionProperty& operator=(const DimSelectionProperty& rhs) = delete;
    DimSelectionProperty(DimSelectionProperty&&) = delete;
    DimSelectionProperty& operator=(DimSelectionProperty&&) = delete;
    virtual DimSelectionProperty* clone() const override;
    virtual ~DimSelectionProperty() = default;

    /**
     * Adapt the start and count max values to @p dimSize.
     */
    void update(size_t dimSize);

    /**
     * The current selection as an HDF5 hyperslab.
     */
    [[nodiscard]] Selection getSelection() const;

    IntSizeTProperty start;
    IntSizeTProperty count;
    IntSizeTProperty stride;
};

}  // namespace inviwo::hdf5
