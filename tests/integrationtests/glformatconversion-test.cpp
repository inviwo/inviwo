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

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/datamapper.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/logcentral.h>

#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>

#include <modules/opengl/openglcapabilities.h>
#include <modules/opengl/buffer/framebufferobject.h>
#include <modules/opengl/texture/texture2d.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/shader/stringshaderresource.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/glformats.h>
#include <modules/opengl/util/glformatutils.h>

#include <ranges>
#include <numeric>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#pragma optimize("", off)

namespace inviwo {

namespace {

constexpr std::string_view vertexShaderSource = R"(
out Fragment {
    smooth vec2 texCoord;
} out_vert;

void main() {
    out_vert.texCoord = in_TexCoord.xy;
    gl_Position = in_Vertex;
}
)";

constexpr std::string_view fragmentShaderSource = R"(
#include "utils/structs.glsl"
#include "utils/sampler2d.glsl"
#include "utils/conversion.glsl"

#if !defined(TEXSAMPLER)
#define sampler2D
#endif

uniform TEXSAMPLER source;
uniform ImageParameters sourceParameters;
uniform RangeConversionMap outputMap;

uniform int testOutput = 0;

in Fragment {
    smooth vec2 texCoord;
} in_frag;

void main() {
    float normalizedGL = //getNormalizedTexel(source, sourceParameters, in_frag.texCoord).x;
        (texture(source, in_frag.texCoord).x - sourceParameters.texToNormalized.offset) *
           sourceParameters.texToNormalized.scale;
    float valueSpace = //getValueTexel(source, sourceParameters, in_frag.texCoord).x;
        ((texture(source, in_frag.texCoord).x - sourceParameters.texToValue.inputOffset) *
            sourceParameters.texToValue.scale) + sourceParameters.texToValue.outputOffset;
    float outputGL = mapFromValueToGLOutput(valueSpace, outputMap);    

    float result = 0.0;
    if (testOutput == 2) {
        result = normalizedGL;
    } else if (testOutput == 1) {
        result = valueSpace;
    } else {
        result = outputGL;
    }

    //result = texture(source, in_frag.texCoord).x;

    FragData0 = vec4(result);
}
)";

constexpr std::string_view shaderoutputSource = R"(
#if !defined(OUTPUT)
#define OUTPUT vec4
#endif
#if !defined(NUM_VALUES)
#define NUM_VALUES 1
#endif

layout(location = 0) out OUTPUT FragData0;

uniform float values[NUM_VALUES];

void main() {
    int index = int(gl_FragCoord.x);

    FragData0 = OUTPUT(values[index]);
}
)";

template <typename T>
[[nodiscard]] std::shared_ptr<Layer> createLayer(LayerConfig config, const std::vector<T>& values) {
    config.updateFrom(LayerConfig{
        .dimensions = size2_t{values.size(), 1},
        .format = DataFormat<T>::get(),
        .interpolation = InterpolationType::Nearest,
    });

    auto layerRam = std::make_shared<LayerRAMPrecision<T>>(LayerReprConfig{
        .dimensions = config.dimensions,
        .format = config.format,
        .interpolation = config.interpolation,
    });
    std::ranges::copy(values, layerRam->getView().begin());

    auto layer = std::make_shared<Layer>(config);
    layer->addRepresentation(layerRam);
    return layer;
}

template <typename T>
[[nodiscard]] std::shared_ptr<Layer> createLayer(LayerConfig config, size_t dim) {
    return createLayer(std::move(config), std::vector<T>(dim, T{0}));
}

}  // namespace

class GLSLShaderOutputTest : public ::testing::Test {
public:
    GLSLShaderOutputTest()
        : shader_{{{ShaderType::Vertex,
                    std::make_shared<StringShaderResource>("vertex.vert", vertexShaderSource)},
                   {ShaderType::Fragment, std::make_shared<StringShaderResource>(
                                              "shaderoutput.frag", shaderoutputSource)}},
                  Shader::Build::No} {}

protected:
    void render(const std::vector<float>& sourceValues, Layer& output);

    template <typename U>
    std::vector<U> test(const std::vector<float>& sourceValues);

private:
    Shader shader_;

    FrameBufferObject fbo_;
};

class GLFormatConversionTest : public ::testing::Test {
public:
    enum class TestOutput : std::uint8_t { GLOutput, ValueSpace, NormalizedInput };

    GLFormatConversionTest()
        : shader_{{{ShaderType::Vertex,
                    std::make_shared<StringShaderResource>("vertex.vert", vertexShaderSource)},
                   {ShaderType::Fragment,
                    std::make_shared<StringShaderResource>("fragment.frag", fragmentShaderSource)}},
                  Shader::Build::No} {}

protected:
    void render(const Layer& input, Layer& output, TestOutput testOutput);

    template <typename T, typename U>
    std::vector<T> test(const LayerConfig& sourceConfig, const std::vector<U>& sourceValues,
                        const LayerConfig& destConfig, TestOutput testOutput);

private:
    Shader shader_;

    FrameBufferObject fbo_;
};

void GLSLShaderOutputTest::render(const std::vector<float>& sourceValues, Layer& output) {
    shader_.getFragmentShaderObject()->addShaderDefine("NUM_VALUES",
                                                       std::to_string(sourceValues.size()));

    if (const auto& glFormat = GLFormats::get(output.getDataFormat()->getId());
        glFormat.layoutQualifier.ends_with("ui")) {
        shader_.getFragmentShaderObject()->addShaderDefine("OUTPUT", "uvec4");
    } else if (glFormat.layoutQualifier.ends_with("i")) {
        shader_.getFragmentShaderObject()->addShaderDefine("OUTPUT", "ivec4");
    } else {
        shader_.getFragmentShaderObject()->addShaderDefine("OUTPUT", "vec4");
    }

    shader_.getFragmentShaderObject()->clearOutDeclarations();
    shader_.build();

    const auto* outputLayerGL = output.getEditableRepresentation<LayerGL>();
    fbo_.activate();
    fbo_.attachColorTexture(outputLayerGL->getTexture().get(), 0);

    const ivec2 dims{output.getDimensions()};
    glViewport(0, 0, dims.x, dims.y);

    shader_.activate();
    shader_.setUniform("values", sourceValues);

    glDisable(GL_BLEND);
    utilgl::singleDrawImagePlaneRect();

    fbo_.deactivate();
}

template <typename U>
std::vector<U> GLSLShaderOutputTest::test(const std::vector<float>& sourceValues) {
    auto output = createLayer<U>({}, sourceValues.size());

    render(sourceValues, *output);

    const auto* layerRam = output->template getRepresentation<LayerRAM>();
    std::vector<U> result;
    for (auto index : std::views::iota(0) | std::views::take(sourceValues.size())) {
        const auto val = layerRam->getAsDouble(size2_t{index, 0});
        result.push_back(static_cast<U>(val));
    }
    return result;
}

void GLFormatConversionTest::render(const Layer& input, Layer& output, TestOutput testOutput) {
    if (const auto& glFormat = GLFormats::get(input.getDataFormat()->getId());
        glFormat.layoutQualifier.ends_with("ui")) {
        shader_.getFragmentShaderObject()->addShaderDefine("TEXSAMPLER", "usampler2D");
    } else if (glFormat.layoutQualifier.ends_with("i")) {
        shader_.getFragmentShaderObject()->addShaderDefine("TEXSAMPLER", "isampler2D");
    } else {
        shader_.getFragmentShaderObject()->addShaderDefine("TEXSAMPLER", "sampler2D");
    }
    shader_.build();

    const auto* outputLayerGL = output.getEditableRepresentation<LayerGL>();
    fbo_.activate();
    fbo_.attachColorTexture(outputLayerGL->getTexture().get(), 0);

    const ivec2 dims{output.getDimensions()};
    glViewport(0, 0, dims.x, dims.y);

    shader_.activate();

    TextureUnitContainer cont;
    utilgl::bindAndSetUniforms(shader_, cont, input, "source");
    utilgl::setShaderUniforms(
        shader_, utilgl::createGLOutputConversion(output.dataMap, output.getDataFormat()),
        "outputMap");
    shader_.setUniform("testOutput", static_cast<int>(testOutput));

    glDisable(GL_BLEND);
    utilgl::singleDrawImagePlaneRect();

    fbo_.deactivate();
}

template <typename T, typename U>
std::vector<T> GLFormatConversionTest::test(const LayerConfig& sourceConfig,
                                            const std::vector<U>& sourceValues,
                                            const LayerConfig& destConfig, TestOutput testOutput) {
    auto input = createLayer<U>(sourceConfig, sourceValues);
    auto output = createLayer<T>(destConfig, sourceValues.size());

    render(*input, *output, testOutput);

    const auto* layerRam = output->template getRepresentation<LayerRAM>();

    std::vector<T> result;
    for (auto index : std::views::iota(0) | std::views::take(sourceValues.size())) {
        const auto val = layerRam->getAsDouble(size2_t{index, 0});
        result.push_back(static_cast<T>(val));
    }

    return result;

    // for (auto&& [index, expected] : std::views::zip(std::views::iota(0), expectedValues)) {
    //     const auto result = layerRam->getAsDouble(size2_t{index, 0});
    //     if constexpr (std::is_floating_point_v<U>) {
    //         EXPECT_DOUBLE_EQ(result, expected);
    //     } else {
    //         const auto res = static_cast<U>(result);
    //         EXPECT_EQ(res, expected);
    //     }
    // }
}

// GLSL shader output
TEST_F(GLSLShaderOutputTest, Float32) {
    const std::vector<float> values{0.0f, -1.0f, 1.0f, 0.5f, -3.355f, 1515.2f};
    auto result = test<float>(values);

    EXPECT_EQ(result, values);
}

TEST_F(GLSLShaderOutputTest, Int8) {
    constexpr std::int8_t max = std::numeric_limits<std::int8_t>::max();
    [[maybe_unused]] const std::int8_t min = OpenGLCapabilities::isSignedIntNormalizationSymmetric()
                                                 ? -max
                                                 : std::numeric_limits<std::int8_t>::min();

    const std::vector<float> values{0.0f, -1.0f, 1.0f, 0.5f, -2.0f, 1.5f};
    // NOTE: negative values are clamped to zero (confirmed on NVIDIA)
    const std::vector<int> expected{0, 0, max, 63, 0, max};

    auto result = test<std::int8_t>(values);
    const std::vector<int> resultInt{result.begin(), result.end()};
    EXPECT_EQ(resultInt, expected);
}

TEST_F(GLSLShaderOutputTest, Int16) {
    constexpr std::int16_t max = std::numeric_limits<std::int16_t>::max();
    [[maybe_unused]] const std::int16_t min =
        OpenGLCapabilities::isSignedIntNormalizationSymmetric()
            ? -max
            : std::numeric_limits<std::int16_t>::min();

    const std::vector<float> values{0.0f, -1.0f, 1.0f, 0.5f, -2.0f, 1.5f};
    // NOTE: negative values are clamped to zero (confirmed on NVIDIA)
    const std::vector<std::int16_t> expected{0, 0, max, 16383, 0, max};

    auto result = test<std::int16_t>(values);
    EXPECT_EQ(result, expected);
}

TEST_F(GLSLShaderOutputTest, Int32) {
    const std::vector<float> values{0.0f, -1.0f, 1.0f, -10.0f, -3.0f, 1515.0f};
    const std::vector<std::int32_t> expected{0, -1, 1, -10, -3, 1515};

    auto result = test<std::int32_t>(values);
    EXPECT_EQ(result, expected);
}

TEST_F(GLSLShaderOutputTest, UInt8) {
    constexpr std::uint8_t max = std::numeric_limits<std::uint8_t>::max();

    const std::vector<float> values{0.0f, -1.0f, 1.0f, 0.5f, -2.0f, 1.5f};
    const std::vector<unsigned int> expected{0, 0, max, 127, 0, max};

    auto result = test<std::uint8_t>(values);
    const std::vector<unsigned int> resultUInt{result.begin(), result.end()};
    EXPECT_EQ(resultUInt, expected);
}

TEST_F(GLSLShaderOutputTest, UInt16) {
    constexpr std::uint16_t max = std::numeric_limits<std::uint16_t>::max();

    const std::vector<float> values{0.0f, -1.0f, 1.0f, 0.5f, -2.0f, 1.5f};
    const std::vector<std::uint16_t> expected{0, 0, max, 32767, 0, max};

    auto result = test<std::uint16_t>(values);
    EXPECT_EQ(result, expected);
}

TEST_F(GLSLShaderOutputTest, UInt32) {
    const std::vector<float> values{0.0f, -1.0f, 1.0f, 1515.0f};
    const std::vector<std::uint32_t> expected{0, 0, 1, 1515};

    auto result = test<std::uint32_t>(values);
    EXPECT_EQ(result, expected);
}

// GLSL Shader output conversion
TEST_F(GLFormatConversionTest, OutputFloat32) {
    const LayerConfig config{
        .dataRange = dvec2{0.0, 1.0},
        .valueRange = dvec2{0.0, 1.0},
    };
    const std::vector<float> values{0.0f, 0.5f, 1.0f, -1.0f};

    auto result = test<float>(config, values, config, TestOutput::GLOutput);
    EXPECT_EQ(result, values);
}

TEST_F(GLFormatConversionTest, OutputFloat32Symmetric) {
    const LayerConfig config{
        .dataRange = dvec2{-1.0, 1.0},
        .valueRange = dvec2{-1.0, 1.0},
    };
    const std::vector<float> values{0.0f, 0.5f, 1.0f, 1.5f, -1.5f};

    auto result = test<float>(config, values, config, TestOutput::GLOutput);
    EXPECT_EQ(result, values);
}

TEST_F(GLFormatConversionTest, OutputFloat32SignedToPositive) {
    const LayerConfig sourceConfig{
        .dataRange = dvec2{-1.0, 1.0},
        .valueRange = dvec2{-1.0, 1.0},
    };
    const LayerConfig destConfig{
        .dataRange = dvec2{0.0, 1.0},
        .valueRange = dvec2{0.0, 2.0},
    };
    const std::vector<float> values{0.0f, 1.0f, -0.5f, 3.0f};
    const std::vector<float> expected{0.0f, 0.5f, -0.25f, 1.5f};

    auto result = test<float>(sourceConfig, values, destConfig, TestOutput::GLOutput);
    EXPECT_EQ(result, expected);
}

TEST_F(GLFormatConversionTest, OutputInt16) {
    using enum DataMapper::SignedNormalization;

    const DataFormatBase* dstFormat = DataFormatBase::get(DataFormatId::Int16);

    const LayerConfig sourceConfig{
        .dataRange = dvec2{0.0, 1.0},
        .valueRange = dvec2{0.0, 1.0},
    };
    const LayerConfig destConfig{
        .dataRange = DataMapper::defaultDataRangeFor(dstFormat, Symmetric),
        .valueRange = dvec2{0.0, 10.0},
    };

    const glm::vec<2, std::int16_t> range{destConfig.dataRange.value()};

    const std::vector<float> values{0.0f, 5.0f, 10.0f};
    // NOTE: negative values are clamped to zero before renormalization (confirmed on NVIDIA)
    // meaning 0.0f will not be mapped to range.x
    const std::vector<std::int16_t> expected{0 /* range.x */, 0, range.y};

    auto result = test<std::int16_t>(sourceConfig, values, destConfig, TestOutput::GLOutput);
    EXPECT_EQ(result, expected);
}

TEST_F(GLFormatConversionTest, OutputInt16Symmetric) {
    using enum DataMapper::SignedNormalization;

    const DataFormatBase* dstFormat = DataFormatBase::get(DataFormatId::Int16);

    LayerConfig sourceConfig{
        .dataRange = dvec2{0.0, 1.0},
        .valueRange = dvec2{0.0, 1.0},
    };
    LayerConfig destConfig{
        .dataRange = DataMapper::defaultDataRangeFor(dstFormat, Symmetric),
        .valueRange = dvec2{-10.0, 10.0},
    };

    const glm::vec<2, std::int16_t> range{destConfig.dataRange.value()};

    const std::vector<float> values{-10.0f, 0.0f, 10.0f};
    // NOTE: negative values are clamped to zero before renormalization (confirmed on NVIDIA)
    // meaning -10.0f will not be mapped to range.x
    const std::vector<std::int16_t> expected{0 /* range.x */, 0, range.y};

    auto result = test<std::int16_t>(sourceConfig, values, destConfig, TestOutput::GLOutput);
    EXPECT_EQ(result, expected);
}

TEST_F(GLFormatConversionTest, OutputInt16Positive) {
    using enum DataMapper::SignedNormalization;

    const DataFormatBase* dstFormat = DataFormatBase::get(DataFormatId::Int16);

    const LayerConfig sourceConfig{
        .dataRange = dvec2{0.0, 1.0},
        .valueRange = dvec2{0.0, 1.0},
    };
    const LayerConfig destConfig{
        .dataRange = dvec2{0.0, dstFormat->getMax()},
        .valueRange = dvec2{0.0, 10.0},
    };

    const glm::vec<2, std::int16_t> range{destConfig.dataRange.value()};

    const std::vector<float> values{-10.0f, 0.0f, 10.0f};
    const std::vector<std::int16_t> expected{range.x, 0, range.y};

    auto result = test<std::int16_t>(sourceConfig, values, destConfig, TestOutput::GLOutput);
    EXPECT_EQ(result, expected);
}

// Normalized texel fetch
TEST_F(GLFormatConversionTest, NormalizedFloat32) {
    const DataMapper dataMap{dvec2{-4.0, 4.0}};
    const LayerConfig config{
        .dataRange = dataMap.dataRange,
    };
    const std::vector<float> values{0.0f, -4.0f, 4.0f, 2.0f};
    std::vector<float> expected;
    std::ranges::transform(values, std::back_inserter(expected), [&dataMap](auto val) {
        return static_cast<float>(dataMap.mapFromDataToNormalized(val));
    });

    auto result = test<float>(config, values, {}, TestOutput::NormalizedInput);
    EXPECT_EQ(result, expected);
}

TEST_F(GLFormatConversionTest, NormalizedInt16) {
    const DataMapper dataMap{dvec2{-4.0, 4.0}};
    const LayerConfig config{
        .dataRange = dataMap.dataRange,
    };
    const std::vector<std::int16_t> values{0, -4, 4, 2};
    std::vector<float> expected;
    std::ranges::transform(values, std::back_inserter(expected), [&dataMap](auto val) {
        return static_cast<float>(dataMap.mapFromDataToNormalized(val));
    });

    auto result = test<float>(config, values, {}, TestOutput::NormalizedInput);
    EXPECT_EQ(result, expected);
}

TEST_F(GLFormatConversionTest, NormalizedInt32) {
    const DataMapper dataMap{dvec2{-4.0, 4.0}};
    const LayerConfig config{
        .dataRange = dataMap.dataRange,
    };

    const std::vector<std::int32_t> values{0, -4, 4, 2, std::numeric_limits<std::int32_t>::max()};
    std::vector<float> expected;
    std::ranges::transform(values, std::back_inserter(expected), [&dataMap](auto val) {
        return static_cast<float>(dataMap.mapFromDataToNormalized(val));
    });

    auto result = test<float>(config, values, {}, TestOutput::NormalizedInput);
    EXPECT_EQ(result, expected);
}

TEST_F(GLFormatConversionTest, NormalizedUInt16) {
    using enum DataMapper::SignedNormalization;

    const DataMapper dataMap{
        DataMapper::defaultDataRangeFor(DataFormatBase::get(DataFormatId::UInt16), Symmetric),
        dvec2{-4.0, 4.0}};
    const LayerConfig config{
        .dataRange = dataMap.dataRange,
        .valueRange = dataMap.valueRange,
    };
    const std::vector<std::uint16_t> values{0, std::numeric_limits<std::uint16_t>::max(),
                                            std::numeric_limits<std::uint16_t>::max() / 2, 49152};
    std::vector<float> expected;
    std::ranges::transform(values, std::back_inserter(expected), [&dataMap](auto val) {
        return static_cast<float>(dataMap.mapFromDataToNormalized(val));
    });

    auto result = test<float>(config, values, {}, TestOutput::NormalizedInput);
    EXPECT_EQ(result, expected);
}

TEST_F(GLFormatConversionTest, NormalizedUInt32) {
    using enum DataMapper::SignedNormalization;

    const DataMapper dataMap{
        DataMapper::defaultDataRangeFor(DataFormatBase::get(DataFormatId::UInt32), Symmetric),
        dvec2{-4.0, 4.0}};
    LayerConfig config{
        .dataRange = dataMap.dataRange,
        .valueRange = dataMap.valueRange,
    };

    const std::vector<std::uint32_t> values{0, std::numeric_limits<std::uint32_t>::max(),
                                            std::numeric_limits<std::uint32_t>::max() / 2, 1572864};
    std::vector<float> expected;
    std::ranges::transform(values, std::back_inserter(expected), [&dataMap](auto val) {
        return static_cast<float>(dataMap.mapFromDataToNormalized(val));
    });

    auto result = test<float>(config, values, {}, TestOutput::NormalizedInput);
    EXPECT_EQ(result, expected);
}

// Value space
TEST_F(GLFormatConversionTest, ValueSpaceFloat32) {
    const LayerConfig config{
        .dataRange = dvec2{-4.0, 4.0},
        .valueRange = dvec2{-4.0, 4.0},
    };
    const std::vector<float> values{0.0f, -4.0f, 4.0f, 2.0f};

    auto result = test<float>(config, values, {}, TestOutput::ValueSpace);
    EXPECT_EQ(result, values);
}

TEST_F(GLFormatConversionTest, ValueSpaceInt16) {
    const LayerConfig config{
        .dataRange = dvec2{-4.0, 4.0},
        .valueRange = dvec2{-4.0, 4.0},
    };
    const std::vector<std::int16_t> values{0, -4, 4, 2};
    const std::vector<float> expected{0.0f, -4.0f, 4.0f, 2.0f};

    auto result = test<float>(config, values, {}, TestOutput::ValueSpace);
    for (auto&& [res, expect] : std::views::zip(result, expected)) {
        EXPECT_NEAR(res, expect, 1.0e-8f);
    }
}

TEST_F(GLFormatConversionTest, ValueSpaceInt32) {
    const LayerConfig config{
        .dataRange = dvec2{-4.0, 4.0},
        .valueRange = dvec2{-4.0, 4.0},
    };
    const std::vector<std::int32_t> values{0, -4, 4, 2};
    const std::vector<float> expected{0.0f, -4.0f, 4.0f, 2.0f};

    auto result = test<float>(config, values, {}, TestOutput::ValueSpace);
    EXPECT_EQ(result, expected);
}

TEST_F(GLFormatConversionTest, ValueSpaceUInt16) {
    const LayerConfig config{
        .dataRange = dvec2{0.0, 16.0},
        .valueRange = dvec2{-4.0, 4.0},
    };
    const std::vector<std::uint16_t> values{0, 16, 8, 4};
    const std::vector<float> expected{-4.0f, 4.0f, 0.0f, -2.0f};

    auto result = test<float>(config, values, {}, TestOutput::ValueSpace);
    for (auto&& [res, expect] : std::views::zip(result, expected)) {
        EXPECT_NEAR(res, expect, 1.0e-8f);
    }
}

TEST_F(GLFormatConversionTest, ValueSpaceUInt32) {
    const LayerConfig config{
        .dataRange = dvec2{0.0, 16.0},
        .valueRange = dvec2{-4.0, 4.0},
    };
    const std::vector<std::uint32_t> values{0, 16, 8, 4};
    const std::vector<float> expected{-4.0f, 4.0f, 0.0f, -2.0f};

    auto result = test<float>(config, values, {}, TestOutput::ValueSpace);
    EXPECT_EQ(result, expected);
}

}  // namespace inviwo
