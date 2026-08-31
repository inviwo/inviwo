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
#include <modules/hdf5/properties/dimselectionproperty.h>

#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/invalidationlevel.h>

#include <cstddef>
#include <memory>
#include <string_view>
#include <vector>

namespace inviwo {

namespace hdf5 {

struct DataSetInfo;

/**
 * @ingroup properties
 * A collection of @see DimSelectionProperty, one per dataset dimension. The property holds a fixed
 * pool of @p maxRank sub-selections and shows only the trailing ones matching the rank of the
 * currently selected dataset. Selections are reported in column major (Inviwo) order.
 */
class IVW_MODULE_HDF5_API DimSelectionsProperty : public CompositeProperty {
public:
    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{"org.inviwo.hdf5.DimSelectionsProperty"};

    DimSelectionsProperty(std::string_view identifier, std::string_view displayName, size_t maxRank,
                          InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput);
    DimSelectionsProperty(const DimSelectionsProperty& rhs);
    virtual DimSelectionsProperty* clone() const override;
    virtual ~DimSelectionsProperty() = default;

    /**
     * Adapt the visible sub-selections and their ranges to the dimensions of @p info.
     */
    void update(const DataSetInfo& info);

    /**
     * The current selection for each active dimension, in column major (Inviwo) order.
     */
    [[nodiscard]] std::vector<Selection> getSelection() const;

    /**
     * The maximal selection for each active dimension, in column major (Inviwo) order.
     */
    [[nodiscard]] std::vector<Selection> getMaxSelection() const;

private:
    size_t maxRank_;
    size_t rank_;
    std::vector<std::unique_ptr<DimSelectionProperty>> selection_;
};

}  // namespace hdf5

}  // namespace inviwo
