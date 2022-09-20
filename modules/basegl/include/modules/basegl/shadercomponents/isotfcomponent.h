/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2022 Inviwo Foundation
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

#include <inviwo/core/ports/volumeport.h>                     // for VolumeInport
#include <inviwo/core/properties/invalidationlevel.h>         // for InvalidationLevel, Invalida...
#include <inviwo/core/properties/isotfproperty.h>             // for IsoTFProperty
#include <inviwo/core/properties/isovalueproperty.h>          // for IsoValueProperty
#include <inviwo/core/properties/property.h>                  // for Property
#include <inviwo/core/properties/transferfunctionproperty.h>  // for TransferFunctionProperty
#include <inviwo/core/util/stdextensions.h>                   // for make_array
#include <inviwo/core/util/stringconversion.h>                // for StrBuffer
#include <inviwo/core/util/zip.h>                             // for enumerate
#include <modules/basegl/shadercomponents/shadercomponent.h>  // for ShaderComponent::Segment
#include <modules/opengl/shader/shader.h>                     // for Shader
#include <modules/opengl/shader/shaderobject.h>               // for ShaderObject
#include <modules/opengl/texture/textureunit.h>               // for TextureUnit, TextureUnitCon...
#include <modules/opengl/image/layergl.h>                     // IWYU pragma: keep

#include <algorithm>    // for max
#include <array>        // for array
#include <cstddef>      // for size_t
#include <string>       // for string
#include <string_view>  // for string_view
#include <vector>       // for vector

#include <fmt/core.h>  // for format

namespace inviwo {

/**
 * Adds a set of `N` IsoTFProperties, and binds them to uniforms in the shader.
 */
template <size_t N>
class IsoTFComponent : public ShaderComponent {
public:
    IsoTFComponent(VolumeInport& volumeInport)
        : ShaderComponent(), isotfs{util::make_array<N>([&]([[maybe_unused]] size_t i) {
            if constexpr (N > 1) {
                auto prop =
                    IsoTFProperty{fmt::format("isotf{}", i), fmt::format("TF & Iso Values #{}", i),
                                  &volumeInport, InvalidationLevel::InvalidResources};
                prop.isovalues_.setIdentifier(fmt::format("isovalues{}", i))
                    .setDisplayName(fmt::format("Iso Values #{}", i));
                prop.tf_.setIdentifier(fmt::format("transferFunction{}", i))
                    .setDisplayName(fmt::format("Transfer Function #{}", i));
                return prop;
            } else {
                return IsoTFProperty{"isotf", "TF & Iso Values", &volumeInport,
                                     InvalidationLevel::InvalidResources};
            }
        })} {
        static_assert(N > 0, "there has to be at least one isotf");
    }

    virtual std::string_view getName() const override { return isotfs[0].getIdentifier(); }

    virtual void process(Shader& shader, TextureUnitContainer& cont) override {
        for (auto&& isotf : isotfs) {
            if (auto tfLayer = isotf.tf_.get().getData()) {
                TextureUnit& unit = cont.emplace_back();
                auto transferFunctionGL = tfLayer->template getRepresentation<LayerGL>();
                transferFunctionGL->bindTexture(unit.getEnum());
                shader.setUniform(isotf.tf_.getIdentifier(), unit);
            }
            {
                const auto positions = isotf.isovalues_.get().getPositionsf();
                const auto colors = isotf.isovalues_.get().getColors();
                const auto name = isotf.isovalues_.getIdentifier();
                StrBuffer buff;
                shader.setUniform(buff.replace("{}.values", name), positions);
                shader.setUniform(buff.replace("{}.colors", name), colors);
                shader.setUniform(buff.replace("{}.size", name), static_cast<int>(colors.size()));
            }
        }
    }

    virtual void initializeResources(Shader& shader) override {
        // need to ensure there is always at least one isovalue due to the use of the macro
        // as array size in IsovalueParameters
        size_t isoCount = 1;
        for (auto&& isotf : isotfs) {
            isoCount = std::max(isoCount, isotf.isovalues_.get().size());
        }
        shader.getFragmentShaderObject()->addShaderDefine("MAX_ISOVALUE_COUNT",
                                                          fmt::format("{}", isoCount));
    }

    virtual std::vector<Property*> getProperties() override {
        std::vector<Property*> props;
        for (auto&& isotf : isotfs) {
            props.push_back(&isotf);
        }

        return props;
    }

    virtual std::vector<Segment> getSegments() override {
        std::vector<Segment> segments;

        for (auto&& [i, isotf] : util::enumerate(isotfs)) {
            segments.push_back(Segment{
                fmt::format("uniform IsovalueParameters {};", isotf.isovalues_.getIdentifier()),
                placeholder::uniform, 1000 + 1});
            segments.push_back(
                Segment{fmt::format("uniform sampler2D {};", isotf.tf_.getIdentifier()),
                        placeholder::uniform, 1050 + 1});
        }

        return segments;
    }

    std::array<IsoTFProperty, N> isotfs;
};

}  // namespace inviwo
