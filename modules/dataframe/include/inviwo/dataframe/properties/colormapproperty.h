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

#pragma once

#include <inviwo/dataframe/dataframemoduledefine.h>
#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/core/datastructures/transferfunction.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/util/colorbrewer.h>

namespace inviwo {
/**
 * Continuous is suitable for ordered/sequential data.
 * Categorical is suitable for data where there should be no magnitude difference between classes,
 * such as volvo and audi.
 */
enum class ColormapType { Continuous, Categorical };

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& os,
                                             ColormapType colormap) {
    // clang-format off
    switch (colormap) {
        case ColormapType::Continuous: os << "Continuous"; break;
        case ColormapType::Categorical: os << "Categorical"; break;
    }
    // clang-format on
    return os;
}

/**
 * \brief Selection of pre-defined color maps based on data type.
 *
 * The following data types are supported:
 * Continuous: Ordered/Sequential data progressing from low to high
 * Continuous discrete: Constant inbetween colors to emphasize differences in the scale.
 * Continuous diverging: Emphasis on mid-point or critical values. Break in the middle.
 *
 * Categorical: Suitable for categorical/nominal data where there should be no magnitude difference
 * between classes. Is discrete and cannot be diverging so those options will be hidden.
 */
class IVW_MODULE_DATAFRAME_API ColormapProperty : public CompositeProperty {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    ColormapProperty(std::string identifier, std::string displayName,
                     ColormapType type = ColormapType::Continuous,
                     colorbrewer::Family family = colorbrewer::Family::Blues,
                     size_t numColors = getMinNumberOfColorsForFamily(colorbrewer::Family::Blues),
                     InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                     PropertySemantics semantics = PropertySemantics::Default);

    ColormapProperty(const ColormapProperty& rhs);
    virtual ColormapProperty* clone() const override;

    virtual ~ColormapProperty() = default;

    virtual std::string getClassIdentifierForWidget() const override {
        return CompositeProperty::classIdentifier;
    }

    // Get settings according to Colorbrewer style
    virtual colorbrewer::Category getCategory() const;
    virtual colorbrewer::Family getFamily() const;

    /**
     * Update settings based on Column.
     * Uses ColormapType::Categorical for CategoricalColumn and ColormapType::Continuous otherwise.
     * Divergence mid-point and its min-max range will be updated based on the Column values.
     * The mid-point will be set to the average of the column,
     * unless the min value is negative and the max value is positive in which case it
     * will be set to 0.
     */
    void setupForColumn(const Column& col);
    void setupForColumn(const Column& col, double minVal, double maxVal);

    /**
     * Get TransferFunction given current settings
     * @see colorbrewer::getTransferFunction
     */
    TransferFunction getTransferFunction() const;

    TemplateOptionProperty<ColormapType> type;
    TemplateOptionProperty<colorbrewer::Family> colormap;
    BoolProperty diverging;
    DoubleProperty divergenceMidPoint;
    BoolProperty discrete;
    IntSizeTProperty nColors;

private:
    void updateColormaps();
};

}  // namespace inviwo
