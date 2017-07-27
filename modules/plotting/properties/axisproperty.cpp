/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#include <modules/plotting/properties/axisproperty.h>

namespace inviwo {

namespace plot {

PropertyClassIdentifier(AxisProperty, "org.inviwo.AxisProperty");

AxisProperty::AxisProperty(const std::string& identifier, const std::string& displayName,
                           Orientation orientation, InvalidationLevel invalidationLevel,
                           PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , visible_("visible", "Visible", true)
    , color_("color", "Color", vec4(vec3(0.0f), 1.0f), vec4(0.0f), vec4(1.0f))
    , width_("width", "Width", 2.5f, 0.0f, 20.0f)
    , range_("range", "Axis Range", 0.0, 100.0, 0.0, 1.0e4)
    , orientation_("orientation", "Orientation",
                   {{"horizontal", "Horizontal", Orientation::Horizontal},
                    {"vertical", "Vertical", Orientation::Vertical}},
                   0)
    , placement_("placement", "Placement",
                 {{"outside", "Bottom / Left", Placement::Outside},
                  {"inside", "Top / Right", Placement::Inside}},
                 0)
    //, logScale_("logScale", "logScale", false)
    , caption_("caption", "Caption", false)
    , labels_("labels", "Axis Labels", true)
    , ticks_("ticks", "Ticks") {

    orientation_.setSelectedValue(orientation);
    orientation_.setCurrentStateAsDefault();
    color_.setSemantics(PropertySemantics::Color);
    range_.setSemantics(PropertySemantics::Text);

    addProperty(visible_);
    addProperty(color_);
    addProperty(width_);
    addProperty(range_);
    addProperty(orientation_);
    addProperty(placement_);
    // addProperty(logScale_);

    // change default fonts, make axis labels slightly less pronounced
    caption_.font_.fontFace_.setSelectedIdentifier("Montserrat-Regular");
    labels_.font_.fontFace_.setSelectedIdentifier("Montserrat-Light");

    caption_.title_.set("Axis Title");
    caption_.offset_.set(35.0f);

    labels_.title_.setDisplayName("Format");
    labels_.title_.set("%.1f");
    labels_.position_.setVisible(false);
    labels_.setCurrentStateAsDefault();

    addProperty(caption_);
    addProperty(labels_);
    addProperty(ticks_);

    caption_.setCollapsed(true);
    labels_.setCollapsed(true);
    ticks_.setCollapsed(true);
    setCollapsed(true);

    auto adjustAlignment = [this]() {
        vec2 anchor;
        if (orientation_.get() == Orientation::Horizontal) {
            // horizontal axis, center labels
            anchor = vec2(0.0f, (placement_.get() == Placement::Outside) ? 1.0 : -1.0);
        } else {
            // vertical axis
            anchor = vec2((placement_.get() == Placement::Outside) ? 1.0 : -1.0, 0.0f);
        }
        labels_.font_.anchorPos_.set(anchor);
        caption_.font_.anchorPos_.set(anchor);
    };
    orientation_.onChange(adjustAlignment);
    placement_.onChange(adjustAlignment);
    // update label alignment to match current status
    adjustAlignment();
}

AxisProperty::AxisProperty(const AxisProperty& rhs)
    : CompositeProperty(rhs)
    , visible_(rhs.visible_)
    , color_(rhs.color_)
    , width_(rhs.width_)
    , range_(rhs.range_)
    , orientation_(rhs.orientation_)
    , placement_(rhs.placement_)
    /*, logScale_(rhs.logScale_)*/
    , caption_(rhs.caption_)
    , labels_(rhs.labels_)
    , ticks_(rhs.ticks_) {

    color_.setSemantics(PropertySemantics::Color);
    addProperty(visible_);
    addProperty(color_);
    addProperty(width_);
    addProperty(range_);
    addProperty(orientation_);
    addProperty(placement_);
    // addProperty(logScale_);

    addProperty(caption_);
    addProperty(labels_);
    addProperty(ticks_);

    caption_.setCollapsed(true);
    labels_.setCollapsed(true);
    ticks_.setCollapsed(true);
    setCollapsed(true);

    auto adjustAlignment = [this]() {
        vec2 anchor;
        if (orientation_.get() == Orientation::Horizontal) {
            // horizontal axis, center labels
            anchor = vec2(0.0f, (placement_.get() == Placement::Outside) ? 1.0 : -1.0);
        } else {
            // vertical axis
            anchor = vec2((placement_.get() == Placement::Outside) ? 1.0 : -1.0, 0.0f);
        }
        labels_.font_.anchorPos_.set(anchor);
        caption_.font_.anchorPos_.set(anchor);
    };
    orientation_.onChange(adjustAlignment);
    placement_.onChange(adjustAlignment);
    // update label alignment to match current status
    adjustAlignment();
}

AxisProperty* AxisProperty::clone() const { return new AxisProperty(*this); }

}  // namespace plot

}  // namespace inviwo
