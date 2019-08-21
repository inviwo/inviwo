/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#include <inviwo/dataframe/properties/colormapproperty.h>

namespace inviwo {

const std::string ColorMapProperty::classIdentifier = "org.inviwo.ColorMapProperty";
std::string ColorMapProperty::getClassIdentifier() const { return classIdentifier; }

ColorMapProperty::ColorMapProperty(std::string identifier, std::string displayName,
                                   colorbrewer::Category selectedCategory,
                                   colorbrewer::Family selectedFamily, size_t numColors,
                                   InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , category("category", "Category")
    , colormap("colormap", "Colormap")
    , nColors("nColors", "Number of classes")
    , discrete_("discrete", "Discrete")
    , divergenceMidPoint_("midPoint", "Mid point", 0.5, 0., 1.) {
    using namespace colorbrewer;
    auto categories = {Category::Diverging, Category::Qualitative, Category::Sequential};
    for (auto cat : categories) {
        auto name = toString(cat);
        category.addOption(name, name, cat);
    }
    category.onChange([&]() {
        colormap.clearOptions();
        for (const auto& family : getFamiliesForCategory(
                 static_cast<colorbrewer::Category>(category.getSelectedValue()))) {
            auto familyName = toString(family);
            colormap.addOption(familyName, familyName, static_cast<colorbrewer::Family>(family));
        }
    });

    colormap.onChange([&]() {
        nColors.set(*nColors, getMinNumberOfColorsForFamily(*colormap),
                    getMaxNumberOfColorsForFamily(*colormap), 1);
    });
    category.setSelectedValue(selectedCategory);
    colormap.setSelectedValue(selectedFamily);
    nColors.set(numColors);
    category.setCurrentStateAsDefault();
    colormap.setCurrentStateAsDefault();

    divergenceMidPoint_.visibilityDependsOn(
        category, [](auto prop) -> bool { return *prop == Category::Diverging; });

    addProperty(category);
    addProperty(colormap);
    addProperty(nColors);
    addProperty(discrete_);
    addProperty(divergenceMidPoint_);
}

ColorMapProperty::ColorMapProperty(const ColorMapProperty& rhs)
    : CompositeProperty(rhs)
    , category(rhs.category)
    , colormap(rhs.colormap)
    , nColors(rhs.nColors)
    , discrete_(rhs.discrete_)
    , divergenceMidPoint_(rhs.divergenceMidPoint_) {
    addProperty(category);
    addProperty(colormap);
    addProperty(nColors);
    addProperty(discrete_);
    addProperty(divergenceMidPoint_);
}

ColorMapProperty* ColorMapProperty::clone() const { return new ColorMapProperty(*this); }

TransferFunction ColorMapProperty::get() const {
    TransferFunction tf;
    auto colors = colorbrewer::getColormap(*colormap, static_cast<glm::uint8>(*nColors));

    if (*category == colorbrewer::Category::Diverging) {
        // Mid point in [0 1]
        auto midPoint = (*divergenceMidPoint_ - divergenceMidPoint_.getMinValue()) /
                        (divergenceMidPoint_.getMaxValue() - divergenceMidPoint_.getMinValue());

        if (*discrete_) {
            auto dt = midPoint / (0.5 * (colors.size()));
            double start = 0, end = std::max(dt - std::numeric_limits<double>::epsilon(), 0.);
            for (auto i = 0; i < colors.size() / 2; i++) {
                tf.add(start, vec4(colors[i]));
                tf.add(end, vec4(colors[i]));
                start += dt;
                end += dt;
            }
            tf.add(start, vec4(colors[colors.size() / 2]));
            if (midPoint < 1.0) {
                dt = (1.0 - midPoint) / (0.5 * (colors.size()));
                tf.add(start + dt - std::numeric_limits<double>::epsilon(),
                       vec4(colors[colors.size() / 2]));
                start = start + dt;
                end = start + dt - std::numeric_limits<double>::epsilon();
                for (auto i = colors.size() / 2 + 1; i < colors.size(); i++) {
                    // Avoid numerical issues with min
                    tf.add(std::min(start, 1.0), vec4(colors[i]));
                    tf.add(std::min(end, 1.0), vec4(colors[i]));
                    start += dt;
                    end += dt;
                }
            }
        } else {
            auto dt = midPoint / (0.5 * (colors.size() - 1.0));
            for (auto i = 0; i < colors.size() / 2; i++) {
                tf.add(i * dt, vec4(colors[i]));
            }
            tf.add(midPoint, vec4(colors[colors.size() / 2]));
            if (midPoint < 1.0) {
                dt = (1.0 - midPoint) / (0.5 * (colors.size() - 1.0));
                auto t = midPoint + dt;
                for (auto i = colors.size() / 2 + 1; i < colors.size(); i++) {
                    // Avoid numerical issues with min
                    tf.add(std::min(t, 1.0), vec4(colors[i]));
                    t += dt;
                }
            }
        }

    } else {
        if (*discrete_) {
            double dt = 1.0 / (colors.size());
            double start = 0, end = dt - std::numeric_limits<double>::epsilon();
            for (const auto& c : colors) {
                tf.add(start, vec4(c));
                tf.add(end, vec4(c));
                start += dt;
                end += dt;
            }
        } else {
            auto dt = 1.0 / (colors.size() - 1.0);
            size_t idx = 0;
            for (const auto& c : colors) {
                tf.add(idx++ * dt, vec4(c));
            }
        }
    }
    return tf;
}  // namespace inviwo

}  // namespace inviwo
