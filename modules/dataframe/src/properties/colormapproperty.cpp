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
#include <inviwo/core/network/networklock.h>

#include <modules/base/algorithm/dataminmax.h>

namespace inviwo {

const std::string ColormapProperty::classIdentifier = "org.inviwo.ColormapProperty";
std::string ColormapProperty::getClassIdentifier() const { return classIdentifier; }

ColormapProperty::ColormapProperty(std::string identifier, std::string displayName,
                                   ColormapType selectedCategory,
                                   colorbrewer::Family selectedFamily, size_t numColors,
                                   InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , type("type", "Type",
           {{"continous", "Continuous", ColormapType::Continuous},
            {"categorical", "Categorical", ColormapType::Categorical}})
    , colormap("colormap", "Colormap")
    , diverging("diverging", "Diverging", false)
    , divergenceMidPoint("midPoint", "Mid point", 0.5, 0., 1.)
    , discrete("discrete", "Discrete")
    , nColors("nColors", "Classes") {
    using namespace colorbrewer;

    updateColormaps();
    type.onChange([&]() { updateColormaps(); });
    diverging.onChange([&]() { updateColormaps(); });

    colormap.onChange([&]() {
        nColors.set(*nColors, getMinNumberOfColorsForFamily(colormap),
                    getMaxNumberOfColorsForFamily(colormap), 1);
    });
    type.setSelectedValue(selectedCategory);
    colormap.setSelectedValue(selectedFamily);
    nColors.set(numColors, getMinNumberOfColorsForFamily(colormap),
                getMaxNumberOfColorsForFamily(colormap), 1);
    type.setCurrentStateAsDefault();
    colormap.setCurrentStateAsDefault();

    diverging.visibilityDependsOn(
        type, [](auto prop) -> bool { return prop == ColormapType::Continuous; });
    divergenceMidPoint.visibilityDependsOn(diverging, [](auto prop) -> bool { return prop; });
    // Always discrete for categorical data
    discrete.visibilityDependsOn(type, [](auto prop) { return prop == ColormapType::Continuous; });

    addProperty(type);
    addProperty(colormap);
    addProperty(diverging);
    addProperty(divergenceMidPoint);

    addProperty(discrete);
    addProperty(nColors);
}

ColormapProperty::ColormapProperty(const ColormapProperty& rhs)
    : CompositeProperty(rhs)
    , type(rhs.type)
    , colormap(rhs.colormap)
    , diverging(rhs.diverging)
    , divergenceMidPoint(rhs.divergenceMidPoint)
    , discrete(rhs.discrete)
    , nColors(rhs.nColors) {

    updateColormaps();
    type.onChange([&]() { updateColormaps(); });
    diverging.onChange([&]() { updateColormaps(); });

    colormap.onChange([&]() {
        nColors.set(*nColors, getMinNumberOfColorsForFamily(colormap),
                    getMaxNumberOfColorsForFamily(colormap), 1);
    });

    diverging.visibilityDependsOn(
        type, [](auto prop) -> bool { return prop == ColormapType::Continuous; });
    divergenceMidPoint.visibilityDependsOn(diverging, [](auto prop) -> bool { return prop; });
    // Always discrete for categorical data
    discrete.visibilityDependsOn(type, [](auto prop) { return prop == ColormapType::Continuous; });

    addProperty(type);
    addProperty(colormap);
    addProperty(diverging);
    addProperty(divergenceMidPoint);

    addProperty(discrete);
    addProperty(nColors);
}

ColormapProperty* ColormapProperty::clone() const { return new ColormapProperty(*this); }

colorbrewer::Category ColormapProperty::getCategory() const {
    colorbrewer::Category cat;
    switch (type) {
        case ColormapType::Categorical:
            cat = colorbrewer::Category::Qualitative;
            break;
        case ColormapType::Continuous:
            [[fallthrough]];
        default:
            cat = diverging ? colorbrewer::Category::Diverging : colorbrewer::Category::Sequential;
    }
    return cat;
}

colorbrewer::Family ColormapProperty::getFamily() const { return *colormap; }

void ColormapProperty::setupForColumn(const Column& col) {
    col.getBuffer()->getRepresentation<BufferRAM>()->dispatch<void, dispatching::filter::Scalars>(
        [&](auto ram) -> void {
            auto minMax = util::bufferMinMax(ram, IgnoreSpecialValues::Yes);
            setupForColumn(col, minMax.first.x, minMax.second.x);
        });
}
void ColormapProperty::setupForColumn(const Column& col, double minVal, double maxVal) {
    NetworkLock lock(this);

    if (auto catCol = dynamic_cast<const CategoricalColumn*>(&col)) {
        type.set(ColormapType::Categorical);
        colormap.set(colorbrewer::Family::Paired);
        auto maxColors = getMaxNumberOfColorsForFamily(colormap);
        auto numCategories = catCol->getCategories().size();
        if (maxColors < numCategories) {
            LogWarn("Categories exceed maximum classes in colormap. "
                    << maxColors << " will be used but " << numCategories
                    << " needed. Override colormap to provide your own colormap with more classes.")
        }
        nColors = numCategories;
        discrete = true;
    } else {
        type.set(ColormapType::Continuous);
        // Better contrast when using many classes
        nColors = getMaxNumberOfColorsForFamily(colormap);
        discrete = false;
    }
    if (minVal < 0 && maxVal > 0) {
        divergenceMidPoint.set(0, minVal, maxVal, 0.01 * (maxVal - minVal));
    } else {
        divergenceMidPoint.set(0.5 * (minVal + maxVal), minVal, maxVal, 0.01 * (maxVal - minVal));
    }
}

TransferFunction ColormapProperty::getTransferFunction() const {
    auto midPoint = (divergenceMidPoint - divergenceMidPoint.getMinValue()) /
                    (divergenceMidPoint.getMaxValue() - divergenceMidPoint.getMinValue());
    return colorbrewer::getTransferFunction(getCategory(), *colormap,
                                            static_cast<glm::uint8>(*nColors), *discrete, midPoint);
}  // namespace inviwo

void ColormapProperty::updateColormaps() {
    colormap.replaceOptions(getFamiliesForCategory(getCategory()));
    if (type == ColormapType::Categorical) {
        diverging = false;
        discrete = true;
    }
}

}  // namespace inviwo
