/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2018 Inviwo Foundation
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

//#include <modules/vectorfieldvisualization/vectorvisnetworkconverter.h>
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

/** \docpage{org.inviwo.IntegralLineVectorToMesh, Integral Line Vector To Mesh}
 * ![](org.inviwo.IntegralLineVectorToMesh.png?classIdentifier=org.inviwo.IntegralLineVectorToMesh)
 * Explanation of how to use the processor.
 *
 * ### Inports
 *   * __<Inport1>__ <description>.
 *
 * ### Outports
 *   * __<Outport1>__ <description>.
 *
 * ### Properties
 *   * __<Prop1>__ <description>.
 *   * __<Prop2>__ <description>
 */
class IVW_MODULE_VECTORFIELDVISUALIZATION_API IntegralLineVectorToMesh : public Processor {
public:
    class ColorByPropertiy : public CompositeProperty {
    public:
        friend class IntegralLineVectorToMesh;

        InviwoPropertyInfo();

        ColorByPropertiy(std::string identifier, std::string displayName,
                         InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput)
            : CompositeProperty(identifier, displayName, invalidationLevel)
            , scaleBy_("scaleBy", "Data Range (for normalization)", 0, 1, 0, 1, 0.01)
            , loopTF_("loopTF", "Loop Transfer Function", false)
            , minValue_("minValue", "Min " + displayName, 0, std::numeric_limits<double>::lowest(),
                        std::numeric_limits<double>::max(), 0.01, InvalidationLevel::InvalidOutput,
                        PropertySemantics::Text)
            , maxValue_("maxValue", "Max " + displayName, 0, std::numeric_limits<double>::lowest(),
                        std::numeric_limits<double>::max(), 0.01, InvalidationLevel::InvalidOutput,
                        PropertySemantics::Text)
            , tf_("transferFunction", "Transfer function")
            , key_(displayName)

        {
            addProperties();
        }

        ColorByPropertiy(const ColorByPropertiy& rhs)
            : CompositeProperty(rhs)
            , scaleBy_(rhs.scaleBy_)
            , loopTF_(rhs.loopTF_)
            , minValue_(rhs.minValue_)
            , maxValue_(rhs.maxValue_)
            , tf_(rhs.tf_)

        {
            addProperties();
        }

        ColorByPropertiy& operator=(const ColorByPropertiy& that) {
            if (this != &that) {
                scaleBy_ = that.scaleBy_;
                loopTF_ = that.loopTF_;
                minValue_ = that.minValue_;
                maxValue_ = that.maxValue_;
                tf_ = that.tf_;
            }
            return *this;
        }
        virtual ColorByPropertiy* clone() const override { return new ColorByPropertiy(*this); }
        virtual ~ColorByPropertiy() {}

        virtual void serialize(Serializer& s) const override {
            CompositeProperty::serialize(s);
            s.serialize("key", key_);
        }
        virtual void deserialize(Deserializer& d) override {
            CompositeProperty::deserialize(d);
            d.deserialize("key", key_);
        }

        std::string getKey() const { return key_; }

    private:
        void addProperties() {
            addProperty(scaleBy_);
            addProperty(loopTF_);
            addProperty(minValue_);
            addProperty(maxValue_);
            addProperty(tf_);

            tf_.get().clearPoints();
            tf_.get().addPoint(0.0, vec4(0, 0, 1, 1));
            tf_.get().addPoint(0.5, vec4(1, 1, 0, 1));
            tf_.get().addPoint(1.0, vec4(1, 0, 0, 1));
            tf_.setCurrentStateAsDefault();
        }

        DoubleMinMaxProperty scaleBy_;
        BoolProperty loopTF_;
        DoubleProperty minValue_;
        DoubleProperty maxValue_;
        TransferFunctionProperty tf_;

        std::string key_;
    };

    enum class Output { Lines, Ribbons };

    enum class BrushBy { Never, LineIndex, VectorPosition };

    IntegralLineVectorToMesh();
    virtual ~IntegralLineVectorToMesh() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
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

    bool isFiltered(const IntegralLine& line, size_t idx) const {
        switch (brushBy_.get()) {
            case BrushBy::LineIndex:
                return brushingList_.isFiltered(line.getIndex());
            case BrushBy::VectorPosition:
                return brushingList_.isFiltered(idx);
            case BrushBy::Never:
            default:
                return false;
        }
    }

    bool isSelected(const IntegralLine& line, size_t idx) const {
        switch (brushBy_.get()) {
            case BrushBy::LineIndex:
                return brushingList_.isSelected(line.getIndex());
            case BrushBy::VectorPosition:
                return brushingList_.isSelected(idx);
            case BrushBy::Never:
            default:
                return true;
        }
    }

    void updateOptions() {
        auto lines = lines_.getData();
        if (lines->size() == 0) return;
        std::string selected = "";
        int selectedIndex = -1;
        int velocityIndex = -1;
        if (colorBy_.size() > 0) {
            selected = colorBy_.getSelectedIdentifier();
            if (selected == "portIndex") {
                selectedIndex = colorBy_.size() - 2;
            }

            if (selected == "portNumber") {
                selectedIndex = colorBy_.size() - 1;
            }
        }

        colorBy_.clearOptions();

        int i = 0;
        for (const auto& key : lines->front().getMetaDataKeys()) {
            if (key == selected) {
                selectedIndex = i;
            }
            if (key == "velocity") {
                velocityIndex = i;
            }
            colorBy_.addOption(key, key);

            if (!getPropertyByIdentifier(key)) {
                auto prop = std::make_unique<ColorByPropertiy>(key, key);
                auto propPtr = prop.get();
                prop->setVisible(selectedIndex == i);
                prop->setSerializationMode(PropertySerializationMode::All);
                addProperty(prop.release());
                colorBy_.onChange([=]() { propPtr->setVisible(i == colorBy_.getSelectedIndex()); });
            }

            i++;
        }

        if (colors_.isConnected()) {
            colorBy_.addOption("portIndex", "Colors in port (line index)");
            colorBy_.addOption("portNumber", "Colors in port (line vector position)");
        }

        colorBy_.setCurrentStateAsDefault();
        if (selectedIndex != -1) {
            colorBy_.setSelectedIndex(selectedIndex);
        } else if (velocityIndex != -1) {
            colorBy_.setSelectedIndex(velocityIndex);
        }
    }

    template <typename T>
    static double metadataToDouble(const T& t) {
        return static_cast<double>(t);
    }

    template <typename T, glm::precision P, template <typename, glm::precision> class G>
    static double metadataToDouble(const G<T, P>& glm) {
        using F = typename std::conditional<std::is_same<T, float>::value, float, double>::type;
        return glm::length(util::glm_convert<G<F, P>>(glm));
    }
};

}  // namespace inviwo

#endif  // IVW_INTEGRALLINEVECTORTOMESH_H
