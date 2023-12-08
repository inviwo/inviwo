/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2023 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>  // for IVW_MODULE_BASE_API

#include <inviwo/core/ports/volumeport.h>                  // for VolumeInport
#include <inviwo/core/ports/layerport.h>
#include <inviwo/core/properties/boolcompositeproperty.h>  // for BoolCompositeProperty
#include <inviwo/core/properties/buttonproperty.h>         // for ButtonProperty
#include <inviwo/core/properties/compositeproperty.h>      // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>      // for InvalidationLevel, Invalidatio...
#include <inviwo/core/properties/minmaxproperty.h>         // for DoubleMinMaxProperty
#include <inviwo/core/properties/propertysemantics.h>      // for PropertySemantics, PropertySem...
#include <inviwo/core/util/glmvec.h>                       // for dvec2

#include <memory>       // for shared_ptr
#include <string>       // for string
#include <string_view>  // for string_view

namespace inviwo {

class Layer;
class Volume;

/**
 * \ingroup properties
 * A CompositeProperty holding data and value ranges for volumes and optional custom range overrides
 */
class IVW_MODULE_BASE_API DataRangeProperty : public CompositeProperty {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;
    DataRangeProperty(std::string_view identifier, std::string_view displayName,
                      bool customRanges = true,
                      InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                      PropertySemantics semantics = PropertySemantics::Default);
    DataRangeProperty(std::string_view identifier, std::string_view displayName, VolumeInport& port,
                      bool customRanges = true,
                      InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                      PropertySemantics semantics = PropertySemantics::Default);
    DataRangeProperty(std::string_view identifier, std::string_view displayName, LayerInport& port,
                      bool customRanges = true,
                      InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                      PropertySemantics semantics = PropertySemantics::Default);
    DataRangeProperty(const DataRangeProperty& rhs);
    virtual DataRangeProperty* clone() const override;
    virtual ~DataRangeProperty() = default;

    /**
     * \brief update both value and data ranges using \p volume
     */
    void updateFromVolume(std::shared_ptr<Volume> volume);
    /**
     * \brief update both value and data ranges using \p layer
     */
    void updateFromLayer(std::shared_ptr<Layer> layer);
    /**
     * \brief update the data range
     */
    void setDataRange(const dvec2& range);
    /**
     * \brief update the value range
     */
    void setValueRange(const dvec2& range);
    /**
     * \brief returns the currently selected data range
     *
     * @return custom data range if enabled, otherwise input data range
     */
    dvec2 getDataRange() const;
    dvec2 getCustomDataRange() const;
    /**
     * \brief returns the currently selected value range
     *
     * @return custom value range if enabled, otherwise input value range
     */
    dvec2 getValueRange() const;
    dvec2 getCustomValueRange() const;

    bool getCustomRangeEnabled() const;

private:
    const bool customRanges_;

    DoubleMinMaxProperty dataRange_;
    DoubleMinMaxProperty valueRange_;
    BoolCompositeProperty useCustomRange_;
    DoubleMinMaxProperty customDataRange_;
    DoubleMinMaxProperty customValueRange_;
    ButtonProperty copyFromInput_;
};

}  // namespace inviwo
