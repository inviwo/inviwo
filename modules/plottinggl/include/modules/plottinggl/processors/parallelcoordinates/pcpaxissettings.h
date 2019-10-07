/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <modules/opengl/texture/texture2d.h>

#include <modules/plotting/datastructures/axissettings.h>

namespace inviwo {

class Column;
class CategoricalColumn;

namespace plot {

class ParallelCoordinates;
class PCPAxisSettings;

class IVW_MODULE_PLOTTINGGL_API PCPLabelSettings : public PlotTextSettings {
public:
    // Inherited via PlotTextSettings
    virtual bool isEnabled() const override;
    virtual vec4 getColor() const override;
    virtual float getPosition() const override;
    virtual vec2 getOffset() const override;
    virtual float getRotation() const override;
    virtual const FontSettings& getFont() const override;

    void setSettings(PCPAxisSettings* settings) { settings_ = settings; }

private:
    PCPAxisSettings* settings_ = nullptr;
};

class IVW_MODULE_PLOTTINGGL_API PCPCaptionSettings : public PlotTextSettings {
public:
    // Inherited via PlotTextSettings
    virtual bool isEnabled() const override;
    virtual vec4 getColor() const override;
    virtual float getPosition() const override;
    virtual vec2 getOffset() const override;
    virtual float getRotation() const override;
    virtual const FontSettings& getFont() const override;

    void setSettings(PCPAxisSettings* settings) { settings_ = settings; }

private:
    PCPAxisSettings* settings_ = nullptr;
};

class IVW_MODULE_PLOTTINGGL_API PCPMajorTickSettings : public MajorTickSettings {
public:
    // Inherited via MajorTickSettings
    virtual TickStyle getStyle() const override;
    virtual vec4 getColor() const override;
    virtual float getTickLength() const override;
    virtual float getTickWidth() const override;
    virtual double getTickDelta() const override;
    virtual bool getRangeBasedTicks() const override;

    void setSettings(PCPAxisSettings* settings) { settings_ = settings; }

private:
    PCPAxisSettings* settings_ = nullptr;
};

class IVW_MODULE_PLOTTINGGL_API PCPMinorTickSettings : public MinorTickSettings {
public:
    // Inherited via MinorTickSettings
    virtual TickStyle getStyle() const override;
    virtual bool getFillAxis() const override;
    virtual vec4 getColor() const override;
    virtual float getTickLength() const override;
    virtual float getTickWidth() const override;
    virtual int getTickFrequency() const override;

    void setSettings(PCPAxisSettings* settings) { settings_ = settings; }

private:
    PCPAxisSettings* settings_ = nullptr;
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

    PCPAxisSettings(std::string identifier, std::string displayName, size_t columnId = 0);
    PCPAxisSettings(const PCPAxisSettings& rhs);
    virtual PCPAxisSettings* clone() const override;

    virtual ~PCPAxisSettings() = default;

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

    std::function<double(size_t)> at = [](size_t) { return 0.0; };

    size_t columnId() const { return columnId_; }
    void setColumnId(size_t id) { columnId_ = id; }

    void setParallelCoordinates(ParallelCoordinates* pcp);

    const std::vector<bool>& getBrushed() const { return brushed_; }

    bool isFiltering() const { return upperBrushed_ || lowerBrushed_; }

    // Inherited via AxisSettings
    virtual dvec2 getRange() const override;
    virtual bool getUseDataRange() const override;

    virtual bool getAxisVisible() const override;
    virtual bool getFlipped() const override;
    virtual vec4 getColor() const override;
    virtual float getWidth() const override;
    virtual Orientation getOrientation() const override;
    virtual Placement getPlacement() const override;

    virtual const std::string& getCaption() const override;
    virtual const PlotTextSettings& getCaptionSettings() const override;

    virtual const std::vector<std::string>& getLabels() const override;
    virtual const PlotTextSettings& getLabelSettings() const override;

    virtual const MajorTickSettings& getMajorTicks() const override;
    virtual const MinorTickSettings& getMinorTicks() const override;

    BoolProperty usePercentiles;
    BoolProperty invertRange;
    DoubleMinMaxProperty range;

    ParallelCoordinates* pcp_ = nullptr;
    std::shared_ptr<const Column> col_;
    const CategoricalColumn* catCol_;

private:
    void updateBrushing();

    PCPCaptionSettings captionSettings_;
    std::vector<std::string> labels_;
    std::shared_ptr<std::function<void()>> labelUpdateCallback_;
    PCPLabelSettings labelSettings_;

    PCPMajorTickSettings major_;
    PCPMinorTickSettings minor_;

    bool upperBrushed_ = false;  //! Flag to indicated if the upper handle is brushing away data
    bool lowerBrushed_ = false;  //! Flag to indicated if the lower handle is brushing away data

    double p0_;
    double p25_;
    double p75_;
    double p100_;

    size_t columnId_;
    std::vector<bool> brushed_;
};

}  // namespace plot
}  // namespace inviwo
