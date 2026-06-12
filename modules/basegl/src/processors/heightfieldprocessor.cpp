/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2026 Inviwo Foundation
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

#include <modules/basegl/processors/heightfieldprocessor.h>

#include <inviwo/core/algorithm/boundingbox.h>
#include <inviwo/core/algorithm/markdown.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/processors/processorstate.h>
#include <inviwo/core/processors/processortags.h>
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <modules/opengl/geometry/meshgl.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/stringshaderresource.h>

#include <memory>
#include <string_view>

namespace inviwo {

namespace {

constexpr std::string_view vertexShader = R"(#include "utils/structs.glsl"
#include "utils/sampler2d.glsl"

uniform GeometryParameters geometry;
uniform CameraParameters camera;

uniform sampler2D heightfield;
uniform ImageParameters heightfieldParameters;

uniform float heightScale = 1.0f;

out Vertex {
    vec4 worldPos;
    vec3 normal;
    vec4 color;
    vec3 texCoord;
} vertex;

void main() {    
    vec4 position = in_Vertex;
    vec3 normal = normalize(in_Normal);
    vec4 color = in_Color;
    vec3 texCoord = in_TexCoord;    
    
    float offset = getValueTexel(heightfield, heightfieldParameters, texCoord.xy).r;
    // displace vertex along the normal
    position.xyz += normal * offset * heightScale;

    vertex.worldPos = geometry.dataToWorld * position;
    vertex.normal = geometry.dataToWorldNormalMatrix * normal;
    vertex.color = color;
    vertex.texCoord = texCoord;

    gl_Position = camera.worldToClip * vertex.worldPos;
}
)";

}  // namespace

const ProcessorInfo HeightFieldProcessor::processorInfo_{
    "org.inviwo.HeightFieldRenderGL",  // Class identifier
    "Height Field Renderer",           // Display name
    "Heightfield",                     // Category
    CodeState::Stable,                 // Code state
    Tags::GL,                          // Tags
    R"(
        Maps a height field onto a geometry and renders it to an image.
        
        ![](file:~modulePath~/docs/images/heightfield-network.png)
        
        Example Network:
        [core/heightfield.inv](file:~basePath~/data/workspaces/heightfield.inv)
    )"_unindentHelp};

const ProcessorInfo& HeightFieldProcessor::getProcessorInfo() const { return processorInfo_; }

HeightFieldProcessor::HeightFieldProcessor()
    : Processor()
    , inport_{"geometry", "Input geometry which is modified by the height field"_help}
    , inportHeightfield_{"heightfield", R"(
        The height field input (single-channel layer).
        If the layer has multiple channels only the red channel is used.)"_unindentHelp}
    , inportTexture_{"colorTexture", "Color texture for color mapping (optional)."_help,
                     OutportDeterminesSize::Yes}
    , inportNormalMap_{"normalmap", "Normal map input (optional)"_help, OutportDeterminesSize::Yes}
    , imageInport_{"imageInport", "Background image (optional)"_help}
    , outport_{"image", "The rendered height field."_help}
    , heightScale_{"heightScale", "Height Scale",
                   util::ordinalLength(1.0f, 10.0f)
                       .setInc(0.001)
                       .set("Scaling factor for the height field"_help)}
    , terrainShadingMode_(
          "terrainShadingMode", "Terrain Shading",
          "Defines the color mapped onto the height field using either constant color, color input texture, or the height field texture"_help,
          {{"shadingConstant", "Constant Color", HeightFieldShading::ConstantColor},
           {"shadingColorTex", "Color Texture", HeightFieldShading::ColorTexture},
           {"shadingHeightField", "Heightfield Texture", HeightFieldShading::HeightField}},
          0)
    , vertexShaderSource_{"vertexShaderSource", "Vertex Shader", vertexShader,
                          InvalidationLevel::InvalidResources, PropertySemantics::ShaderEditor}
    , camera_("camera", "Camera", util::boundingBox(inport_))
    , trackball_(&camera_)
    , lightingProperty_("lighting", "Lighting", &camera_)
    , vertexShader_{std::make_shared<StringShaderResource>("heightfield.vert", vertexShader)}
    , shader_({{ShaderType::Vertex, vertexShader_},
               {ShaderType::Fragment, utilgl::findShaderResource("heightfield.frag")}},
              Shader::Build::No) {

    addPort(inport_);
    addPort(inportHeightfield_).setOptional(true);
    addPort(inportTexture_).setOptional(true);
    addPort(inportNormalMap_).setOptional(true);
    addPort(imageInport_).setOptional(true);
    addPort(outport_);

    addProperties(heightScale_, terrainShadingMode_, vertexShaderSource_, camera_,
                  lightingProperty_, trackball_);

    vertexShaderSource_.onChange([this]() { vertexShader_->setSource(vertexShaderSource_.get()); });

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

HeightFieldProcessor::~HeightFieldProcessor() = default;

void HeightFieldProcessor::initializeResources() {
    utilgl::addShaderDefines(shader_, lightingProperty_);
    shader_.build();
}

void HeightFieldProcessor::process() {
    utilgl::activateTargetAndClearOrCopySource(outport_, imageInport_);

    shader_.activate();

    // bind input textures
    TextureUnitContainer cont;

    int terrainShadingMode = terrainShadingMode_.get();
    if (inportHeightfield_.isReady()) {
        utilgl::bindAndSetUniforms(shader_, cont, inportHeightfield_);
    } else if (terrainShadingMode == HeightFieldShading::HeightField) {
        // switch to flat shading since color texture is not available
        terrainShadingMode = HeightFieldShading::ConstantColor;
    }

    if (inportTexture_.isReady()) {
        utilgl::bindAndSetUniforms(shader_, cont, inportTexture_, ImageType::ColorOnly);
    } else if (terrainShadingMode == HeightFieldShading::ColorTexture) {
        // switch to flat shading since heightfield texture is not available
        terrainShadingMode = HeightFieldShading::ConstantColor;
    }

    const bool normalMapping = inportNormalMap_.isReady();
    if (normalMapping && inportNormalMap_.isReady()) {
        utilgl::bindAndSetUniforms(shader_, cont, inportNormalMap_, ImageType::ColorOnly);
    }

    shader_.setUniform("terrainShadingMode", terrainShadingMode);
    shader_.setUniform("normalMapping", (normalMapping ? 1 : 0));

    utilgl::setUniforms(shader_, camera_, lightingProperty_, heightScale_);

    for (auto mesh : inport_) {
        utilgl::setShaderUniforms(shader_, *mesh, "geometry");
        MeshDrawerGL::DrawObject drawer{mesh->getRepresentation<MeshGL>(),
                                        mesh->getDefaultMeshInfo()};
        drawer.draw();
    }

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
