/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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

namespace inviwo {
namespace plot {

/**
 * \class ParallelCoordinatesAxisSettingsProperty
 * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
 * DESCRIBE_THE_CLASS_FROM_A_DEVELOPER_PERSPECTIVE
 */
class IVW_MODULE_PLOTTINGGL_API ParallelCoordinatesAxisSettingsProperty
    : public BoolCompositeProperty {
public:
    friend class ParallelCoordinates;
    InviwoPropertyInfo();
    ParallelCoordinatesAxisSettingsProperty(std::string identifier, std::string displayName);
    virtual ~ParallelCoordinatesAxisSettingsProperty() = default;

    void updateFromColumn(std::shared_ptr<const Column> col);

    double getNormalizedAt(size_t idx) const;
    double getNormalized(double v) const;
    double getValue(double v) const;

    void updateRange(bool upper, double mouseY);
    void updateBrushing(std::unordered_set<size_t> &brushed);

    std::function<double(size_t)> at = [](size_t) { return 0.0; };

private:
    BoolProperty usePercentiles_;
    DoubleMinMaxProperty range_;

    std::shared_ptr<const Column> col_;

    bool upperBrushed_ = false;
    bool lowerBrushed_ = false;

    double p0_;
    double p25_;
    double p75_;
    double p100_;

    size_t columnId_;
    bool updating_ = false;

    std::string name_;
    std::shared_ptr<Texture2D> labelTexture_;
    std::shared_ptr<Texture2D> minValTexture_;
    std::shared_ptr<Texture2D> maxValTexture_;
};

}  // namespace plot
}  // namespace inviwo

#endif  // IVW_PARALLELCOORDINATESAXISSETTINGSPROPERTY_H
