/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023-2025 Inviwo Foundation
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

#include <modules/basegl/processors/imageprocessing/layershader.h>

#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/util/foreacharg.h>
#include <modules/opengl/shader/shaderresource.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/image/layergl.h>

namespace inviwo {

namespace detail {

constexpr std::string_view vertexShader = R"(out Fragment {
    smooth vec4 color;
    smooth vec2 texCoord;
} out_vert;

void main() {
    out_vert.color = in_Color;
    out_vert.texCoord = in_TexCoord.xy;
    gl_Position = in_Vertex;
}
)";

constexpr std::string_view defaultFrag = R"(#include "utils/structs.glsl"
#include "utils/sampler2d.glsl"

uniform sampler2D inport;
uniform ImageParameters outportParameters;

struct FormatScaling {
    float formatScaling;
    float formatOffset;
    float signedFormatScaling;
    float signedFormatOffset;
};

uniform FormatScaling destRange = FormatScaling(1.0, 0.0, 1.0, 0.0);

in Fragment {
    smooth vec4 color;
    smooth vec2 texCoord;
} in_frag;

void main() {
    // access normalized input value, then renormalize with respect to output data range
    vec4 value = getNormalizedTexel(inport, outportParameters, in_frag.texCoord);
    FragData0 = value / (1.0 - destRange.formatScaling) - destRange.formatOffset;

    // regular texture access, no output mapping
    // vec4 value = texture(inport, in_frag.texCoord);
    // FragData0 = value;
}
)";

}  // namespace detail

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LayerShader::processorInfo_{
    "org.inviwo.LayerShader",                 // Class identifier
    "Layer Shader",                           // Display name
    "Layer Operation",                        // Category
    CodeState::Experimental,                  // Code state
    Tags::GL | Tag{"Layer"} | Tag{"Shader"},  // Tags
    R"(Applies a customizable shader to perform computations on the input layer.)"_unindentHelp};

const ProcessorInfo& LayerShader::getProcessorInfo() const { return processorInfo_; }

LayerShader::LayerShader()
    : LayerGLProcessor(Shader{
          {{ShaderType::Vertex,
            std::make_shared<StringShaderResource>("layer_shader.vert", detail::vertexShader)}},
          Shader::Build::No})
    , fragmentShader_{std::make_shared<StringShaderResource>("layer_shader.frag",
                                                             detail::defaultFrag)}
    , fragmentShaderSource_{"fragmentShaderSource", "Fragment Shader",
                            std::string{detail::defaultFrag}, InvalidationLevel::InvalidResources,
                            PropertySemantics::ShaderEditor}
    , inputFormat_{"inputFormat", "Input Format", "Data format of the input layer"_help}
    , format_{"outputFormat", "Output Format",
              "Determines the data format of the output in combination with the number of "
              "channels. If set to `Same as Input`, the data format of the input layer is used "
              "instead with the exception of the number of channels."_help,
              [&]() {
                  std::vector<OptionPropertyOption<DataFormatId>> formats;
                  formats.emplace_back("asinput", "Same as Input", DataFormatId::NotSpecialized);
                  util::for_each_type<
                      std::tuple<DataFloat32, DataFloat64, DataInt8, DataInt16, DataInt32,
                                 DataInt64, DataUInt8, DataUInt16, DataUInt32, DataUInt64>>{}(
                      [&]<typename Format>() {
                          formats.emplace_back(Format::str(), Format::str(), Format::id());
                      });
                  return formats;
              }(),
              0}
    , channels_{"outputChannels", "Output Channels",
                []() {
                    OptionPropertyState<int> state{
                        .options = util::enumeratedOptions("Channels", 4, 1),
                        .selectedIndex = 0,
                        .help = "Number of channels in the output layer"_help};
                    state.options.emplace(state.options.begin(), "asinput", "Same as Input", 0);
                    return state;
                }()}

    , dataRange_{.comp{"dataRange", "Data Range"},
                 .input{"input", "Input", 0.0, 1.0, std::numeric_limits<double>::lowest(),
                        std::numeric_limits<double>::max(), 0.01, 0.0, InvalidationLevel::Valid,
                        PropertySemantics::Text},
                 .mode{"output",
                       "Output",
                       {{"asinput", "Same as Input", 0},
                        {"asformat", "Default For Output Format", 1},
                        {"custom", "Custom Range", 2}},
                       0},
                 .output{"custom", "Custom", 0.0, 1.0, std::numeric_limits<double>::lowest(),
                         std::numeric_limits<double>::max(), 0.01, 0.0,
                         InvalidationLevel::InvalidOutput, PropertySemantics::Text}}
    , valueRange_{.comp{"valueRange", "Value Range"},
                  .input{"input", "Input", 0.0, 1.0, std::numeric_limits<double>::lowest(),
                         std::numeric_limits<double>::max(), 0.01, 0.0, InvalidationLevel::Valid,
                         PropertySemantics::Text},
                  .mode{"output",
                        "Output",
                        {{"asinput", "Same as Input", 0},
                         {"asformat", "Default For Output Format", 1},
                         {"custom", "Custom Range", 2}},
                        0},
                  .output{"custom", "Custom", 0.0, 1.0, std::numeric_limits<double>::lowest(),
                          std::numeric_limits<double>::max(), 0.01, 0.0,
                          InvalidationLevel::InvalidOutput, PropertySemantics::Text}}
    , valueAxis_{.comp{"valueAxis", "Value Axis"},
                 .input{"input", "Input", std::array{std::string_view{""}, std::string_view{""}},
                        InvalidationLevel::Valid},
                 .mode{"output",
                       "Output",
                       {{"asinput", "Same as Input", 0}, {"custom", "Custom Axis", 2}},
                       0},
                 .output{"custom", "Custom"}}

{

    shader_.setShaderObject(ShaderType::Fragment, fragmentShader_);
    shader_.onReload([&]() { invalidate(InvalidationLevel::InvalidResources); });
    fragmentShaderSource_.onChange(
        [this]() { fragmentShader_->setSource(fragmentShaderSource_.get()); });

    dataRange_.comp.addProperties(dataRange_.input, dataRange_.mode, dataRange_.output);
    valueRange_.comp.addProperties(valueRange_.input, valueRange_.mode, valueRange_.output);
    valueAxis_.comp.addProperties(valueAxis_.input, valueAxis_.mode, valueAxis_.output);

    addProperties(fragmentShaderSource_, inputFormat_, format_, channels_, dataRange_.comp,
                  valueRange_.comp, valueAxis_.comp);

    inputFormat_.setReadOnly(true);
    dataRange_.input.setReadOnly(true);
    valueRange_.input.setReadOnly(true);
    valueAxis_.input.setReadOnly(true);

    valueAxis_.input.strings[0].setDisplayName("name").setCurrentStateAsDefault();
    valueAxis_.input.strings[1].setDisplayName("unit").setCurrentStateAsDefault();
    valueAxis_.output.strings[0].setDisplayName("name").setCurrentStateAsDefault();
    valueAxis_.output.strings[1].setDisplayName("unit").setCurrentStateAsDefault();

    inport_.onChange([this]() {
        if (inport_.hasData()) {
            inputFormat_.set(inport_.getData()->getDataFormat()->getString());
            dataRange_.input.set(inport_.getData()->dataMap.dataRange);
            valueRange_.input.set(inport_.getData()->dataMap.valueRange);
            valueAxis_.input.strings[0].set(inport_.getData()->dataMap.valueAxis.name);
            valueAxis_.input.strings[1].set(
                fmt::to_string(inport_.getData()->dataMap.valueAxis.unit));
        }
    });
}

void LayerShader::preProcess(TextureUnitContainer&, const Layer&, Layer& output) {
    utilgl::setShaderUniforms(shader_, output.dataMap, output.getDataFormat(), "destRange");
}

LayerConfig LayerShader::outputConfig(const Layer& input) const {
    auto inputFormat = inport_.getData()->getDataFormat();
    const auto* format = [&]() {
        const auto numericType = format_.get() == DataFormatId::NotSpecialized
                                     ? inputFormat->getNumericType()
                                     : DataFormatBase::get(format_)->getNumericType();
        const auto precision = format_.get() == DataFormatId::NotSpecialized
                                   ? inputFormat->getPrecision()
                                   : DataFormatBase::get(format_)->getPrecision();
        const auto channels =
            channels_.get() == 0 ? static_cast<int>(inputFormat->getComponents()) : channels_.get();
        return DataFormatBase::get(numericType, channels, precision);
    }();

    const auto dataRange = [&]() {
        if (dataRange_.mode.get() == 0) {
            return input.dataMap.dataRange;
        } else if (dataRange_.mode.get() == 1) {
            return DataMapper::defaultDataRangeFor(format);
        } else {
            return dataRange_.output.get();
        }
    };

    const auto valueRange = [&]() {
        if (valueRange_.mode.get() == 0) {
            return input.dataMap.valueRange;
        } else if (valueRange_.mode.get() == 1) {
            return DataMapper::defaultDataRangeFor(format);
        } else {
            return valueRange_.output.get();
        }
    };

    const auto valueAxis = [&]() {
        if (valueAxis_.mode.get() == 0) {
            return input.dataMap.valueAxis;
        } else {
            return Axis{.name = valueAxis_.output.strings[0].get(),
                        .unit = units::unit_from_string(valueAxis_.output.strings[1].get())};
        }
    };

    return input.config().updateFrom(
        {.format = format,
         .swizzleMask = swizzlemasks::defaultData(format->getComponents()),
         .valueAxis = valueAxis(),
         .dataRange = dataRange(),
         .valueRange = valueRange()});
}

}  // namespace inviwo
