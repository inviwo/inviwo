/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2024 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided rhs the following conditions are met:
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

#include <modules/plotting/properties/axisstyleproperty.h>

#include <inviwo/core/network/networklock.h>                          // for NetworkLock
#include <inviwo/core/properties/compositeproperty.h>                 // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>                 // for InvalidationLevel
#include <inviwo/core/properties/ordinalproperty.h>                   // for FloatProperty, Floa...
#include <inviwo/core/properties/propertysemantics.h>                 // for PropertySemantics
#include <inviwo/core/properties/stringproperty.h>                    // for StringProperty
#include <inviwo/core/util/foreacharg.h>                              // for for_each_in_tuple
#include <inviwo/core/util/glmvec.h>                                  // for vec4
#include <inviwo/core/util/stdextensions.h>                           // for contains, find
#include <modules/fontrendering/properties/fontfaceoptionproperty.h>  // for FontFaceOptionProperty
#include <modules/fontrendering/util/fontutils.h>                     // for FontType, FontType:...
#include <modules/plotting/properties/axisproperty.h>                 // for AxisProperty

#include <functional>  // for __base

namespace inviwo {

namespace plot {

std::string_view AxisStyleProperty::getClassIdentifier() const { return classIdentifier; }

AxisStyleProperty::AxisStyleProperty(std::string_view identifier, std::string_view displayName,
                                     InvalidationLevel invalidationLevel,
                                     PropertySemantics semantics)
    : CompositeProperty(
          identifier, displayName,
          "Convenience property for updating/overriding multiple axes properties. "
          "A property change will propagate to all the subproperties of the registered axes."_help,
          invalidationLevel, semantics)
    , fontFace_("fontFace", "Font Face", "Font face used for axis labels and captions"_help,
                font::FontType::Label)
    , fontSize_("fontSize", "Font Size",
                util::ordinalCount(14, 144)
                    .set("Font size for both axis labels and captions"_help)
                    .set(PropertySemantics("Fontsize")))
    , color_("defaultcolor", "Color",
             util::ordinalColor(vec4(0.0f, 0.0f, 0.0f, 1.0f)).set("Default color of the axis"_help))
    , lineWidth_("lineWidth", "Line Width",
                 util::ordinalLength(2.5f, 20.0f).set("Line width of the axis"_help))
    , tickLength_(
          "tickLength", "Tick Length",
          util::ordinalLength(8.0f, 20.0f)
              .set(
                  "Length of major ticks. The length of minor tick will be set to 75% of this length"_help))
    , labelFormat_("labelFormat", "Label Format",
                   "Formatting string for labels using the printf format specification."_help,
                   "%.1f") {

    util::for_each_in_tuple([&](auto& e) { this->addProperty(e); }, props());

    fontFace_.onChange([this]() {
        NetworkLock lock(this);
        for (auto a : axes_) {
            a->setFontFace(fontFace_);
        }
    });
    fontSize_.onChange([this]() {
        NetworkLock lock(this);
        for (auto a : axes_) {
            a->setFontSize(fontSize_);
        }
    });
    color_.onChange([this]() {
        NetworkLock lock(this);
        for (auto a : axes_) {
            a->setColor(color_);
        }
    });
    lineWidth_.onChange([this]() {
        NetworkLock lock(this);
        for (auto a : axes_) {
            a->setLineWidth(lineWidth_);
        }
    });
    tickLength_.onChange([this]() {
        NetworkLock lock(this);
        const auto len = tickLength_.get();
        for (auto a : axes_) {
            a->setTickLength(len, 0.75f * len);
        }
    });
    labelFormat_.onChange([this]() {
        NetworkLock lock(this);
        for (auto a : axes_) {
            a->setLabelFormat(labelFormat_);
        }
    });
}

AxisStyleProperty::AxisStyleProperty(const AxisStyleProperty& rhs)
    : CompositeProperty(rhs)
    , fontFace_(rhs.fontFace_)
    , fontSize_(rhs.fontSize_)
    , color_(rhs.color_)
    , lineWidth_(rhs.lineWidth_)
    , tickLength_(rhs.tickLength_)
    , labelFormat_(rhs.labelFormat_)
    , axes_(rhs.axes_) {
    util::for_each_in_tuple([&](auto& e) { this->addProperty(e); }, props());
}

AxisStyleProperty* AxisStyleProperty::clone() const { return new AxisStyleProperty(*this); }

void AxisStyleProperty::registerProperty(AxisProperty& p) {
    if (util::contains(axes_, &p)) {
        return;
    }
    axes_.push_back(&p);
}

void AxisStyleProperty::unregisterProperty(AxisProperty& p) {
    auto it = util::find(axes_, &p);
    if (it == axes_.end()) return;

    axes_.erase(it);
}

void AxisStyleProperty::unregisterAll() { axes_.clear(); }

}  // namespace plot

}  // namespace inviwo
