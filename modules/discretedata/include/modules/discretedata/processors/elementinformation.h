/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <modules/discretedata/discretedatamoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <modules/discretedata/ports/datasetport.h>
#include <modules/discretedata/dataset.h>

namespace inviwo {
namespace discretedata {

/** \docpage{org.inviwo.ElementInformation, Element Information}
 * ![](org.inviwo.ElementInformation.png?classIdentifier=org.inviwo.ElementInformation)
 * Get data for a single element in a grid. Only up to 4 dimensions will be displayed.
 */
class IVW_MODULE_DISCRETEDATA_API ElementInformation : public Processor {
public:
    ElementInformation();
    virtual ~ElementInformation() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    DataSetInport dataIn_;
    DataSetOutport dataOut_;

    TemplateOptionProperty<GridPrimitive> primitive_;
    IntSizeTProperty elementIndex_;
    CompositeProperty elementInformation_;

    BoolCompositeProperty addHighlightChannel_;
    FloatProperty baseValue_, highlightValue_;
};

namespace detail {
// struct AddPropertyDispatcher {
//     template <typename T, ind N>
//     void operator()(const DataChannel<T, N>* channel, CompositeProperty* composite,
//                     ind elementIndex) {
//         // if constexpr (N == 1)
//         //     using Vec = T;
//         // else
//         //     using Vec = glm::vec<N, T>;
//         using Vec = inviwo::Vector<N, T>;
//         Vec value;
//         channel->fill(value, elementIndex);

//         std::string name = channel->getName();
//         OrdinalProperty<Vec>* prop =
//             dynamic_cast<OrdinalProperty<Vec>*>(composite->getPropertyByIdentifier(name));
//         std::cout << fmt::format("{}[{}] = {}", name, elementIndex, value) << std::endl;
//         if (!prop) {
//             prop = new OrdinalProperty<Vec>(name, name, value, value, value, value,
//                                             InvalidationLevel::Valid, PropertySemantics::Text);
//             composite->addProperties(prop);
//             // std::make_pair<Vec, ConstraintBehavior>(value, ConstraintBehavior::Ignore),
//             //     std::make_pair<Vec, ConstraintBehavior>(value, ConstraintBehavior::Ignore),
//         } else {
//             prop->set(value);
//             std::cout << "\tExisted, is readOnly? " << (prop->getReadOnly() ? "YES" : "NO")
//                       << std::endl;
//             if (prop->get() != value) std::cout << "\tSetting didn't work :(" << std::endl;
//         }
//         prop->setReadOnly(true);
//     }
// };

struct AddStringPropertyDispatcher {
    template <typename T, ind N>
    void operator()(const DataChannel<T, N>* channel, CompositeProperty* composite,
                    ind elementIndex) {
        using Vec = std::array<T, N>;  // inviwo::Vector<N, T>;
        Vec value;
        channel->fill(value, elementIndex);

        std::string name = channel->getName();
        std::stringstream str;
        str << "[" << value[0];
        for (size_t i = 1; i < N; ++i) {
            str << ", " << value[i];
        }
        str << "]";
        StringProperty* prop =
            dynamic_cast<StringProperty*>(composite->getPropertyByIdentifier(name));
        if (!prop) {
            prop = new StringProperty(name, name, str.str(), InvalidationLevel::Valid);
            composite->addProperties(prop);
        } else {
            prop->set(str.str());
        }
        prop->setReadOnly(true);
    }
};

/**
 *	Matches all scalar types that have an OrdinalProperty equivalent.
 */
template <typename Format>
struct OrdinalScalars : std::integral_constant<bool, Format::id() == DataFormatId::Float32 ||
                                                         Format::id() == DataFormatId::Float64 ||
                                                         Format::id() == DataFormatId::Int32 ||
                                                         Format::id() == DataFormat<size_t>::id()> {
};

/**
 *	Matches all scalar types that do NoT have an OrdinalProperty equivalent.
 */
template <typename Format>
struct NonOrdinalScalars
    : std::integral_constant<bool, Format::id() != DataFormatId::Float32 &&
                                       Format::id() != DataFormatId::Float64 &&
                                       Format::id() != DataFormatId::Int32 &&
                                       Format::id() != DataFormat<size_t>::id()> {};
}  // namespace detail

}  // namespace discretedata
}  // namespace inviwo
