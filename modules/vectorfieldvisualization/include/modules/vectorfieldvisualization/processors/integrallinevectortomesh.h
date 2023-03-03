/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2023 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/vectorfieldvisualizationmoduledefine.h>  // for IVW_M...

#include <inviwo/core/ports/datainport.h>                                     // for DataI...
#include <inviwo/core/ports/meshport.h>                                       // for MeshO...
#include <inviwo/core/ports/outportiterable.h>                                // for Outpo...
#include <inviwo/core/processors/processor.h>                                 // for Proce...
#include <inviwo/core/processors/processorinfo.h>                             // for Proce...
#include <inviwo/core/properties/boolcompositeproperty.h>                     // for BoolC...
#include <inviwo/core/properties/boolproperty.h>                              // for BoolP...
#include <inviwo/core/properties/buttonproperty.h>                            // for Butto...
#include <inviwo/core/properties/compositeproperty.h>                         // for Compo...
#include <inviwo/core/properties/invalidationlevel.h>                         // for Inval...
#include <inviwo/core/properties/minmaxproperty.h>                            // for Doubl...
#include <inviwo/core/properties/optionproperty.h>                            // for Optio...
#include <inviwo/core/properties/ordinalproperty.h>                           // for Float...
#include <inviwo/core/properties/transferfunctionproperty.h>                  // for Trans...
#include <inviwo/core/util/glmvec.h>                                          // for vec4
#include <inviwo/core/util/staticstring.h>                                    // for opera...
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>         // for Brush...
#include <modules/vectorfieldvisualization/datastructures/integrallineset.h>  // for Integ...

#include <cstdint>        // for uint32_t
#include <functional>     // for __base
#include <string>         // for opera...
#include <string_view>    // for opera...
#include <unordered_map>  // for opera...
#include <vector>         // for opera...

#include <fmt/core.h>      // for format
#include <glm/gtx/io.hpp>  // for opera...

namespace inviwo {
class Deserializer;
class IntegralLine;
class Serializer;

class IVW_MODULE_VECTORFIELDVISUALIZATION_API IntegralLineVectorToMesh : public Processor {
public:
    class ColorByProperty : public CompositeProperty {
    public:
        friend class IntegralLineVectorToMesh;

        virtual std::string getClassIdentifier() const override;
        static const std::string classIdentifier;

        ColorByProperty(std::string_view identifier, std::string_view displayName,
                        InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput);

        ColorByProperty(const ColorByProperty& rhs);

        virtual ColorByProperty* clone() const override;
        virtual ~ColorByProperty();

        virtual void serialize(Serializer& s) const override;
        virtual void deserialize(Deserializer& d) override;

        std::string getKey() const;

    private:
        void addProperties();

        DoubleMinMaxProperty scaleBy_;
        BoolProperty loopTF_;
        DoubleProperty minValue_;
        DoubleProperty maxValue_;
        TransferFunctionProperty tf_;

        std::string key_;
    };

    enum class Output { Lines, Ribbons };

    enum class BrushBy { Nothing, LineIndex, VectorPosition };

    IntegralLineVectorToMesh();
    virtual ~IntegralLineVectorToMesh() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    void updatePropertyVisibility() {
        ribbonWidth_.setVisible(output_.get() == Output::Ribbons);
        brushBy_.setVisible(brushingList_.isConnected());
    }
    IntegralLineSetInport lines_;
    BrushingAndLinkingInport brushingList_;
    DataInport<std::vector<vec4>> colors_;
    MeshOutport mesh_;

    OptionProperty<BrushBy> brushBy_;

    OptionPropertyString colorBy_;

    IntSizeTProperty stride_;

    BoolCompositeProperty timeBasedFiltering_;
    DoubleMinMaxProperty minMaxT_;
    ButtonProperty setFromData_;

    OptionProperty<Output> output_;

    FloatProperty ribbonWidth_;

    FloatVec4Property selectedColor_;

    bool isFiltered(const IntegralLine& line, uint32_t idx) const;
    bool isSelected(const IntegralLine& line, uint32_t idx) const;

    void updateOptions();
};

}  // namespace inviwo
