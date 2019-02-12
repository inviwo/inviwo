/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#ifndef IVW_PARALLELCOORDINATESAXISSETTINGSPROPERTY_H
#define IVW_PARALLELCOORDINATESAXISSETTINGSPROPERTY_H

#include <modules/plottinggl/plottingglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <modules/opengl/texture/texture2d.h>

namespace inviwo {
namespace plot {
class Column;
/**
 * \class ParallelCoordinatesAxisSettingsProperty
 * Helper class for handling axis specific tasks for the parallel coordinates plot
 */
class IVW_MODULE_PLOTTINGGL_API ParallelCoordinatesAxisSettingsProperty
    : public BoolCompositeProperty {
public:
    friend class ParallelCoordinates;

    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    ParallelCoordinatesAxisSettingsProperty(std::string identifier, std::string displayName);
    ParallelCoordinatesAxisSettingsProperty(const ParallelCoordinatesAxisSettingsProperty& rhs);
    ParallelCoordinatesAxisSettingsProperty& operator=(
        const ParallelCoordinatesAxisSettingsProperty& that);
    virtual ParallelCoordinatesAxisSettingsProperty* clone() const override;

    virtual ~ParallelCoordinatesAxisSettingsProperty() = default;

    /**
     * Update the range of the axis based on the given column.
     */
    void updateFromColumn(std::shared_ptr<const Column> col);

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
     * (\f$ x = getValue(getNormalized(x)) \f$).
     * @see getNormalized(double)
     */
    double getValue(double normalizedV) const;

    /**
     * Helper function for ParallelCoordinates::handlePicked
     */
    void moveHandle(bool upper, double mouseY);

    /**
     * Helper function for ParallelCoordinates::updateBrushing
     */
    void updateBrushing(std::unordered_set<size_t>& brushed);

    std::function<double(size_t)> at = [](size_t) { return 0.0; };

    BoolProperty usePercentiles;
    DoubleMinMaxProperty range;

private:
    std::shared_ptr<const Column> col_;

    bool upperBrushed_ = false;  //! Flag to indicated if the upper handle is brushing away data
    bool lowerBrushed_ = false;  //! Flag to indicated if the lower handle is brushing away data

    double p0_;
    double p25_;
    double p75_;
    double p100_;

    size_t columnId_;
    bool updating_ = false;

    std::string name_;
    std::shared_ptr<Texture2D> labelTexture_;   //! Texture cache used by ParallelCoordiantes
    std::shared_ptr<Texture2D> minValTexture_;  //! Texture cache used by ParallelCoordiantes
    std::shared_ptr<Texture2D> maxValTexture_;  //! Texture cache used by ParallelCoordiantes
};

}  // namespace plot
}  // namespace inviwo

#endif  // IVW_PARALLELCOORDINATESAXISSETTINGSPROPERTY_H
