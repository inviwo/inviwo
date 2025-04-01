/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>

#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/ports/layerport.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/datastructures/unitsystem.h>

namespace inviwo {

class Layer;
class Volume;

/**
 * \ingroup properties
 * A CompositeProperty holding value axis name and unit for DataMapper holders like volumes and
 * layers, and optional overrides
 */
class IVW_MODULE_BASE_API ValueAxisProperty : public CompositeProperty {
public:
    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{"org.inviwo.ValueAxisProperty"};
    ValueAxisProperty(std::string_view identifier, std::string_view displayName,
                      bool customAxis = true,
                      InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                      PropertySemantics semantics = PropertySemantics::Default);
    ValueAxisProperty(std::string_view identifier, std::string_view displayName, VolumeInport& port,
                      bool customAxis = true,
                      InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                      PropertySemantics semantics = PropertySemantics::Default);
    ValueAxisProperty(std::string_view identifier, std::string_view displayName, LayerInport& port,
                      bool customAxis = true,
                      InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                      PropertySemantics semantics = PropertySemantics::Default);
    ValueAxisProperty(const ValueAxisProperty& rhs);
    virtual ValueAxisProperty* clone() const override;
    virtual ~ValueAxisProperty() = default;

    /**
     * \brief update value axis name and unit using \p volume
     */
    void updateFromVolume(std::shared_ptr<Volume> volume);
    /**
     * \brief update value axis name and unit using \p layer
     */
    void updateFromLayer(std::shared_ptr<Layer> layer);
    /**
     * \brief update the value axis name
     */
    void setValueName(std::string_view name);
    /**
     * \brief update the value axis unit
     */
    void setValueUnit(Unit unit);
    /**
     * \brief returns the currently selected value axis name
     *
     * @return custom value axis name if enabled, otherwise input value axis name
     */
    std::string_view getValueName() const;
    std::string_view getCustomValueName() const;
    /**
     * \brief returns the currently selected value range
     *
     * @return custom value axis unit if enabled, otherwise input value axis unit
     */
    Unit getValueUnit() const;
    Unit getCustomValueUnit() const;

    bool getCustomAxisEnabled() const;

private:
    const bool customAxis_;

    StringProperty valueName_;
    StringProperty valueUnit_;
    BoolCompositeProperty useCustomAxis_;
    StringProperty customValueName_;
    StringProperty customValueUnit_;
    ButtonProperty copyFromInput_;
};

}  // namespace inviwo
