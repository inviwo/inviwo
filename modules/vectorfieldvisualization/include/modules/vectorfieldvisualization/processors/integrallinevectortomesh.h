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

#ifndef IVW_INTEGRALLINEVECTORTOMESH_H
#define IVW_INTEGRALLINEVECTORTOMESH_H

#include <modules/vectorfieldvisualization/vectorfieldvisualizationmoduledefine.h>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/ports/imageport.h>

#include <modules/vectorfieldvisualization/datastructures/integralline.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/ports/meshport.h>

#include <modules/vectorfieldvisualization/datastructures/integrallineset.h>
#include <inviwo/core/properties/boolproperty.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/compositeproperty.h>

namespace inviwo {

class IVW_MODULE_VECTORFIELDVISUALIZATION_API IntegralLineVectorToMesh : public Processor {
public:
    class ColorByProperty : public CompositeProperty {
    public:
        friend class IntegralLineVectorToMesh;

        virtual std::string getClassIdentifier() const override;
        static const std::string classIdentifier;

        ColorByProperty(std::string identifier, std::string displayName,
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

    TemplateOptionProperty<BrushBy> brushBy_;

    OptionPropertyString colorBy_;

    IntSizeTProperty stride_;

    BoolCompositeProperty timeBasedFiltering_;
    DoubleMinMaxProperty minMaxT_;
    ButtonProperty setFromData_;

    TemplateOptionProperty<Output> output_;

    FloatProperty ribbonWidth_;

    FloatVec4Property selectedColor_;

    bool isFiltered(const IntegralLine& line, size_t idx) const;
    bool isSelected(const IntegralLine& line, size_t idx) const;

    void updateOptions();
};

}  // namespace inviwo

#endif  // IVW_INTEGRALLINEVECTORTOMESH_H
