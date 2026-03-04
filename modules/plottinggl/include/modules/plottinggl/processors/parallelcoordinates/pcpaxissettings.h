/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2026 Inviwo Foundation
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

#include <modules/plottinggl/plottingglmoduledefine.h>

#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/util/glmvec.h>
#include <modules/plotting/datastructures/axisdata.h>
#include <modules/plotting/datastructures/tickdata.h>
#include <modules/plotting/datastructures/plottextdata.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace inviwo {

class CategoricalColumn;
class Column;
class DataFrame;
class FontSettings;

namespace plot {

class PCPAxisSettings;
class ParallelCoordinates;

/**
 * Helper class for handling axis specific tasks for the parallel coordinates plot
 */
class IVW_MODULE_PLOTTINGGL_API PCPAxisSettings : public BoolCompositeProperty {
public:
    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{
        "org.inviwo.parallelcoordinates.axissettingsproperty"};

    PCPAxisSettings(std::string_view identifier, std::string_view displayName, size_t columnId = 0);
    PCPAxisSettings(const PCPAxisSettings& rhs);
    virtual PCPAxisSettings* clone() const override;

    virtual ~PCPAxisSettings() = default;

    /**
     * Update the range of the axis based on the given column.
     */
    void update(std::shared_ptr<const DataFrame> frame);

    /**
     * Normalizes the value v from the range of the parameter to zero and one. Clamps out-of-bounds
     * values to zero and one. Using inverse linear interpolation between min and max unless
     * usePercentiles is checked. If usePercentiles is checked linear interpolation is used within
     * three percentile ranges (0-25, 25-75, 75-100).
     */
    double getNormalized(double v) const;

    /**
     * Samples the column at the given index and returned the normalized value.
     * @see getNormalized(double)
     */
    double getNormalizedAt(size_t idx) const;

    /**
     * Get data-range value from a normalized value. This the inverse function of getNormalized, ie
     * `x = getValue(getNormalized(x))`.
     * @see getNormalized(double)
     */
    double getValue(double normalizedV) const;

    /**
     * Helper function for ParallelCoordinates::handlePicked
     */
    void moveHandle(bool upper, double mouseY);

    uint32_t columnId() const { return columnId_; }
    void setColumnId(uint32_t id) { columnId_ = id; }

    void setParallelCoordinates(ParallelCoordinates* pcp);

    const std::vector<bool>& getBrushed() const { return brushed_; }

    bool isFiltering() const { return upperBrushed_ || lowerBrushed_; }

    dvec2 getRange() const;
    vec4 getColor() const;
    float getWidth() const;
    void update(AxisData& data) const;

    std::function<double(size_t)> at = [](size_t) { return 0.0; };

    DoubleMinMaxProperty range;
    BoolProperty invertRange;

    ParallelCoordinates* pcp_ = nullptr;
    std::shared_ptr<const Column> col_;
    const CategoricalColumn* catCol_ = nullptr;
    std::string caption_;

private:
    void updateBrushing();

    bool upperBrushed_ = false;  //! Flag to indicated if the upper handle is brushing away data
    bool lowerBrushed_ = false;  //! Flag to indicated if the lower handle is brushing away data

    uint32_t columnId_;
    std::vector<bool> brushed_;
};

}  // namespace plot

}  // namespace inviwo
