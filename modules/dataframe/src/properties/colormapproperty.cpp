/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <inviwo/core/datastructures/buffer/bufferram.h>                // for BufferRAM
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/transferfunction.h>                // for TransferFunction
#include <inviwo/core/network/networklock.h>                            // for NetworkLock
#include <inviwo/core/properties/boolproperty.h>                        // for BoolProperty
#include <inviwo/core/properties/compositeproperty.h>                   // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>                   // for InvalidationLevel
#include <inviwo/core/properties/optionproperty.h>                      // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>                     // for DoubleProperty
#include <inviwo/core/properties/propertysemantics.h>                   // for PropertySemantics
#include <inviwo/core/properties/templateproperty.h>                    // for TemplateProperty
#include <inviwo/core/util/colorbrewer.h>                               // for getTransferFunction
#include <inviwo/core/util/formatdispatching.h>                         // for Scalars
#include <inviwo/core/util/logcentral.h>                                // for LogCentral, LogWarn
#include <inviwo/core/util/staticstring.h>                              // for operator+
#include <inviwo/dataframe/datastructures/column.h>                     // for CategoricalColumn
#include <modules/base/algorithm/algorithmoptions.h>                    // for IgnoreSpecialValues
#include <modules/base/algorithm/dataminmax.h>                          // for bufferMinMax

#include <memory>         // for shared_ptr, share...
#include <ostream>        // for operator<<, basic...
#include <type_traits>    // for remove_extent_t
#include <unordered_set>  // for unordered_set

#include <glm/fwd.hpp>  // for uint8
#include <fmt/core.h>

namespace inviwo {

const std::string ColormapProperty::classIdentifier = "org.inviwo.ColormapProperty";
std::string ColormapProperty::getClassIdentifier() const { return classIdentifier; }

ColormapProperty::ColormapProperty(std::string_view identifier, std::string_view displayName,
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
            LogWarn(fmt::format(
                "Categories exceed maximum classes in colormap. {} will be used but {} classes are "
                "needed. Override colormap to provide your own colormap with more classes.",
                maxColors, numCategories));
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
        diverging.set(true);
    } else {
        divergenceMidPoint.set(0.5 * (minVal + maxVal), minVal, maxVal, 0.01 * (maxVal - minVal));
    }
}

TransferFunction ColormapProperty::getTransferFunction() const {
    auto midPoint = (divergenceMidPoint - divergenceMidPoint.getMinValue()) /
                    (divergenceMidPoint.getMaxValue() - divergenceMidPoint.getMinValue());
    return colorbrewer::getTransferFunction(getCategory(), *colormap,
                                            static_cast<glm::uint8>(*nColors), *discrete, midPoint);
}

void ColormapProperty::updateColormaps() {
    colormap.replaceOptions(getFamiliesForCategory(getCategory()));
    if (type == ColormapType::Categorical) {
        diverging = false;
        discrete = true;
    }
}

std::ostream& operator<<(std::ostream& os, ColormapType colormap) {
    switch (colormap) {
        case ColormapType::Continuous:
            os << "Continuous";
            break;
        case ColormapType::Categorical:
            os << "Categorical";
            break;
    }
    return os;
}

}  // namespace inviwo
