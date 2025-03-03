/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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

#include <modules/opengl/shader/shaderutils.h>

#include <inviwo/core/datastructures/camera/camera.h>          // for Camera, mat4
#include <inviwo/core/datastructures/coordinatetransformer.h>  // for SpatialCoordinateTransformer
#include <inviwo/core/datastructures/isovaluecollection.h>     // for IsoValueCollection
#include <inviwo/core/datastructures/light/lightingstate.h>    // for ShadingMode, LightingState
#include <inviwo/core/datastructures/spatialdata.h>            // for SpatialEntity
#include <inviwo/core/datastructures/datamapper.h>
#include <inviwo/core/ports/imageport.h>                      // for ImageInport
#include <inviwo/core/properties/cameraproperty.h>            // for CameraProperty
#include <inviwo/core/properties/isotfproperty.h>             // for IsoTFProperty
#include <inviwo/core/properties/isovalueproperty.h>          // for IsoValueProperty
#include <inviwo/core/properties/optionproperty.h>            // for OptionPropertyString, Opti...
#include <inviwo/core/properties/ordinalproperty.h>           // for FloatProperty, FloatVec3Pr...
#include <inviwo/core/properties/planeproperty.h>             // for PlaneProperty
#include <inviwo/core/properties/raycastingproperty.h>        // for RaycastingProperty, Raycas...
#include <inviwo/core/properties/selectioncolorproperty.h>    // for SelectionColorProperty
#include <inviwo/core/properties/simplelightingproperty.h>    // for SimpleLightingProperty
#include <inviwo/core/properties/simpleraycastingproperty.h>  // for SimpleRaycastingProperty
#include <inviwo/core/properties/volumeindicatorproperty.h>   // for VolumeIndicatorProperty
#include <inviwo/core/util/formats.h>                         // for DataFormatBase, NumericType
#include <inviwo/core/util/glmvec.h>                          // for vec4
#include <inviwo/core/util/sourcecontext.h>                   // for SourceContext
#include <inviwo/core/util/stringconversion.h>                // for StrBuffer, splitStringView
#include <modules/opengl/inviwoopengl.h>                      // for LGL_ERROR
#include <modules/opengl/glformats.h>
#include <modules/opengl/openglcapabilities.h>
#include <modules/opengl/openglexception.h>       // for OpenGLException
#include <modules/opengl/shader/shader.h>         // for Shader
#include <modules/opengl/shader/shadermanager.h>  // for ShaderManager
#include <modules/opengl/shader/shaderobject.h>   // for ShaderObject
#include <modules/opengl/shader/shadertype.h>     // for ShaderType

#include <algorithm>  // for max
#include <istream>    // for basic_istream, stringstream

#include <fmt/core.h>      // for basic_string_view, format_to
#include <glm/fwd.hpp>     // for mat3
#include <glm/mat4x4.hpp>  // for operator*
#include <glm/matrix.hpp>  // for inverse, transpose
#include <glm/vec4.hpp>    // for operator*, operator+, oper...

namespace inviwo {
class ShaderResource;

namespace utilgl {

void setShaderUniforms(Shader& shader, const DataMapper& dataMapper, const DataFormatBase* format,
                       std::string_view name) {
    using enum DataMapper::SignedNormalization;

    const dvec2 dataRange = dataMapper.dataRange;
    const DataMapper defaultRange(
        format, OpenGLCapabilities::isSignedIntNormalizationSymmetric() ? Symmetric : Asymmetric);

    const double invRange = 1.0 / (dataRange.y - dataRange.x);
    const double defaultToDataRange =
        (defaultRange.dataRange.y - defaultRange.dataRange.x) * invRange;
    const double defaultToDataOffset = (dataRange.x - defaultRange.dataRange.x) /
                                       (defaultRange.dataRange.y - defaultRange.dataRange.x);

    double scalingFactor = 1.0;
    double signedScalingFactor = 1.0;
    double offset = 0.0;
    double signedOffset = 0.0;

    switch (GLFormats::get(format->getId()).normalization) {
        case utilgl::Normalization::None:
            scalingFactor = invRange;
            offset = -dataRange.x;
            signedScalingFactor = scalingFactor;
            signedOffset = offset;
            break;
        case utilgl::Normalization::Normalized:
            scalingFactor = defaultToDataRange;
            offset = -defaultToDataOffset;
            signedScalingFactor = scalingFactor;
            signedOffset = offset;
            break;
        case utilgl::Normalization::SignNormalized:
            scalingFactor = 0.5 * defaultToDataRange;
            offset = 1.0 - 2 * defaultToDataOffset;
            signedScalingFactor = defaultToDataRange;
            signedOffset = -defaultToDataOffset;
            break;
    }
    // offset scaling because of reversed scaling in the shader, i.e. (1 - formatScaling_)
    StrBuffer buff;
    shader.setUniform(buff.replace("{}.formatScaling", name),
                      static_cast<float>(1.0 - scalingFactor));
    shader.setUniform(buff.replace("{}.formatOffset", name), static_cast<float>(offset));

    shader.setUniform(buff.replace("{}.signedFormatScaling", name),
                      static_cast<float>(1.0 - signedScalingFactor));
    shader.setUniform(buff.replace("{}.signedFormatOffset", name),
                      static_cast<float>(signedOffset));
}

void addShaderDefines(Shader& shader, const SimpleLightingProperty& property) {
    addShaderDefines(shader, property.shadingMode_.get());
}

void addShaderDefines(Shader& shader, const LightingState& state) {
    addShaderDefines(shader, state.shadingMode);
}

void addShaderDefines(Shader& shader, const ShadingMode& mode) {
    // implementations in  modules/opengl/glsl/utils/shading.glsl
    constexpr std::string_view shadingKey =
        "APPLY_LIGHTING(lighting, materialAmbientColor, materialDiffuseColor, "
        "materialSpecularColor, position, normal, toCameraDir)";
    const std::string_view shadingValue = [&]() {
        switch (mode) {
            case ShadingMode::Ambient:
                return "shadeAmbient(lighting, materialAmbientColor)";
            case ShadingMode::Diffuse:
                return "shadeDiffuse(lighting, materialDiffuseColor, position, normal)";
            case ShadingMode::Specular:
                return "shadeSpecular(lighting, materialSpecularColor, position, normal, "
                       "toCameraDir)";
            case ShadingMode::BlinnPhong:
            case ShadingMode::BlinnPhongFront:
            case ShadingMode::BlinnPhongBack:
                return "shadeBlinnPhong(lighting, materialAmbientColor, materialDiffuseColor, "
                       "materialSpecularColor, position, normal, toCameraDir)";
            case ShadingMode::Phong:
            case ShadingMode::PhongFront:
            case ShadingMode::PhongBack:
                return "shadePhong(lighting, materialAmbientColor, materialDiffuseColor, "
                       "materialSpecularColor, position, normal, toCameraDir)";
            case ShadingMode::None:
            default:
                return "materialAmbientColor";
        }
    }();

    const int shadingNormalValue = [&]() -> int {
        switch (mode) {
            case ShadingMode::Ambient:
            case ShadingMode::Diffuse:
            case ShadingMode::Specular:
            case ShadingMode::BlinnPhong:
            case ShadingMode::Phong:
                return 2;
            case ShadingMode::BlinnPhongFront:
            case ShadingMode::PhongFront:
                return 0;
            case ShadingMode::BlinnPhongBack:
            case ShadingMode::PhongBack:
                return 1;
            case ShadingMode::None:
            default:
                return 0;
        }
    }();

    StrBuffer buff;
    ShaderObject* shaderObjects[] = {shader.getFragmentShaderObject(), shader.getComputeShaderObject()};
    for (auto&& shaderObject: shaderObjects) {
        if (shaderObject) {
            shaderObject->addShaderDefine(shadingKey, shadingValue);
            shaderObject->addShaderDefine("SHADING_NORMAL", buff.replace("{}", shadingNormalValue));
            shaderObject->setShaderDefine("SHADING_ENABLED", mode != ShadingMode::None);
        }
     }
}

void setShaderUniforms(Shader& shader, const SimpleLightingProperty& property,
                       std::string_view name) {
    setShaderUniforms(shader, property.getState(), name);
}

void setShaderUniforms(Shader& shader, const LightingState& state, std::string_view name) {
    StrBuffer buff;
    shader.setUniform(buff.replace("{}.position", name), state.position);
    shader.setUniform(buff.replace("{}.ambientColor", name), state.ambient);
    shader.setUniform(buff.replace("{}.diffuseColor", name), state.diffuse);
    shader.setUniform(buff.replace("{}.specularColor", name), state.specular);
    shader.setUniform(buff.replace("{}.specularExponent", name), state.exponent);
}

void addShaderDefines(Shader& /*shader*/, const CameraProperty& /*property*/) {}

void setShaderUniforms(Shader& shader, const CameraProperty& property, std::string_view name) {
    StrBuffer buff;

    shader.setUniform(buff.replace("{}.worldToView", name), property.viewMatrix());
    shader.setUniform(buff.replace("{}.viewToWorld", name), property.inverseViewMatrix());
    shader.setUniform(buff.replace("{}.worldToClip", name),
                      property.projectionMatrix() * property.viewMatrix());
    shader.setUniform(buff.replace("{}.viewToClip", name), property.projectionMatrix());
    shader.setUniform(buff.replace("{}.clipToView", name), property.inverseProjectionMatrix());
    shader.setUniform(buff.replace("{}.clipToWorld", name),
                      property.inverseViewMatrix() * property.inverseProjectionMatrix());
    shader.setUniform(buff.replace("{}.position", name), property.getLookFrom());
    shader.setUniform(buff.replace("{}.nearPlane", name), property.getNearPlaneDist());
    shader.setUniform(buff.replace("{}.farPlane", name), property.getFarPlaneDist());
}

void setShaderUniforms(Shader& shader, const Camera& camera, std::string_view name) {
    StrBuffer buff;

    shader.setUniform(buff.replace("{}.worldToView", name), camera.getViewMatrix());
    shader.setUniform(buff.replace("{}.viewToWorld", name), camera.getInverseViewMatrix());
    shader.setUniform(buff.replace("{}.worldToClip", name),
                      camera.getProjectionMatrix() * camera.getViewMatrix());
    shader.setUniform(buff.replace("{}.viewToClip", name), camera.getProjectionMatrix());
    shader.setUniform(buff.replace("{}.clipToView", name), camera.getInverseProjectionMatrix());
    shader.setUniform(buff.replace("{}.clipToWorld", name),
                      camera.getInverseViewMatrix() * camera.getInverseProjectionMatrix());
    shader.setUniform(buff.replace("{}.position", name), camera.getLookFrom());
    shader.setUniform(buff.replace("{}.nearPlane", name), camera.getNearPlaneDist());
    shader.setUniform(buff.replace("{}.farPlane", name), camera.getFarPlaneDist());
}

void addShaderDefines(Shader& shader, const RaycastingProperty& property) {
    {
        // rendering type
        switch (property.renderingType_.get()) {
            case RaycastingProperty::RenderingType::DvrIsosurface:
                if (auto fso = shader.getFragmentShaderObject()) {
                    fso->addShaderDefine("INCLUDE_DVR");
                    fso->addShaderDefine("INCLUDE_ISOSURFACES");
                }
                if (auto cso = shader.getComputeShaderObject()) {
                    cso->addShaderDefine("INCLUDE_DVR");
                    cso->addShaderDefine("INCLUDE_ISOSURFACES");
                }
                // shader.getFragmentShaderObject()->addShaderDefine("INCLUDE_DVR");
                // shader.getFragmentShaderObject()>addShaderDefine("INCLUDE_ISOSURFACES");
                break;
            case RaycastingProperty::RenderingType::Isosurface:
                if (auto fso = shader.getFragmentShaderObject()) {
                    fso->addShaderDefine("INCLUDE_ISOSURFACES");
                    fso->removeShaderDefine("INCLUDE_DVR");
                }
                if (auto cso = shader.getComputeShaderObject()) {
                    cso->addShaderDefine("INCLUDE_ISOSURFACES");
                    cso->removeShaderDefine("INCLUDE_DVR");
                }
                // shader.getFragmentShaderObject()->addShaderDefine("INCLUDE_ISOSURFACES");
                // shader.getFragmentShaderObject()->removeShaderDefine("INCLUDE_DVR");
                break;
            case RaycastingProperty::RenderingType::Dvr:
            default:
                if (auto fso = shader.getFragmentShaderObject()) {
                    fso->addShaderDefine("INCLUDE_DVR");
                    fso->removeShaderDefine("INCLUDE_ISOSURFACES");
                }
                if (auto cso = shader.getComputeShaderObject()) {
                    cso->addShaderDefine("INCLUDE_DVR");
                    cso->removeShaderDefine("INCLUDE_ISOSURFACES");
                }
                // shader.getFragmentShaderObject()->addShaderDefine("INCLUDE_DVR");
                // shader.getFragmentShaderObject()->removeShaderDefine("INCLUDE_ISOSURFACES");
                break;
        }
    }

    {
        // classification (default (red channel) or specific channel)
        std::string_view value;
        std::string_view valueMulti;
        switch (property.classification_.get()) {
            case RaycastingProperty::Classification::None:
                value = "vec4(voxel.r)";
                valueMulti = "vec4(voxel[channel])";
                break;
            case RaycastingProperty::Classification::TF:
                value = "applyTF(transferFunc, voxel.r)";
                valueMulti = "applyTF(transferFunc, voxel, channel)";
                break;
            case RaycastingProperty::Classification::Voxel:
            default:
                value = "voxel";
                valueMulti = "voxel";
                break;
        }
        const std::string_view key = "APPLY_CLASSIFICATION(transferFunc, voxel)";
        const std::string_view keyMulti =
            "APPLY_CHANNEL_CLASSIFICATION(transferFunc, voxel, channel)";

        if (auto fso = shader.getFragmentShaderObject()) {
            fso->addShaderDefine(key, value);
            fso->addShaderDefine(keyMulti, valueMulti);
        }
        if (auto cso = shader.getComputeShaderObject()) {
            cso->addShaderDefine(key, value);
            cso->addShaderDefine(keyMulti, valueMulti);
        }
        // shader.getFragmentShaderObject()->addShaderDefine(key, value);
        // shader.getFragmentShaderObject()->addShaderDefine(keyMulti, valueMulti);
    }

    {
        // compositing
        std::string_view value;
        switch (property.compositing_.get()) {
            case RaycastingProperty::CompositingType::Dvr:
                value = "compositeDVR(result, color, t, tDepth, tIncr)";
                break;
            case RaycastingProperty::CompositingType::MaximumIntensity:
                value = "compositeMIP(result, color, t, tDepth)";
                break;
            case RaycastingProperty::CompositingType::FirstHitPoints:
                value = "compositeFHP(result, color, samplePos, t, tDepth)";
                break;
            case RaycastingProperty::CompositingType::FirstHitNormals:
                value = "compositeFHN(result, color, gradient, t, tDepth)";
                break;
            case RaycastingProperty::CompositingType::FirstHistNormalsView:
                value = "compositeFHN_VS(result, color, gradient, t, camera, tDepth)";
                break;
            case RaycastingProperty::CompositingType::FirstHitDepth:
                value = "compositeFHD(result, color, t, tDepth)";
                break;
            default:
                value = "result";
                break;
        }
        const std::string_view key =
            "APPLY_COMPOSITING(result, color, samplePos, voxel, gradient, camera, isoValue, t, "
            "tDepth, tIncr)";
        if (auto fso = shader.getFragmentShaderObject()) {
            fso->addShaderDefine(key, value);
        }
        if (auto cso = shader.getComputeShaderObject()) {
            cso->addShaderDefine(key, value);
        }

        // shader.getFragmentShaderObject()->addShaderDefine(key, value);
    }

    // gradients
    setShaderDefines(shader, property.gradientComputation_,
                     property.classification_.get() == RaycastingProperty::Classification::Voxel);
}

void setShaderDefines(Shader& shader,
                      const OptionProperty<RaycastingProperty::GradientComputation>& property,
                      bool voxelClassification) {

    const std::string_view channel = (voxelClassification ? "3" : "channel");
    const std::string_view channelDef = (voxelClassification ? "3" : "0");

    std::string_view value;         // compute gradient for default channel
    std::string_view valueChannel;  // compute gradient for specific channel
    std::string_view valueAll;      // compute gradient for all channels
    switch (property.get()) {
        case RaycastingProperty::GradientComputation::None:
        default:
            value = "vec3(0)";
            valueChannel = "vec3(0)";
            valueAll = "mat4x3(0)";
            break;
        case RaycastingProperty::GradientComputation::Forward:
            value = "gradientForwardDiff(voxel, volume, volumeParams, samplePos, {channelDef})";
            valueChannel = "gradientForwardDiff(voxel, volume, volumeParams, samplePos, {channel})";
            valueAll = "gradientAllForwardDiff(voxel, volume, volumeParams, samplePos)";
            break;
        case RaycastingProperty::GradientComputation::Backward:
            value = "gradientBackwardDiff(voxel, volume, volumeParams, samplePos, {channelDef})";
            valueChannel =
                "gradientBackwardDiff(voxel, volume, volumeParams, samplePos, {channel})";
            valueAll = "gradientAllBackwardDiff(voxel, volume, volumeParams, samplePos)";
            break;
        case RaycastingProperty::GradientComputation::Central:
            value = "gradientCentralDiff(voxel, volume, volumeParams, samplePos, {channelDef})";
            valueChannel = "gradientCentralDiff(voxel, volume, volumeParams, samplePos, {channel})";
            valueAll = "gradientAllCentralDiff(voxel, volume, volumeParams, samplePos)";
            break;
        case RaycastingProperty::GradientComputation::CentralHigherOrder:
            value = "gradientCentralDiffH(voxel, volume, volumeParams, samplePos, {channelDef})";
            valueChannel =
                "gradientCentralDiffH(voxel, volume, volumeParams, samplePos, {channel})";
            valueAll = "gradientAllCentralDiffH(voxel, volume, volumeParams, samplePos)";
            break;
        case RaycastingProperty::GradientComputation::PrecomputedXYZ:
            value = valueChannel = valueAll =
                "gradientPrecomputedXYZ(volume, volumeParams, samplePos)";
            break;
        case RaycastingProperty::GradientComputation::PrecomputedYZW:
            value = valueChannel = valueAll =
                "gradientPrecomputedYZW(volume, volumeParams, samplePos)";
            break;
    }

    // gradient for channel 1
    const std::string_view key = "COMPUTE_GRADIENT(voxel, volume, volumeParams, samplePos)";
    // gradient for specific channel
    const std::string_view keyChannel =
        "COMPUTE_GRADIENT_FOR_CHANNEL(voxel, volume, volumeParams, samplePos, channel)";
    // gradients for all channels
    const std::string_view keyAll = "COMPUTE_ALL_GRADIENTS(voxel, volume, volumeParams, samplePos)";

    StrBuffer buff;
    buff.replace(fmt::runtime(value), fmt::arg("channelDef", channelDef));
    if (auto fso = shader.getFragmentShaderObject()) {
        fso->addShaderDefine(key, buff);
    }
    if (auto cso = shader.getComputeShaderObject()) {
        cso->addShaderDefine(key, buff);
    }
    // shader.getFragmentShaderObject()->addShaderDefine(key, buff);
    buff.replace(fmt::runtime(valueChannel), fmt::arg("channel", channel));

    if (auto fso = shader.getFragmentShaderObject()) {
        fso->addShaderDefine(keyChannel, buff);
    }
    if (auto cso = shader.getComputeShaderObject()) {
        cso->addShaderDefine(keyChannel, buff);
    }
    // shader.getFragmentShaderObject()->addShaderDefine(keyChannel, buff);

    if (auto fso = shader.getFragmentShaderObject()) {
        fso->addShaderDefine(keyAll, valueAll);
    }
    if (auto cso = shader.getComputeShaderObject()) {
        cso->addShaderDefine(keyAll, valueAll);
    }
    // shader.getFragmentShaderObject()->addShaderDefine(keyAll, valueAll);

    if (property.get() != RaycastingProperty::GradientComputation::None) {
        if (auto fso = shader.getFragmentShaderObject()) {
            fso->addShaderDefine("GRADIENTS_ENABLED");
        }
        if (auto cso = shader.getComputeShaderObject()) {
            cso->addShaderDefine("GRADIENTS_ENABLED");
        }
        // shader.getFragmentShaderObject()->addShaderDefine("GRADIENTS_ENABLED");
    } else {
        if (auto fso = shader.getFragmentShaderObject()) {
            fso->removeShaderDefine("GRADIENTS_ENABLED");
        }
        if (auto cso = shader.getComputeShaderObject()) {
            cso->removeShaderDefine("GRADIENTS_ENABLED");
        }
        // shader.getFragmentShaderObject()->removeShaderDefine("GRADIENTS_ENABLED");
    }
}

void setShaderUniforms(Shader& shader, const RaycastingProperty& property) {
    shader.setUniform("samplingRate_", property.samplingRate_.get());
}

void setShaderUniforms(Shader& shader, const RaycastingProperty& property, std::string_view name) {
    StrBuffer buff;
    shader.setUniform(buff.replace("{}.samplingRate", name), property.samplingRate_.get());
}

void setShaderUniforms(Shader& shader, const SpatialEntity& object, std::string_view name) {
    const SpatialCoordinateTransformer& ct = object.getCoordinateTransformer();

    mat4 dataToWorldMatrix = ct.getDataToWorldMatrix();
    mat4 modelToWorldMatrix = ct.getModelToWorldMatrix();

    StrBuffer buff;

    shader.setUniform(buff.replace("{}.dataToModel", name), ct.getDataToModelMatrix());
    shader.setUniform(buff.replace("{}.modelToData", name), ct.getModelToDataMatrix());

    shader.setUniform(buff.replace("{}.dataToWorld", name), dataToWorldMatrix);
    shader.setUniform(buff.replace("{}.worldToData", name), ct.getWorldToDataMatrix());

    shader.setUniform(buff.replace("{}.modelToWorld", name), modelToWorldMatrix);
    shader.setUniform(buff.replace("{}.worldToModel", name), ct.getWorldToModelMatrix());
    shader.setUniform(buff.replace("{}.modelToWorldNormalMatrix", name),
                      glm::mat3(glm::transpose(glm::inverse(modelToWorldMatrix))));

    shader.setUniform(buff.replace("{}.dataToWorldNormalMatrix", name),
                      glm::mat3(glm::transpose(glm::inverse(dataToWorldMatrix))));
}

void addShaderDefines(Shader& shader, const SimpleRaycastingProperty& property) {
    // gradient for channel 1
    const std::string_view gradientComputationKey =
        "COMPUTE_GRADIENT(voxel, volume, volumeParams, samplePos)";
    // gradient for specific channel
    const std::string_view singleChannelGradientKey =
        "COMPUTE_GRADIENT_FOR_CHANNEL(voxel, volume, volumeParams, samplePos, channel)";
    // gradients for all channels
    const std::string_view allChannelsGradientKey =
        "COMPUTE_ALL_GRADIENTS(voxel, volume, volumeParams, samplePos)";

    std::string_view gradientValue = "";
    std::string_view singleChannelGradientValue = "";
    std::string_view allChannelsGradientValue = "";
    std::string_view channel = "channel";
    std::string_view defaultChannel = "0";

    if (property.classificationMode_.isSelectedIdentifier("voxel-value")) {
        channel = "3";
        defaultChannel = "3";
    }

    if (property.gradientComputationMode_.isSelectedIdentifier("none")) {
        gradientValue = "vec3(0)";
        singleChannelGradientValue = "vec3(0)";
        allChannelsGradientValue = "mat4x3(0)";
    } else if (property.gradientComputationMode_.isSelectedIdentifier("forward")) {
        gradientValue =
            "gradientForwardDiff(voxel, volume, volumeParams, samplePos, {defaultChannel})";
        singleChannelGradientValue =
            "gradientForwardDiff(voxel, volume, volumeParams, samplePos, {channel})";
        allChannelsGradientValue = "gradientAllForwardDiff(voxel, volume, volumeParams, samplePos)";
    } else if (property.gradientComputationMode_.isSelectedIdentifier("central")) {
        gradientValue =
            "gradientCentralDiff(voxel, volume, volumeParams, samplePos, {defaultChannel})";
        singleChannelGradientValue =
            "gradientCentralDiff(voxel, volume, volumeParams, samplePos, {channel})";
        allChannelsGradientValue = "gradientAllCentralDiff(voxel, volume, volumeParams, samplePos)";
    } else if (property.gradientComputationMode_.isSelectedIdentifier("central-higher")) {
        gradientValue =
            "gradientCentralDiffH(voxel, volume, volumeParams, samplePos, {defaultChannel})";
        singleChannelGradientValue =
            "gradientCentralDiffH(voxel, volume, volumeParams, samplePos, {channel})";
        allChannelsGradientValue =
            "gradientAllCentralDiffH(voxel, volume, volumeParams, samplePos)";
    } else if (property.gradientComputationMode_.isSelectedIdentifier("backward")) {
        gradientValue =
            "gradientBackwardDiff(voxel, volume, volumeParams, samplePos, {defaultChannel})";
        singleChannelGradientValue =
            "gradientBackwardDiff(voxel, volume, volumeParams, samplePos, {channel})";
        allChannelsGradientValue =
            "gradientAllBackwardDiff(voxel, volume, volumeParams, samplePos)";
    } else if (property.gradientComputationMode_.isSelectedIdentifier("precomputedXYZ")) {
        gradientValue = "gradientPrecomputedXYZ(voxel, volumeParams)";
        singleChannelGradientValue = "gradientPrecomputedXYZ(voxel, volumeParams)";
        allChannelsGradientValue = "gradientPrecomputedXYZ(voxel, volumeParams)";
    } else if (property.gradientComputationMode_.isSelectedIdentifier("precomputedYZW")) {
        gradientValue = "gradientPrecomputedYZW(voxel, volumeParams)";
        singleChannelGradientValue = "gradientPrecomputedYZW(voxel, volumeParams)";
        allChannelsGradientValue = "gradientPrecomputedYZW(voxel, volumeParams)";
    }

    StrBuffer buff;
    buff.replace(fmt::runtime(gradientValue), fmt::arg("defaultChannel", defaultChannel));
    if (auto fso = shader.getFragmentShaderObject()) {
        fso->addShaderDefine(gradientComputationKey, buff);
    }
    if (auto cso = shader.getComputeShaderObject()) {
        cso->addShaderDefine(gradientComputationKey, buff);
    }
    // shader.getFragmentShaderObject()->addShaderDefine(gradientComputationKey, buff);

    buff.replace(fmt::runtime(singleChannelGradientValue), fmt::arg("channel", channel));
    if (auto fso = shader.getFragmentShaderObject()) {
        fso->addShaderDefine(singleChannelGradientKey, buff);
    }
    if (auto cso = shader.getComputeShaderObject()) {
        cso->addShaderDefine(singleChannelGradientKey, buff);
    }
    // shader.getFragmentShaderObject()->addShaderDefine(singleChannelGradientKey, buff);

    if (auto fso = shader.getFragmentShaderObject()) {
        fso->addShaderDefine(allChannelsGradientKey, allChannelsGradientValue);
    }
    if (auto cso = shader.getComputeShaderObject()) {
        cso->addShaderDefine(allChannelsGradientKey, allChannelsGradientValue);
    }
    // shader.getFragmentShaderObject()->addShaderDefine(allChannelsGradientKey,
    //                                                   allChannelsGradientValue);

    if (auto fso = shader.getFragmentShaderObject()) {
        if (property.gradientComputationMode_.isSelectedIdentifier("none")) {
            fso->removeShaderDefine("GRADIENTS_ENABLED");
        } else {
            fso->addShaderDefine("GRADIENTS_ENABLED");
        }
    }

    if (auto cso = shader.getComputeShaderObject()) {
        if (property.gradientComputationMode_.isSelectedIdentifier("none")) {
            cso->removeShaderDefine("GRADIENTS_ENABLED");
        } else {
            cso->addShaderDefine("GRADIENTS_ENABLED");
        }
    }
    // classification defines, red channel is used
    std::string_view classificationKey = "APPLY_CLASSIFICATION(transferFunc, voxel)";
    std::string_view classificationValue = "";
    if (property.classificationMode_.isSelectedIdentifier("none"))
        classificationValue = "vec4(voxel.r)";
    else if (property.classificationMode_.isSelectedIdentifier("transfer-function"))
        classificationValue = "applyTF(transferFunc, voxel.r)";
    else if (property.classificationMode_.isSelectedIdentifier("voxel-value"))
        classificationValue = "voxel";

    if (auto fso = shader.getFragmentShaderObject()) {
        fso->addShaderDefine(classificationKey, classificationValue);
    }
    if (auto cso = shader.getComputeShaderObject()) {
        cso->addShaderDefine(classificationKey, classificationValue);
    }
    // shader.getFragmentShaderObject()->addShaderDefine(classificationKey, classificationValue);

    // classification of specific channel
    classificationKey = "APPLY_CHANNEL_CLASSIFICATION(transferFunc, voxel, channel)";
    classificationValue = "";
    if (property.classificationMode_.isSelectedIdentifier("none"))
        classificationValue = "vec4(voxel[channel])";
    else if (property.classificationMode_.isSelectedIdentifier("transfer-function"))
        classificationValue = "applyTF(transferFunc, voxel, channel)";
    else if (property.classificationMode_.isSelectedIdentifier("voxel-value"))
        classificationValue = "voxel";

    if (auto fso = shader.getFragmentShaderObject()) {
        fso->addShaderDefine(classificationKey, classificationValue);
    }
    if (auto cso = shader.getComputeShaderObject()) {
        cso->addShaderDefine(classificationKey, classificationValue);
    }
    // shader.getFragmentShaderObject()->addShaderDefine(classificationKey, classificationValue);
    //  compositing defines
    std::string_view compositingKey =
        "APPLY_COMPOSITING(result, color, samplePos, voxel, gradient, camera, isoValue, t, tDepth, "
        "tIncr)";
    std::string_view compositingValue = "result";

    if (property.compositingMode_.isSelectedIdentifier("dvr"))
        compositingValue = "compositeDVR(result, color, t, tDepth, tIncr)";
    else if (property.compositingMode_.isSelectedIdentifier("mip"))
        compositingValue = "compositeMIP(result, color, t, tDepth)";
    else if (property.compositingMode_.isSelectedIdentifier("fhp"))
        compositingValue = "compositeFHP(result, color, samplePos, t, tDepth)";
    else if (property.compositingMode_.isSelectedIdentifier("fhn"))
        compositingValue = "compositeFHN(result, color, gradient, t, tDepth)";
    else if (property.compositingMode_.isSelectedIdentifier("fhnvs"))
        compositingValue = "compositeFHN_VS(result, color, gradient, t, camera, tDepth)";
    else if (property.compositingMode_.isSelectedIdentifier("fhd"))
        compositingValue = "compositeFHD(result, color, t, tDepth)";
    else if (property.compositingMode_.isSelectedIdentifier("iso"))
        compositingValue = "compositeISO(result, color, voxel.r, t, tDepth, tIncr, isoValue)";
    else if (property.compositingMode_.isSelectedIdentifier("ison"))
        compositingValue = "compositeISON(result, color, voxel.r, gradient, t, tDepth, isoValue)";

    if (auto fso = shader.getFragmentShaderObject()) {
        fso->addShaderDefine(compositingKey, compositingValue);
    }
    if (auto cso = shader.getComputeShaderObject()) {
        cso->addShaderDefine(compositingKey, compositingValue);
    }
    // shader.getFragmentShaderObject()->addShaderDefine(compositingKey, compositingValue);
}

void setShaderUniforms(Shader& shader, const SimpleRaycastingProperty& property) {
    shader.setUniform("samplingRate_", property.samplingRate_.get());
    shader.setUniform("isoValue_", property.isoValue_.get());
}

void setShaderUniforms(Shader& shader, const SimpleRaycastingProperty& property,
                       std::string_view name) {
    StrBuffer buff;
    shader.setUniform(buff.replace("{}.samplingRate", name), property.samplingRate_.get());
    shader.setUniform(buff.replace("{}.isoValue", name), property.isoValue_.get());
}

void addShaderDefines(Shader& shader, const IsoValueProperty& property) {
    const auto isovalueCount = property.get().size();

    // need to ensure there is always at least one isovalue due to the use of the macro
    // as array size in IsovalueParameters

    shader.getFragmentShaderObject()->addShaderDefine(
        "MAX_ISOVALUE_COUNT", StrBuffer{"{}", std::max<size_t>(1, isovalueCount)});

    shader.getFragmentShaderObject()->setShaderDefine("ISOSURFACE_ENABLED",
                                                      !property.get().empty());
}

void setShaderUniforms(Shader& shader, const IsoValueProperty& property) {
    auto data = property.get().getVectorsf();

    shader.setUniform("isovalues", data.first);
    shader.setUniform("isosurfaceColors", data.second);
}

void setShaderUniforms(Shader& shader, const IsoValueProperty& property, std::string_view name) {
    auto data = property.get().getVectorsf();

    StrBuffer buff;
    shader.setUniform(buff.replace("{}.values", name), data.first);
    shader.setUniform(buff.replace("{}.colors", name), data.second);
}

void addShaderDefines(Shader& shader, const IsoTFProperty& property) {
    addShaderDefines(shader, property.isovalues_);
}

void setShaderUniforms(Shader& shader, const IsoTFProperty& property) {
    setShaderUniforms(shader, property.isovalues_);
}

void setShaderUniforms(Shader& shader, const IsoTFProperty& property, std::string_view) {
    setShaderUniforms(shader, property.isovalues_, property.isovalues_.getIdentifier());
}

void setShaderUniforms(Shader& shader, const SelectionColorProperty& property) {
    setShaderUniforms(shader, property.getState(), property.getIdentifier());
}

void setShaderUniforms(Shader& shader, const SelectionColorProperty& property,
                       std::string_view name) {
    setShaderUniforms(shader, property.getState(), name);
}

void setShaderUniforms(Shader& shader, const SelectionColorState& state, std::string_view name) {
    StrBuffer buff;
    shader.setUniform(buff.replace("{}.color", name), state.color);
    shader.setUniform(buff.replace("{}.colorMixIn", name), state.colorMixIn);
    shader.setUniform(buff.replace("{}.alphaMixIn", name), 1.0f);
    shader.setUniform(buff.replace("{}.visible", name), state.visible);
}

void addShaderDefinesBGPort(Shader& shader, const ImageInport& port) {
    std::string_view bgKey = "DRAW_BACKGROUND(result,t,tIncr,color,bgTDepth,tDepth)";
    if (port.isConnected()) {
        shader.getFragmentShaderObject()->addShaderDefine("BACKGROUND_AVAILABLE");
        shader.getFragmentShaderObject()->addShaderDefine(
            bgKey, "drawBackground(result,t,tIncr,color,bgTDepth,tDepth)");
    } else {
        shader.getFragmentShaderObject()->removeShaderDefine("BACKGROUND_AVAILABLE");
        shader.getFragmentShaderObject()->addShaderDefine(bgKey, "result");
    }
}

void addShaderDefines(Shader& shader, const VolumeIndicatorProperty& indicator) {
    const bool enabled = indicator && (indicator.plane1_ || indicator.plane2_ || indicator.plane3_);
    shader.getFragmentShaderObject()->setShaderDefine("PLANES_ENABLED", enabled);

    constexpr std::string_view key =
        "DRAW_PLANES(result, samplePosition, rayDirection, increment, params, t, tDepth)";
    if (enabled) {
        StrBuffer planes(
            "drawPlanes(result, samplePosition, rayDirection, increment{}{}{}, t, tDepth)",
            (indicator.plane1_ ? ", params.plane1" : ""),
            (indicator.plane2_ ? ", params.plane2" : ""),
            (indicator.plane3_ ? ", params.plane3" : ""));
        shader.getFragmentShaderObject()->addShaderDefine(key, planes);
    } else {
        shader.getFragmentShaderObject()->addShaderDefine(key, "result");
    }
}

void setShaderUniforms(Shader& shader, const VolumeIndicatorProperty& indicator,
                       std::string_view name) {
    StrBuffer buff;

    if (indicator) {
        if (indicator.plane1_) {
            shader.setUniform(buff.replace("{}.plane1.position", name),
                              indicator.plane1_.position_.get());
            shader.setUniform(buff.replace("{}.plane1.normal", name),
                              indicator.plane1_.normal_.get());
            shader.setUniform(buff.replace("{}.plane1.color", name),
                              indicator.plane1_.color_.get());
        }
        if (indicator.plane2_) {
            shader.setUniform(buff.replace("{}.plane2.position", name),
                              indicator.plane2_.position_.get());
            shader.setUniform(buff.replace("{}.plane2.normal", name),
                              indicator.plane2_.normal_.get());
            shader.setUniform(buff.replace("{}.plane2.color", name),
                              indicator.plane2_.color_.get());
        }
        if (indicator.plane3_) {
            shader.setUniform(buff.replace("{}.plane3.position", name),
                              indicator.plane3_.position_.get());
            shader.setUniform(buff.replace("{}.plane3.normal", name),
                              indicator.plane3_.normal_.get());
            shader.setUniform(buff.replace("{}.plane3.color", name),
                              indicator.plane3_.color_.get());
        }
    }
}

int getLogLineNumber(std::string_view compileLogLine) {
    int result = -1;
    std::stringstream input;
    input << compileLogLine;
    int num;

    if (input >> num) {
        char c;
        if (input >> c && c == '(') {
            if (input >> result) {
                return result;
            }
        }
    }

    // ATI parsing:
    // ATI error: "ERROR: 0:145: Call to undeclared function 'texelFetch'\n"
    const auto elems = util::splitStringView(compileLogLine, ':');
    if (elems.size() >= 3) {
        std::stringstream number;
        number << elems[2];
        number >> result;
    }

    return result;
}

std::string getShaderInfoLog(GLuint id) {
    GLint maxLogLength;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLogLength);
    LGL_ERROR;

    if (maxLogLength > 1) {
        auto shaderInfoLog = std::make_unique<GLchar[]>(maxLogLength);
        GLsizei logLength{0};
        glGetShaderInfoLog(id, maxLogLength, &logLength, shaderInfoLog.get());
        return std::string(shaderInfoLog.get(), logLength);
    } else {
        return "";
    }
}

std::string getProgramInfoLog(GLuint id) {
    GLint maxLogLength;
    glGetProgramiv(id, GL_INFO_LOG_LENGTH, &maxLogLength);
    LGL_ERROR;

    if (maxLogLength > 1) {
        auto shaderInfoLog = std::make_unique<GLchar[]>(maxLogLength);
        GLsizei logLength{0};
        glGetProgramInfoLog(id, maxLogLength, &logLength, shaderInfoLog.get());
        return std::string(shaderInfoLog.get(), logLength);
    } else {
        return "";
    }
}

std::shared_ptr<const ShaderResource> findShaderResource(std::string_view fileName) {
    auto resource = ShaderManager::getPtr()->getShaderResource(fileName);
    if (!resource) {
        throw OpenGLException(
            SourceContext{},
            "Shader file: '{}' not found in shader search paths or shader resources.", fileName);
    }
    return resource;
}

std::vector<std::pair<ShaderType, std::shared_ptr<const ShaderResource>>> toShaderResources(
    const std::vector<std::pair<ShaderType, std::string>>& items) {

    std::vector<std::pair<ShaderType, std::shared_ptr<const ShaderResource>>> res;
    for (auto& item : items) {
        res.emplace_back(item.first, utilgl::findShaderResource(item.second));
    }
    return res;
}

std::string getGLSLTypeName(const DataFormatBase* format) {
    if (format->getComponents() == 1) {
        switch (format->getNumericType()) {
            case NumericType::Float: {
                switch (format->getPrecision()) {
                    case 32:
                        return "float";
                    case 64:
                        return "double";
                    default:
                        return "";
                }
            }
            case NumericType::UnsignedInteger: {
                if (format->getPrecision() < 64) {
                    return "uint";
                } else {
                    return "";
                }
            }
            case NumericType::SignedInteger: {
                if (format->getPrecision() < 64) {
                    return "int";
                } else {
                    return "";
                }
            }
            default:
                return "";
        }
    } else {
        const auto comp = toString(format->getComponents());
        switch (format->getNumericType()) {
            case NumericType::Float: {
                switch (format->getPrecision()) {
                    case 32:
                        return "vec" + comp;
                    case 64:
                        return "dvec" + comp;
                    default:
                        return "";
                }
            }
            case NumericType::UnsignedInteger: {
                if (format->getPrecision() < 64) {
                    return "uvec" + comp;
                } else {
                    return "";
                }
            }
            case NumericType::SignedInteger: {
                if (format->getPrecision() < 64) {
                    return "ivec" + comp;
                } else {
                    return "";
                }
            }
            default:
                return "";
        }
    }
}

}  // namespace utilgl

}  // namespace inviwo
