/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <modules/basegl/processors/transferfunction2dtolayer.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/geometry/meshgl.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/image/layergl.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TransferFunction2DToLayer::processorInfo_{
    "org.inviwo.TransferFunction2DToLayer",       // Class identifier
    "Transfer Function 2D To Layer",              // Display name
    "Layer Operation",                            // Category
    CodeState::Experimental,                      // Code state
    Tags::GL | Tag{"TF"} | Tag{"Multispectral"},  // Tags
    R"(Creates a 2D transfer function in form of a Layer from a set of points. A TF
       primitive is represented by a 2D Gaussian centered around its position.
)"_unindentHelp,
};

const ProcessorInfo& TransferFunction2DToLayer::getProcessorInfo() const { return processorInfo_; }

namespace {

using BlendMode = TransferFunction2DToLayer::BlendMode;
using ColorMode = TransferFunction2DToLayer::ColorMode;
using PrimitiveFunc = TransferFunction2DToLayer::PrimitiveFunc;

OptionPropertyState<BlendMode> blendModeOptionState() {
    return OptionPropertyState<BlendMode>{
        .options = {{"premultiplied", "Pre-multiplied", BlendMode::PreMultiplied},
                    {"alphacompositing", "Alpha compositing", BlendMode::AlphaCompositing},
                    {"additive", "Additive", BlendMode::Additive}},
        .help = "Adjusts how the TF primitive are composited with the background."_help,
    };
}

OptionPropertyState<ColorMode> colorModeOptionState() {
    return OptionPropertyState<ColorMode>{
        .options = {{"reggular", "Regular", ColorMode::Regular},
                    {"premultiplied", "Pre-multiplied", ColorMode::Premultiplied}},
        .help = "Adjusts how the TF primitive color is weighted."_help,
    };
}

OptionPropertyState<PrimitiveFunc> primitiveOptionState() {
    return OptionPropertyState<PrimitiveFunc>{
        .options = {{"box", "Box (const)", PrimitiveFunc::Box},
                    {"linear", "Linear", PrimitiveFunc::Linear},
                    {"smooth", "Smooth (cubic)", PrimitiveFunc::Smooth},
                    {"gaussian", "Gaussian", PrimitiveFunc::Gaussian}},
        .help = "Weighting function for TF primitives based on the Euclidean distance."_help,
    };
}

}  // namespace

TransferFunction2DToLayer::TransferFunction2DToLayer()
    : Processor{}
    , inport_{"mesh",
              "Input mesh where each point and its radius corresponds to a 2D TF primitive."_help}
    , outport_{"layer",
               "Resulting Layer representing the 2D transfer function of the input mesh"_help}
    , tfResolution_{"tfResolution", "TF Resolution",
                    util::ordinalCount(1024, 16384)
                        .setMin(1)
                        .set("Resolution of the resulting 2D transfer function."_help)}
    , blendMode_{"blendMode", "Blend mode",
                 blendModeOptionState().setSelectedValue(BlendMode::PreMultiplied)}
    , colorMode_{"colorMode", "Color mode",
                 colorModeOptionState()
                     .set(InvalidationLevel::InvalidResources)
                     .setSelectedValue(ColorMode::Premultiplied)}
    , primitiveFunc_{"primitiveFunc", "Primitive Function",
                     primitiveOptionState()
                         .set(InvalidationLevel::InvalidResources)
                         .setSelectedValue(PrimitiveFunc::Gaussian)}

    , shaders_{{{ShaderType::Vertex, std::string{"tflayer2d.vert"}},
                {ShaderType::Geometry, std::string{"tflayer2d.geom"}},
                {ShaderType::Fragment, std::string{"tflayer2d.frag"}}},
               {
                   {BufferType::PositionAttrib, MeshShaderCache::Mandatory, "vec3"},
                   {BufferType::ColorAttrib, MeshShaderCache::Optional, "vec4"},
                   {BufferType::ScalarMetaAttrib, MeshShaderCache::Optional, "vec4"},
                   {BufferType::RadiiAttrib, MeshShaderCache::Optional, "float"},
                   {BufferType::IndexAttrib, MeshShaderCache::Optional, "int"},
                   {BufferType::PickingAttrib, MeshShaderCache::Optional, "uint"},
               },

               [&](Shader& shader) -> void {
                   shader.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
                   configureShader(shader);
               }} {

    addPorts(inport_, outport_);

    addProperties(tfResolution_, blendMode_, colorMode_, primitiveFunc_);
}

void TransferFunction2DToLayer::initializeResources() {
    for (auto& [state, shader] : shaders_.getShaders()) {
        configureShader(shader);
    }
}

void TransferFunction2DToLayer::configureShader(Shader& shader) {
    auto fragmentShader = shader.getFragmentShaderObject();

    std::string_view weightingFunc = [func = primitiveFunc_.get()]() -> std::string_view {
        switch (func) {
            case PrimitiveFunc::Box:
                return "evalBoxFunc";
            case PrimitiveFunc::Smooth:
                return "evalSmoothFunc";
            case PrimitiveFunc::Gaussian:
                return "evalGaussianFunc";
            case PrimitiveFunc::Linear:
                [[fallthrough]];
            default:
                return "evalLinearFunc";
        }
    }();
    fragmentShader->addShaderDefine("WEIGHTING_FUNC", weightingFunc);

    const std::string_view blendfunc = [&]() {
        switch (colorMode_.get()) {
            case ColorMode::Regular:
                return "blendRegular";
            case ColorMode::Premultiplied:
                [[fallthrough]];
            default:
                return "blendPremultiplied";
        }
    }();
    fragmentShader->addShaderDefine("BLEND_FUNC", blendfunc);

    shader.build();
}

void TransferFunction2DToLayer::process() {
    if (const auto newConfig = outputConfig(); config_ != newConfig) {
        cache_.clear();
        config_ = newConfig;
    }

    auto&& [fbo, layer] = [&]() -> decltype(auto) {
        auto unusedIt = std::ranges::find_if(
            cache_, [](const std::pair<FrameBufferObject, std::shared_ptr<Layer>>& item) {
                return item.second.use_count() == 1;
            });
        if (unusedIt != cache_.end()) {
            return *unusedIt;
        } else {
            auto& item = cache_.emplace_back(FrameBufferObject{}, std::make_shared<Layer>(config_));
            const auto* layerGL = item.second->getEditableRepresentation<LayerGL>();
            utilgl::Activate activateFbo{&item.first};
            item.first.attachColorTexture(layerGL->getTexture().get(), 0);
            return item;
        }
    }();

    const auto& mesh = inport_.getData();

    auto& shader = shaders_.getShader(*mesh);

    utilgl::Activate activateShader{&shader};
    utilgl::setShaderUniforms(shader, *layer, "outportParameters");
    utilgl::setShaderUniforms(shader, *mesh, "geometry");

    const auto dim = layer->getDimensions();
    utilgl::Activate activateFbo{&fbo};
    utilgl::clearCurrentTarget();

    utilgl::ViewportState viewport{0, 0, static_cast<GLsizei>(dim.x), static_cast<GLsizei>(dim.y)};
    utilgl::DepthMaskState depthMask{GL_FALSE};
    utilgl::GlBoolState depthTest{GL_DEPTH_TEST, false};
    utilgl::GlBoolState blend{GL_BLEND, true};

    auto state = [mode = blendMode_.get()]() {
        switch (mode) {
            case BlendMode::AlphaCompositing:
                return utilgl::BlendModeEquationState(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
                                                      GL_FUNC_ADD);
            case BlendMode::Additive:
                return utilgl::BlendModeEquationState(GL_ONE, GL_ONE, GL_FUNC_ADD);
            case BlendMode::PreMultiplied:
                [[fallthrough]];
            default:
                return utilgl::BlendModeEquationState(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD);
        }
    }();

    layer->getEditableRepresentation<LayerGL>();

    MeshDrawerGL::DrawObject drawer(*mesh);
    drawer.draw(MeshDrawerGL::DrawMode::Points);

    outport_.setData(layer);
}

LayerConfig TransferFunction2DToLayer::outputConfig() const {
    return LayerConfig{
        .dimensions = size2_t{static_cast<size_t>(tfResolution_)},
        .format = DataVec4Float32::get(),
        .swizzleMask = swizzlemasks::rgba,
        .interpolation = InterpolationType::Linear,
        .dataRange = dvec2{0.0, 1.0},
        .valueRange = dvec2{0.0, 1.0},
    };
}

}  // namespace inviwo
