/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#include <modules/basegl/processors/lightvolumegl.h>

#include <inviwo/core/datastructures/coordinatetransformer.h>           // for StructuredCoordin...
#include <inviwo/core/datastructures/image/layer.h>                     // for Layer
#include <inviwo/core/datastructures/light/baselightsource.h>           // for LightSourceType
#include <inviwo/core/datastructures/light/directionallight.h>          // for DirectionalLight
#include <inviwo/core/datastructures/light/pointlight.h>                // for PointLight
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/transferfunction.h>                // for TransferFunction
#include <inviwo/core/datastructures/volume/volume.h>                   // for Volume
#include <inviwo/core/ports/datainport.h>                               // for DataInport
#include <inviwo/core/ports/outportiterable.h>                          // for OutportIterable
#include <inviwo/core/ports/volumeport.h>                               // for VolumeInport, Vol...
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tags, Tags::GL
#include <inviwo/core/properties/boolproperty.h>                        // for BoolProperty
#include <inviwo/core/properties/invalidationlevel.h>                   // for InvalidationLevel
#include <inviwo/core/properties/optionproperty.h>                      // for OptionPropertyInt
#include <inviwo/core/properties/transferfunctionproperty.h>            // for TransferFunctionP...
#include <inviwo/core/util/formats.h>                                   // for DataFormatBase
#include <inviwo/core/util/glmvec.h>                                    // for vec3, size3_t, vec4
#include <inviwo/core/util/logcentral.h>                                // for LogCentral
#include <modules/opengl/buffer/framebufferobject.h>                    // for FrameBufferObject
#include <modules/opengl/geometry/meshgl.h>                             // for MeshGL
#include <modules/opengl/image/layergl.h>                               // for LayerGL
#include <modules/opengl/openglutils.h>                                 // for DepthFuncState
#include <modules/opengl/shader/shader.h>                               // for Shader
#include <modules/opengl/shader/shaderobject.h>                         // for ShaderObject
#include <modules/opengl/sharedopenglresources.h>                       // for SharedOpenGLResou...
#include <modules/opengl/texture/texture3d.h>                           // for Texture3D
#include <modules/opengl/texture/textureunit.h>                         // for TextureUnit
#include <modules/opengl/texture/textureutils.h>                        // for multiDrawImagePla...
#include <modules/opengl/volume/volumegl.h>                             // for VolumeGL
#include <modules/opengl/volume/volumeutils.h>                          // for setShaderUniforms

#include <cmath>          // for acos, M_PI
#include <cstddef>        // for size_t
#include <functional>     // for __base
#include <string_view>    // for string_view
#include <type_traits>    // for remove_extent_t
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set
#include <numbers>

#include <fmt/core.h>                 // for format_to, basic_...
#include <glm/common.hpp>             // for abs
#include <glm/geometric.hpp>          // for dot, length, norm...
#include <glm/mat4x4.hpp>             // for operator*, mat<>:...
#include <glm/matrix.hpp>             // for inverse
#include <glm/vec3.hpp>               // for operator*, operator-
#include <glm/vec4.hpp>               // for operator*, operator+
#include <glm/vector_relational.hpp>  // for any, notEqual

namespace inviwo {

const ProcessorInfo LightVolumeGL::processorInfo_{
    "org.inviwo.LightVolumeGL",  // Class identifier
    "Light Volume",              // Display name
    "Illumination",              // Category
    CodeState::Experimental,     // Code state
    Tags::GL,                    // Tags
};
const ProcessorInfo& LightVolumeGL::getProcessorInfo() const { return processorInfo_; }

const vec4 LightVolumeGL::borderColor_{1.f, 1.f, 1.f, 1.f};

static const int faceAxis_[6] = {0, 0, 1, 1, 2, 2};

static const vec3 faceNormals_[6] = {vec3(1.f, 0.f, 0.f), vec3(-1.f, 0.f, 0.f),
                                     vec3(0.f, 1.f, 0.f), vec3(0.f, -1.f, 0.f),
                                     vec3(0.f, 0.f, 1.f), vec3(0.f, 0.f, -1.f)};

// Defines permutation axis based on face index for chosen propagation axis.
inline void definePermutationMatrices(mat4& permMat, mat4& permLightMat, int permFaceIndex) {
    permMat = mat4(0.f);
    permMat[3][3] = 1.f;

    // Permutation of x and y
    switch (faceAxis_[permFaceIndex]) {
        case 0:
            permMat[0][2] = 1.f;
            permMat[1][1] = 1.f;
            break;

        case 1:
            permMat[0][0] = 1.f;
            permMat[1][2] = 1.f;
            break;

        case 2:
            permMat[0][0] = 1.f;
            permMat[1][1] = 1.f;
            break;
    }

    permLightMat = permMat;
    // Permutation of z
    const float closestAxisZ = (faceNormals_[permFaceIndex])[faceAxis_[permFaceIndex]];
    permMat[2][faceAxis_[permFaceIndex]] = closestAxisZ;
    permLightMat[2][faceAxis_[permFaceIndex]] = glm::abs<float>(closestAxisZ);

    // For reverse axis-aligned
    if (closestAxisZ < 0.f) {
        permMat[3][faceAxis_[permFaceIndex]] = 1.f;
    }
}

LightVolumeGL::LightVolumeGL()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , lightSource_("lightSource")
    , supportColoredLight_("supportColoredLight", "Support Light Color", false)
    , volumeSizeOption_("volumeSizeOption", "Light Volume Size")
    , transferFunction_("transferFunction", "Transfer function", &inport_)
    , floatPrecision_("floatPrecision", "Float Precision", false)
    , propagationShader_("lighting/lightpropagation.vert", "lighting/lightpropagation.geom",
                         "lighting/lightpropagation.frag")
    , mergeShader_("lighting/lightvolumeblend.vert", "lighting/lightvolumeblend.geom",
                   "lighting/lightvolumeblend.frag")
    , lightColor_(1.f)
    , propParams_{{{supportColoredLight_ ? lightColor_ : borderColor_},
                   {supportColoredLight_ ? lightColor_ : borderColor_}}}
    , mergeFBO_{}
    , internalVolumesInvalid_(false)
    , volumeDimOut_(0)
    , lightDir_(0.f)
    , lightPos_(0.f)
    , calculatedOnes_(false) {

    addPort(inport_);
    addPort(outport_);
    addPort(lightSource_);
    supportColoredLight_.onChange([this]() { supportColoredLightChanged(); });
    addProperty(supportColoredLight_);
    volumeSizeOption_.addOption("1", "Full of incoming volume", 1);
    volumeSizeOption_.addOption("1/2", "Half of incoming volume", 2);
    volumeSizeOption_.addOption("1/4", "Quarter of incoming volume", 4);
    volumeSizeOption_.setSelectedIndex(1);
    volumeSizeOption_.setCurrentStateAsDefault();
    volumeSizeOption_.onChange([this]() { volumeSizeOptionChanged(); });
    addProperty(volumeSizeOption_);
    addProperty(transferFunction_);
    floatPrecision_.onChange([this]() { floatPrecisionChanged(); });
    addProperty(floatPrecision_);

    propagationShader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    mergeShader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    supportColoredLightChanged();
}

void LightVolumeGL::process() {
    bool lightColorChanged = false;

    if (lightSource_.isChanged()) {
        lightColorChanged = lightSourceChanged();
    }

    bool reattach = false;

    if (internalVolumesInvalid_ || lightColorChanged || inport_.isChanged()) {
        reattach = volumeChanged(lightColorChanged);
    }

    auto* outVolumeGL = volume_->getEditableRepresentation<VolumeGL>();
    const TextureUnit volUnit;
    const auto* inVolumeGL = inport_.getData()->getRepresentation<VolumeGL>();
    inVolumeGL->bindTexture(volUnit.getEnum());
    const TextureUnit transFuncUnit;
    const auto* tfLayer = transferFunction_.getRepresentation<LayerGL>();
    tfLayer->bindTexture(transFuncUnit.getEnum());

    const TextureUnit lightVolUnit[2];
    glActiveTexture(lightVolUnit[0].getEnum());
    propParams_[0].tex.bind();
    glActiveTexture(lightVolUnit[1].getEnum());
    propParams_[1].tex.bind();
    glActiveTexture(GL_TEXTURE0);

    propagationShader_.activate();
    propagationShader_.setUniform("volume_", volUnit.getUnitNumber());
    utilgl::setShaderUniforms(propagationShader_, *inport_.getData(), "volumeParameters_");
    propagationShader_.setUniform("transferFunc_", transFuncUnit.getUnitNumber());
    propagationShader_.setUniform("lightVolumeParameters_.dimensions", volumeDimOutF_);
    propagationShader_.setUniform("lightVolumeParameters_.reciprocalDimensions", volumeDimOutFRCP_);

    {
        const auto* rect = SharedOpenGLResources::getPtr()->imagePlaneRect();
        const utilgl::Enable<MeshGL> enable(rect);
        const utilgl::DepthFuncState depth(GL_ALWAYS);

        // Perform propagation passes
        for (int i = 0; i < 2; ++i) {
            propParams_[i].fbo.activate();
            glViewport(0, 0, static_cast<GLsizei>(volumeDimOut_.x),
                       static_cast<GLsizei>(volumeDimOut_.y));

            if (reattach) {
                propParams_[i].fbo.attachColorTexture(&propParams_[i].tex, 0);
            }

            propagationShader_.setUniform("lightVolume_", lightVolUnit[i].getUnitNumber());
            propagationShader_.setUniform("permutationMatrix_", propParams_[i].axisPermutation);

            if (lightSource_.getData()->getLightSourceType() == LightSourceType::Point) {
                propagationShader_.setUniform("lightPos_", lightPos_);
                propagationShader_.setUniform("permutedLightMatrix_",
                                              propParams_[i].axisPermutationLight);
            } else {
                propagationShader_.setUniform("permutedLightDirection_",
                                              propParams_[i].permutedLightDirection);
            }

            for (unsigned int z = 0; z < volumeDimOut_.z; ++z) {
                glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                                          GL_TEXTURE_3D, propParams_[i].tex.getID(), 0, z);
                propagationShader_.setUniform("sliceNum_", static_cast<GLint>(z));
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                glFlush();
            }

            propParams_[i].fbo.deactivate();
        }
    }

    propagationShader_.deactivate();
    mergeShader_.activate();
    mergeShader_.setUniform("lightVolume_", lightVolUnit[0].getUnitNumber());
    mergeShader_.setUniform("lightVolumeSec_", lightVolUnit[1].getUnitNumber());
    mergeShader_.setUniform("lightVolumeParameters_.dimensions", volumeDimOutF_);
    mergeShader_.setUniform("lightVolumeParameters_.reciprocalDimensions", volumeDimOutFRCP_);
    mergeShader_.setUniform("permMatInv_", propParams_[0].axisPermutationINV);
    mergeShader_.setUniform("permMatInvSec_", propParams_[1].axisPermutationINV);
    mergeShader_.setUniform("blendingFactor_", blendingFactor_);
    // Perform merge pass
    mergeFBO_.activate();
    glViewport(0, 0, static_cast<GLsizei>(volumeDimOut_.x), static_cast<GLsizei>(volumeDimOut_.y));

    if (reattach) mergeFBO_.attachColorTexture(outVolumeGL->getTexture().get(), 0);

    utilgl::multiDrawImagePlaneRect(static_cast<int>(volumeDimOut_.z));
    mergeShader_.deactivate();
    mergeFBO_.deactivate();
}

bool LightVolumeGL::lightSourceChanged() {
    vec3 color = vec3(1.f);
    vec3 lightDirection = vec3(0.f);

    switch (lightSource_.getData()->getLightSourceType()) {
        case LightSourceType::Directional: {
            if (lightType_ != LightSourceType::Directional) {
                lightType_ = LightSourceType::Directional;
                propagationShader_.getFragmentShaderObject()->removeShaderDefine("POINT_LIGHT");
                propagationShader_.getFragmentShaderObject()->build();
                propagationShader_.link();
            }

            if (const auto* directionLight =
                    dynamic_cast<const DirectionalLight*>(lightSource_.getData().get())) {

                const auto& transformer = inport_.getData()->getCoordinateTransformer();
                lightDirection =
                    transformer.transformPosition(vec3{0.0f}, CoordinateSpace::World,
                                                  CoordinateSpace::Data) -
                    transformer.transformPosition(directionLight->getDirection(),
                                                  CoordinateSpace::World, CoordinateSpace::Data);
                if (glm::length(lightDirection) == 0) {
                    lightDirection = vec3{1, 0, 0};
                } else {
                    lightDirection = glm::normalize(lightDirection);
                }
                lightPos_ = lightDirection + vec3{0.5f};
                color = directionLight->getIntensity();
            }

            break;
        }

        case LightSourceType::Point: {
            if (lightType_ != LightSourceType::Point) {
                lightType_ = LightSourceType::Point;
                propagationShader_.getFragmentShaderObject()->addShaderDefine("POINT_LIGHT");
                propagationShader_.getFragmentShaderObject()->build();
                propagationShader_.link();
            }

            if (auto pointLight = dynamic_cast<const PointLight*>(lightSource_.getData().get())) {
                const auto& transformer = inport_.getData()->getCoordinateTransformer();
                lightPos_ = transformer.transformPosition(
                    pointLight->getPosition(), CoordinateSpace::World, CoordinateSpace::Data);
                lightDirection = lightPos_ - vec3(0.5f);
                if (glm::length(lightDirection) == 0) {
                    lightDirection = vec3(1, 0, 0);
                } else {
                    lightDirection = glm::normalize(lightDirection);
                }
                color = pointLight->getIntensity();
            }

            break;
        }

        case LightSourceType::SpotLight:
        case LightSourceType::Area:
        default:
            throw Exception(SourceContext{}, "unsupported light source '{}', expected '{}' or '{}'",
                            lightSource_.getData()->getLightSourceType(), LightSourceType::Point,
                            LightSourceType::Directional);
            break;
    }

    updatePermuationMatrices(lightDirection, propParams_.data(), &propParams_[1]);

    bool lightColorChanged = false;
    if (glm::any(glm::notEqual(color, vec3(lightColor_)))) {
        lightColor_ = vec4{color, lightColor_.a};
        lightColorChanged = true;
    }

    return lightColorChanged;
}

bool LightVolumeGL::volumeChanged(bool lightColorChanged) {
    auto input = inport_.getData();
    const size3_t inDim = input->getDimensions();
    const size3_t outDim{inDim.x / volumeSizeOption_.get(), inDim.y / volumeSizeOption_.get(),
                         inDim.z / volumeSizeOption_.get()};

    if (internalVolumesInvalid_ || (volumeDimOut_ != outDim)) {
        volumeDimOut_ = outDim;
        volumeDimOutF_ = vec3(volumeDimOut_);
        volumeDimOutFRCP_ = vec3(1.0f) / volumeDimOutF_;
        volumeDimInF_ = vec3(inDim);
        volumeDimInFRCP_ = vec3(1.0f) / volumeDimInF_;
        DataFormatId format;

        if (supportColoredLight_) {
            format = floatPrecision_ ? DataVec4Float32::id() : DataVec4UInt8::id();
        } else {
            format = floatPrecision_ ? DataFloat32::id() : DataUInt8::id();
        }

        for (auto& elem : propParams_) {
            if (elem.tex.getDataFormat()->getId() != format) {
                elem.tex = PropagationParameters::makeTex(
                    volumeDimOut_, format, supportColoredLight_ ? lightColor_ : borderColor_);
            } else {
                elem.tex.uploadAndResize(nullptr, volumeDimOut_);
            }
        }

        outport_.setData(nullptr);
        auto volumeGL = std::make_shared<VolumeGL>(volumeDimOut_, DataFormatBase::get(format));
        volumeGL->getTexture()->initialize(nullptr);
        volume_ = std::make_shared<Volume>(volumeGL);
        outport_.setData(volume_);

        internalVolumesInvalid_ = false;
        return true;
    } else if (lightColorChanged) {
        for (auto& elem : propParams_) {
            elem.tex.bind();
            glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR,
                             glm::value_ptr(supportColoredLight_ ? lightColor_ : borderColor_));
            elem.tex.unbind();
        }
    }

    return false;
}

void LightVolumeGL::volumeSizeOptionChanged() {
    if (inport_.hasData()) {
        if ((inport_.getData()->getDimensions() / size3_t(volumeSizeOption_.get())) !=
            volumeDimOut_) {
            internalVolumesInvalid_ = true;
        }
    }
}

void LightVolumeGL::supportColoredLightChanged() {
    propagationShader_.getFragmentShaderObject()->setShaderDefine("SUPPORT_LIGHT_COLOR",
                                                                  supportColoredLight_);

    propagationShader_.getFragmentShaderObject()->build();
    propagationShader_.link();

    if (volume_) {
        const std::size_t components = volume_->getDataFormat()->getComponents();

        if ((components < 3 && supportColoredLight_.get()) ||
            (components > 1 && !supportColoredLight_.get())) {
            internalVolumesInvalid_ = true;
        }
    }
}

void LightVolumeGL::floatPrecisionChanged() { internalVolumesInvalid_ = true; }

void LightVolumeGL::updatePermuationMatrices(const vec3& lightDir, PropagationParameters* closest,
                                             PropagationParameters* secondClosest) {
    if (calculatedOnes_ && lightDir == lightDir_) return;

    lightDir_ = lightDir;

    // Calculate closest and second closest axis-aligned face.
    float thisVal = glm::dot(faceNormals_[0], -lightDir);
    float closestVal = thisVal;
    float secondClosestVal = 0.f;
    int closestFace = 0;
    int secondClosestFace = 0;
    for (int i = 1; i < 6; ++i) {
        thisVal = glm::dot(faceNormals_[i], -lightDir);
        if (thisVal > closestVal) {
            secondClosestVal = closestVal;
            secondClosestFace = closestFace;
            closestVal = thisVal;
            closestFace = i;
        } else if (thisVal > secondClosestVal) {
            secondClosestVal = thisVal;
            secondClosestFace = i;
        }
    }

    const vec4 tmpLightDir = vec4(-lightDir.x, -lightDir.y, -lightDir.z, 1.f);

    // Perform permutation calculation for closest face
    definePermutationMatrices(closest->axisPermutation, closest->axisPermutationLight, closestFace);
    closest->axisPermutationINV = glm::inverse(closest->axisPermutation);
    closest->permutedLightDirection = closest->axisPermutationLight * tmpLightDir;

    // Perform permutation calculation for second closest face
    definePermutationMatrices(secondClosest->axisPermutation, secondClosest->axisPermutationLight,
                              secondClosestFace);
    secondClosest->axisPermutationINV = glm::inverse(secondClosest->axisPermutation);
    secondClosest->permutedLightDirection = secondClosest->axisPermutationLight * tmpLightDir;

    // Calculate the blending factor
    blendingFactor_ =
        static_cast<float>(1.f - (2.f * std::acos(closestVal) / std::numbers::pi_v<float>));

    calculatedOnes_ = true;
}

}  // namespace inviwo
