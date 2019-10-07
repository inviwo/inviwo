/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#ifndef IVW_AXISPROPERTY_H
#define IVW_AXISPROPERTY_H

#include <modules/plotting/plottingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>

#include <modules/plotting/properties/tickproperty.h>
#include <modules/plotting/properties/plottextproperty.h>

#include <modules/plotting/datastructures/axissettings.h>

namespace inviwo {

namespace plot {

class IVW_MODULE_PLOTTING_API AxisProperty : public AxisSettings, public CompositeProperty {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    AxisProperty(const std::string& identifier, const std::string& displayName,
                 Orientation orientation = Orientation::Horizontal,
                 InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                 PropertySemantics semantics = PropertySemantics::Default);
    AxisProperty(const AxisProperty& rhs);
    virtual AxisProperty* clone() const override;
    virtual ~AxisProperty() = default;

    virtual void setCaption(const std::string& title);

    void setLabelFormat(const std::string& formatStr);
    /**
     * \brief sets range property of axis and ensures the min/max limits are adjusted accordingly
     * @param range   new axis range
     */
    void setRange(const dvec2& range);

    /**
     * \brief set all colors to \p c, i.e. axis, ticks, labels, and caption
     */
    void setColor(const vec4& c);

    /**
     * \brief set font face of labels and caption to \p fontFace
     */
    void setFontFace(const std::string& fontFace);

    /**
     * \brief set font size for caption and labels
     */
    void setFontSize(int fontsize);

    void setTickLength(float major, float minor);

    /**
     * \brief set the line width of the axis, major, and minor ticks. Minor ticks will be 2/3 the
     * width.
     */
    void setLineWidth(float width);

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

    // general properties
    BoolProperty visible_;
    FloatVec4Property color_;
    FloatProperty width_;
    BoolProperty useDataRange_;
    DoubleMinMaxProperty range_;

    BoolProperty flipped_;
    TemplateOptionProperty<Orientation> orientation_;
    TemplateOptionProperty<Placement> placement_;

    // caption besides axis
    PlotTextProperty captionSettings_;
    // labels showing numbers along axis
    PlotTextProperty labelSettings_;
    std::vector<std::string> categories_;

    MajorTickProperty majorTicks_;
    MinorTickProperty minorTicks_;

private:
    virtual void updateLabels();

    void adjustAlignment();
};

}  // namespace plot

}  // namespace inviwo

#endif  // IVW_AXISPROPERTY_H
