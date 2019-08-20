/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/network/networklock.h>
#include <modules/fontrendering/util/fontutils.h>

namespace inviwo {

namespace plot {

const std::string AxisStyleProperty::classIdentifier = "org.inviwo.AxisStyleProperty";
std::string AxisStyleProperty::getClassIdentifier() const { return classIdentifier; }

AxisStyleProperty::AxisStyleProperty(const std::string& identifier, const std::string& displayName,
                                     InvalidationLevel invalidationLevel,
                                     PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , fontFace_("fontFace", "Font Face")
    , fontSize_("fontSize", "Font Size", 14, 0, 144, 1)
    , color_("defaultcolor", "Color", vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f), vec4(1.0f))
    , lineWidth_("lineWidth", "Line Width", 2.5f, 0.0f, 20.0f)
    , tickLength_("tickLength", "Tick Length", 8.0f, 0.0f, 20.0f)
    , labelFormat_("labelFormat", "Label Format", "%.1f") {

    util::for_each_in_tuple([&](auto& e) { this->addProperty(e); }, props());

    for (auto font : util::getAvailableFonts()) {
        auto name = filesystem::getFileNameWithoutExtension(font.second);
        // use the file name w/o extension as identifier
        fontFace_.addOption(name, font.first, font.second);
    }
    fontFace_.setSelectedIdentifier("Montserrat-Medium");
    fontFace_.setCurrentStateAsDefault();
    fontSize_.setSemantics(PropertySemantics("Fontsize"));
    color_.setSemantics(PropertySemantics::Color);

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
