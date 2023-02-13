/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2022 Inviwo Foundation
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

#include <modules/plottinggl/plottingglmoduledefine.h>  // for IVW_MODULE_PLOTTINGGL_API

#include <inviwo/core/properties/boolcompositeproperty.h>       // for BoolCompositeProperty
#include <inviwo/core/properties/boolproperty.h>                // for BoolProperty
#include <inviwo/core/properties/minmaxproperty.h>              // for DoubleMinMaxProperty
#include <inviwo/core/util/glmvec.h>                            // for vec4, vec2, dvec2
#include <modules/plotting/datastructures/axissettings.h>       // for AxisSettings, AxisSetting...
#include <modules/plotting/datastructures/majorticksettings.h>  // for TickStyle, MajorTickSettings
#include <modules/plotting/datastructures/minorticksettings.h>  // for MinorTickSettings
#include <modules/plotting/datastructures/plottextsettings.h>   // for PlotTextSettings

#include <cstddef>      // for size_t
#include <cstdint>      // for uint32_t
#include <functional>   // for function, __base
#include <memory>       // for shared_ptr
#include <string>       // for string
#include <string_view>  // for string_view
#include <vector>       // for vector

namespace inviwo {

class CategoricalColumn;
class Column;
class DataFrame;
class FontSettings;

namespace plot {

class PCPAxisSettings;
class ParallelCoordinates;

class IVW_MODULE_PLOTTINGGL_API PCPLabelSettings : public PlotTextSettings {
public:
    PCPLabelSettings(PCPAxisSettings* settings) : settings_{settings} {}

    // Inherited via PlotTextSettings
    virtual bool isEnabled() const override;
    virtual LabelPlacement getPlacement() const override;
    virtual vec4 getColor() const override;
    virtual float getPosition() const override;
    virtual vec2 getOffset() const override;
    virtual float getRotation() const override;
    virtual const FontSettings& getFont() const override;

private:
    PCPAxisSettings* settings_;
};

class IVW_MODULE_PLOTTINGGL_API PCPCaptionSettings : public PlotTextSettings {
public:
    PCPCaptionSettings(PCPAxisSettings* settings) : settings_{settings} {}

    // Inherited via PlotTextSettings
    virtual bool isEnabled() const override;
    virtual LabelPlacement getPlacement() const override;
    virtual vec4 getColor() const override;
    virtual float getPosition() const override;
    virtual vec2 getOffset() const override;
    virtual float getRotation() const override;
    virtual const FontSettings& getFont() const override;

private:
    PCPAxisSettings* settings_;
};

class IVW_MODULE_PLOTTINGGL_API PCPMajorTickSettings : public MajorTickSettings {
public:
    PCPMajorTickSettings(PCPAxisSettings* settings) : settings_{settings} {}

    // Inherited via MajorTickSettings
    virtual TickStyle getStyle() const override;
    virtual vec4 getColor() const override;
    virtual float getTickLength() const override;
    virtual float getTickWidth() const override;
    virtual double getTickDelta() const override;
    virtual bool getRangeBasedTicks() const override;

private:
    PCPAxisSettings* settings_;
};

class IVW_MODULE_PLOTTINGGL_API PCPMinorTickSettings : public MinorTickSettings {
public:
    PCPMinorTickSettings(PCPAxisSettings* settings) : settings_{settings} {}

    // Inherited via MinorTickSettings
    virtual TickStyle getStyle() const override;
    virtual bool getFillAxis() const override;
    virtual vec4 getColor() const override;
    virtual float getTickLength() const override;
    virtual float getTickWidth() const override;
    virtual int getTickFrequency() const override;

private:
    PCPAxisSettings* settings_;
};

/**
 * \class PCPAxisSettings
 * Helper class for handling axis specific tasks for the parallel coordinates plot
 */
class IVW_MODULE_PLOTTINGGL_API PCPAxisSettings : public AxisSettings,
                                                  public BoolCompositeProperty {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

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

    // Inherited via AxisSettings
    virtual dvec2 getRange() const override;
    virtual bool getUseDataRange() const override;

    virtual bool getAxisVisible() const override;
    virtual bool getMirrored() const override;
    virtual vec4 getColor() const override;
    virtual float getWidth() const override;
    virtual float getScalingFactor() const override;
    virtual Orientation getOrientation() const override;

    virtual const std::string& getCaption() const override;
    virtual const PlotTextSettings& getCaptionSettings() const override;

    virtual const std::vector<std::string>& getLabels() const override;
    virtual const PlotTextSettings& getLabelSettings() const override;

    virtual const MajorTickSettings& getMajorTicks() const override;
    virtual const MinorTickSettings& getMinorTicks() const override;

    std::function<double(size_t)> at = [](size_t) { return 0.0; };

    DoubleMinMaxProperty range;
    BoolProperty invertRange;

    ParallelCoordinates* pcp_ = nullptr;
    std::shared_ptr<const Column> col_;
    const CategoricalColumn* catCol_ = nullptr;

private:
    void updateBrushing();
    void updateLabels();

    std::string caption_;
    std::vector<std::string> labels_;
    std::shared_ptr<std::function<void()>> labelUpdateCallback_;

    PCPCaptionSettings captionSettings_;
    PCPLabelSettings labelSettings_;
    PCPMajorTickSettings major_;
    PCPMinorTickSettings minor_;

    bool upperBrushed_ = false;  //! Flag to indicated if the upper handle is brushing away data
    bool lowerBrushed_ = false;  //! Flag to indicated if the lower handle is brushing away data

    uint32_t columnId_;
    std::vector<bool> brushed_;
};

}  // namespace plot

}  // namespace inviwo
