/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#include <inviwo/core/datastructures/coordinatetransformer.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/shader/shadermanager.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/volume/volumeutils.h>

#include <inviwo/core/properties/simpleraycastingproperty.h>
#include <inviwo/core/properties/isotfproperty.h>
#include <inviwo/core/properties/isovalueproperty.h>
#include <inviwo/core/datastructures/camera.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/volumeindicatorproperty.h>
#include <inviwo/core/ports/imageport.h>

namespace inviwo {

namespace utilgl {

void addShaderDefines(Shader& shader, const SimpleLightingProperty& property) {
    addShaderDefines(shader, ShadingMode::Modes(property.shadingMode_.get()));
}

void addShaderDefines(Shader& shader, const ShadingMode::Modes& mode) {
    // implementations in  modules/opengl/glsl/utils/shading.glsl
    std::string shadingKey =
        "APPLY_LIGHTING(lighting, materialAmbientColor, materialDiffuseColor, "
        "materialSpecularColor, position, normal, toCameraDir)";
    std::string shadingValue = "";

    switch (mode) {
        case ShadingMode::Ambient:
            shadingValue = "shadeAmbient(lighting, materialAmbientColor)";
            break;
        case ShadingMode::Diffuse:
            shadingValue = "shadeDiffuse(lighting, materialDiffuseColor, position, normal)";
            break;
        case ShadingMode::Specular:
            shadingValue =
                "shadeSpecular(lighting, materialSpecularColor, position, normal, toCameraDir)";
            break;
        case ShadingMode::BlinnPhong:
            shadingValue =
                "shadeBlinnPhong(lighting, materialAmbientColor, materialDiffuseColor, "
                "materialSpecularColor, position, normal, toCameraDir)";
            break;
        case ShadingMode::Phong:
            shadingValue =
                "shadePhong(lighting, materialAmbientColor, materialDiffuseColor, "
                "materialSpecularColor, position, normal, toCameraDir)";
            break;
        case ShadingMode::None:
        default:
            shadingValue = "materialAmbientColor";
            break;
    }

    shader.getFragmentShaderObject()->addShaderDefine(shadingKey, shadingValue);

    if (mode == ShadingMode::None) {
        shader.getFragmentShaderObject()->removeShaderDefine("SHADING_ENABLED");
    } else {
        shader.getFragmentShaderObject()->addShaderDefine("SHADING_ENABLED");
    }
}

void setShaderUniforms(Shader& shader, const SimpleLightingProperty& property, std::string name) {
    shader.setUniform(name + ".position", property.getTransformedPosition());
    shader.setUniform(name + ".ambientColor", property.ambientColor_.get());
    shader.setUniform(name + ".diffuseColor", property.diffuseColor_.get());
    shader.setUniform(name + ".specularColor", property.specularColor_.get());
    shader.setUniform(name + ".specularExponent", property.specularExponent_.get());
}

void addShaderDefines(Shader& /*shader*/, const CameraProperty& /*property*/) {}

void setShaderUniforms(Shader& shader, const CameraProperty& property, std::string name) {
    shader.setUniform(name + ".worldToView", property.viewMatrix());
    shader.setUniform(name + ".viewToWorld", property.inverseViewMatrix());
    shader.setUniform(name + ".worldToClip", property.projectionMatrix() * property.viewMatrix());
    shader.setUniform(name + ".viewToClip", property.projectionMatrix());
    shader.setUniform(name + ".clipToView", property.inverseProjectionMatrix());
    shader.setUniform(name + ".clipToWorld",
                      property.inverseViewMatrix() * property.inverseProjectionMatrix());
    shader.setUniform(name + ".position", property.getLookFrom());
    shader.setUniform(name + ".nearPlane", property.getNearPlaneDist());
    shader.setUniform(name + ".farPlane", property.getFarPlaneDist());
}

void setShaderUniforms(Shader& shader, const Camera& property, std::string name) {
    shader.setUniform(name + ".worldToView", property.getViewMatrix());
    shader.setUniform(name + ".viewToWorld", property.getInverseViewMatrix());
    shader.setUniform(name + ".worldToClip",
                      property.getProjectionMatrix() * property.getViewMatrix());
    shader.setUniform(name + ".viewToClip", property.getProjectionMatrix());
    shader.setUniform(name + ".clipToView", property.getInverseProjectionMatrix());
    shader.setUniform(name + ".clipToWorld",
                      property.getInverseViewMatrix() * property.getInverseProjectionMatrix());
    shader.setUniform(name + ".position", property.getLookFrom());
    shader.setUniform(name + ".nearPlane", property.getNearPlaneDist());
    shader.setUniform(name + ".farPlane", property.getFarPlaneDist());
}

void addShaderDefines(Shader& shader, const RaycastingProperty& property) {
    {
        // rendering type
        switch (property.renderingType_.get()) {
            case RaycastingProperty::RenderingType::DvrIsosurface:
                shader.getFragmentShaderObject()->addShaderDefine("INCLUDE_DVR");
                shader.getFragmentShaderObject()->addShaderDefine("INCLUDE_ISOSURFACES");
                break;
            case RaycastingProperty::RenderingType::Isosurface:
                shader.getFragmentShaderObject()->addShaderDefine("INCLUDE_ISOSURFACES");
                shader.getFragmentShaderObject()->removeShaderDefine("INCLUDE_DVR");
                break;
            case RaycastingProperty::RenderingType::Dvr:
            default:
                shader.getFragmentShaderObject()->addShaderDefine("INCLUDE_DVR");
                shader.getFragmentShaderObject()->removeShaderDefine("INCLUDE_ISOSURFACES");
                break;
        }
    }

    {
        // classification (default (red channel) or specific channel)
        std::string value;
        std::string valueMulti;
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
        const std::string key = "APPLY_CLASSIFICATION(transferFunc, voxel)";
        const std::string keyMulti = "APPLY_CHANNEL_CLASSIFICATION(transferFunc, voxel, channel)";
        shader.getFragmentShaderObject()->addShaderDefine(key, value);
        shader.getFragmentShaderObject()->addShaderDefine(keyMulti, valueMulti);
    }

    {
        // compositing
        std::string value;
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
        const std::string key =
            "APPLY_COMPOSITING(result, color, samplePos, voxel, gradient, camera, isoValue, t, "
            "tDepth, tIncr)";
        shader.getFragmentShaderObject()->addShaderDefine(key, value);
    }

    // gradients
    setShaderDefines(shader, property.gradientComputation_,
                     property.classification_.get() == RaycastingProperty::Classification::Voxel);
}

void setShaderDefines(
    Shader& shader, const TemplateOptionProperty<RaycastingProperty::GradientComputation>& property,
    bool voxelClassification) {

    const std::string channel = (voxelClassification ? "3" : "channel");
    const std::string channelDef = (voxelClassification ? "3" : "0");

    std::string value;         // compute gradient for default channel
    std::string valueChannel;  // compute gradient for specific channel
    std::string valueAll;      // compute gradient for all channels
    switch (property.get()) {
        case RaycastingProperty::GradientComputation::None:
        default:
            value = "vec3(0)";
            valueChannel = "vec3(0)";
            valueAll = "mat4x3(0)";
            break;
        case RaycastingProperty::GradientComputation::Forward:
            value =
                "gradientForwardDiff(voxel, volume, volumeParams, samplePos, " + channelDef + ")";
            valueChannel =
                "gradientForwardDiff(voxel, volume, volumeParams, samplePos, " + channel + ")";
            valueAll = "gradientAllForwardDiff(voxel, volume, volumeParams, samplePos)";
            break;
        case RaycastingProperty::GradientComputation::Backward:
            value =
                "gradientBackwardDiff(voxel, volume, volumeParams, samplePos, " + channelDef + ")";
            valueChannel =
                "gradientBackwardDiff(voxel, volume, volumeParams, samplePos, " + channel + ")";
            valueAll = "gradientAllBackwardDiff(voxel, volume, volumeParams, samplePos)";
            break;
        case RaycastingProperty::GradientComputation::Central:
            value =
                "gradientCentralDiff(voxel, volume, volumeParams, samplePos, " + channelDef + ")";
            valueChannel =
                "gradientCentralDiff(voxel, volume, volumeParams, samplePos, " + channel + ")";
            valueAll = "gradientAllCentralDiff(voxel, volume, volumeParams, samplePos)";
            break;
        case RaycastingProperty::GradientComputation::CentralHigherOrder:
            value =
                "gradientCentralDiffH(voxel, volume, volumeParams, samplePos, " + channelDef + ")";
            valueChannel =
                "gradientCentralDiffH(voxel, volume, volumeParams, samplePos, " + channel + ")";
            valueAll = "gradientAllCentralDiffH(voxel, volume, volumeParams, samplePos)";
            break;
        case RaycastingProperty::GradientComputation::PrecomputedXYZ:
            value = valueChannel = valueAll = "gradientPrecomputedXYZ(voxel, volumeParams)";
            break;
        case RaycastingProperty::GradientComputation::PrecomputedYZW:
            value = valueChannel = valueAll = "gradientPrecomputedYZW(voxel, volumeParams)";
            break;
    }

    // gradient for channel 1
    const std::string key = "COMPUTE_GRADIENT(voxel, volume, volumeParams, samplePos)";
    // gradient for specific channel
    const std::string keyChannel =
        "COMPUTE_GRADIENT_FOR_CHANNEL(voxel, volume, volumeParams, samplePos, channel)";
    // gradients for all channels
    const std::string keyAll = "COMPUTE_ALL_GRADIENTS(voxel, volume, volumeParams, samplePos)";

    shader.getFragmentShaderObject()->addShaderDefine(key, value);
    shader.getFragmentShaderObject()->addShaderDefine(keyChannel, valueChannel);
    shader.getFragmentShaderObject()->addShaderDefine(keyAll, valueAll);

    if (property.get() != RaycastingProperty::GradientComputation::None) {
        shader.getFragmentShaderObject()->addShaderDefine("GRADIENTS_ENABLED");
    } else {
        shader.getFragmentShaderObject()->removeShaderDefine("GRADIENTS_ENABLED");
    }
}

void setShaderUniforms(Shader& shader, const RaycastingProperty& property) {
    shader.setUniform("samplingRate_", property.samplingRate_.get());
}

void setShaderUniforms(Shader& shader, const RaycastingProperty& property, std::string name) {
    shader.setUniform(name + ".samplingRate", property.samplingRate_.get());
}

void setShaderUniforms(Shader& shader, const SpatialEntity<3>& object, const std::string& name) {
    const SpatialCoordinateTransformer<3>& ct = object.getCoordinateTransformer();

    mat4 dataToWorldMatrix = ct.getDataToWorldMatrix();
    mat4 modelToWorldMatrix = ct.getModelToWorldMatrix();

    shader.setUniform(name + ".dataToModel", ct.getDataToModelMatrix());
    shader.setUniform(name + ".modelToData", ct.getModelToDataMatrix());

    shader.setUniform(name + ".dataToWorld", dataToWorldMatrix);
    shader.setUniform(name + ".worldToData", ct.getWorldToDataMatrix());

    shader.setUniform(name + ".modelToWorld", modelToWorldMatrix);
    shader.setUniform(name + ".worldToModel", ct.getWorldToModelMatrix());
    shader.setUniform(name + ".modelToWorldNormalMatrix",
                      glm::mat3(glm::transpose(glm::inverse(modelToWorldMatrix))));

    shader.setUniform(name + ".dataToWorldNormalMatrix",
                      glm::mat3(glm::transpose(glm::inverse(dataToWorldMatrix))));
}

void addShaderDefines(Shader& shader, const SimpleRaycastingProperty& property) {
    // gradient for channel 1
    std::string gradientComputationKey = "COMPUTE_GRADIENT(voxel, volume, volumeParams, samplePos)";
    // gradient for specific channel
    std::string singleChannelGradientKey =
        "COMPUTE_GRADIENT_FOR_CHANNEL(voxel, volume, volumeParams, samplePos, channel)";
    // gradients for all channels
    std::string allChannelsGradientKey =
        "COMPUTE_ALL_GRADIENTS(voxel, volume, volumeParams, samplePos)";

    std::string gradientValue = "";
    std::string singleChannelGradientValue = "";
    std::string allChannelsGradientValue = "";
    std::string channel = "channel";
    std::string defaultChannel = "0";

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
            "gradientForwardDiff(voxel, volume, volumeParams, samplePos, " + defaultChannel + ")";
        singleChannelGradientValue =
            "gradientForwardDiff(voxel, volume, volumeParams, samplePos, " + channel + ")";
        allChannelsGradientValue = "gradientAllForwardDiff(voxel, volume, volumeParams, samplePos)";
    } else if (property.gradientComputationMode_.isSelectedIdentifier("central")) {
        gradientValue =
            "gradientCentralDiff(voxel, volume, volumeParams, samplePos, " + defaultChannel + ")";
        singleChannelGradientValue =
            "gradientCentralDiff(voxel, volume, volumeParams, samplePos, " + channel + ")";
        allChannelsGradientValue = "gradientAllCentralDiff(voxel, volume, volumeParams, samplePos)";
    } else if (property.gradientComputationMode_.isSelectedIdentifier("central-higher")) {
        gradientValue =
            "gradientCentralDiffH(voxel, volume, volumeParams, samplePos, " + defaultChannel + ")";
        singleChannelGradientValue =
            "gradientCentralDiffH(voxel, volume, volumeParams, samplePos, " + channel + ")";
        allChannelsGradientValue =
            "gradientAllCentralDiffH(voxel, volume, volumeParams, samplePos)";
    } else if (property.gradientComputationMode_.isSelectedIdentifier("backward")) {
        gradientValue =
            "gradientBackwardDiff(voxel, volume, volumeParams, samplePos, " + defaultChannel + ")";
        singleChannelGradientValue =
            "gradientBackwardDiff(voxel, volume, volumeParams, samplePos, " + channel + ")";
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

    shader.getFragmentShaderObject()->addShaderDefine(gradientComputationKey, gradientValue);
    shader.getFragmentShaderObject()->addShaderDefine(singleChannelGradientKey,
                                                      singleChannelGradientValue);
    shader.getFragmentShaderObject()->addShaderDefine(allChannelsGradientKey,
                                                      allChannelsGradientValue);

    if (property.gradientComputationMode_.isSelectedIdentifier("none")) {
        shader.getFragmentShaderObject()->removeShaderDefine("GRADIENTS_ENABLED");
    } else {
        shader.getFragmentShaderObject()->addShaderDefine("GRADIENTS_ENABLED");
    }

    // classification defines, red channel is used
    std::string classificationKey = "APPLY_CLASSIFICATION(transferFunc, voxel)";
    std::string classificationValue = "";
    if (property.classificationMode_.isSelectedIdentifier("none"))
        classificationValue = "vec4(voxel.r)";
    else if (property.classificationMode_.isSelectedIdentifier("transfer-function"))
        classificationValue = "applyTF(transferFunc, voxel.r)";
    else if (property.classificationMode_.isSelectedIdentifier("voxel-value"))
        classificationValue = "voxel";
    shader.getFragmentShaderObject()->addShaderDefine(classificationKey, classificationValue);

    // classification of specific channel
    classificationKey = "APPLY_CHANNEL_CLASSIFICATION(transferFunc, voxel, channel)";
    classificationValue = "";
    if (property.classificationMode_.isSelectedIdentifier("none"))
        classificationValue = "vec4(voxel[channel])";
    else if (property.classificationMode_.isSelectedIdentifier("transfer-function"))
        classificationValue = "applyTF(transferFunc, voxel, channel)";
    else if (property.classificationMode_.isSelectedIdentifier("voxel-value"))
        classificationValue = "voxel";
    shader.getFragmentShaderObject()->addShaderDefine(classificationKey, classificationValue);

    // compositing defines
    std::string compositingKey =
        "APPLY_COMPOSITING(result, color, samplePos, voxel, gradient, camera, isoValue, t, tDepth, "
        "tIncr)";
    std::string compositingValue = "result";

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

    shader.getFragmentShaderObject()->addShaderDefine(compositingKey, compositingValue);
}

void setShaderUniforms(Shader& shader, const SimpleRaycastingProperty& property) {
    shader.setUniform("samplingRate_", property.samplingRate_.get());
    shader.setUniform("isoValue_", property.isoValue_.get());
}

void setShaderUniforms(Shader& shader, const SimpleRaycastingProperty& property, std::string name) {
    shader.setUniform(name + ".samplingRate", property.samplingRate_.get());
    shader.setUniform(name + ".isoValue", property.isoValue_.get());
}

void addShaderDefines(Shader& shader, const IsoValueProperty& property) {
    auto isovalueCount = property.get().size();

    // need to ensure there is always at least one isovalue due to the use of the macro
    // as array size in IsovalueParameters
    shader.getFragmentShaderObject()->addShaderDefine("MAX_ISOVALUE_COUNT",
                                                      toString(std::max<size_t>(1, isovalueCount)));

    if (!property.get().empty()) {
        shader.getFragmentShaderObject()->addShaderDefine("ISOSURFACE_ENABLED");
    } else {
        shader.getFragmentShaderObject()->removeShaderDefine("ISOSURFACE_ENABLED");
    }
}

void setShaderUniforms(Shader& shader, const IsoValueProperty& property) {
    auto data = property.get().getVectorsf();

    shader.setUniform("isovalues", data.first.size(), data.first.data());
    shader.setUniform("isosurfaceColors", data.second.size(), data.second.data());
}

void setShaderUniforms(Shader& shader, const IsoValueProperty& property, std::string name) {
    auto data = property.get().getVectorsf();

    shader.setUniform(name + ".values", data.first.size(), data.first.data());
    shader.setUniform(name + ".colors", data.second.size(), data.second.data());
}

void addShaderDefines(Shader& shader, const IsoTFProperty& property) {
    addShaderDefines(shader, property.isovalues_);
}

void setShaderUniforms(Shader& shader, const IsoTFProperty& property) {
    setShaderUniforms(shader, property.isovalues_);
}

void setShaderUniforms(Shader& shader, const IsoTFProperty& property, std::string name) {
    setShaderUniforms(shader, property.isovalues_, property.isovalues_.getIdentifier());
}

void addShaderDefinesBGPort(Shader& shader, const ImageInport& port) {
    std::string bgKey = "DRAW_BACKGROUND(result,t,tIncr,color,bgTDepth,tDepth)";
    if (port.isConnected()) {
        shader.getFragmentShaderObject()->addShaderDefine("BACKGROUND_AVAILABLE");
        shader.getFragmentShaderObject()->addShaderDefine(
            bgKey, "drawBackground(result,t,tIncr, texture(bgColor,texCoords),bgTDepth,tDepth)");
    } else {
        shader.getFragmentShaderObject()->removeShaderDefine("BACKGROUND_AVAILABLE");
        shader.getFragmentShaderObject()->addShaderDefine(bgKey, "result");
    }
}

void addShaderDefines(Shader& shader, const VolumeIndicatorProperty& indicator) {
    // compositing defines
    std::string key =
        "DRAW_PLANES(result, samplePosition, rayDirection, increment, params,t,tDepth)";
    std::string value = "result";

    if (indicator && (indicator.plane1_ || indicator.plane2_ || indicator.plane3_)) {
        std::string planes("");
        planes += indicator.plane1_ ? ", params.plane1" : "";
        planes += indicator.plane2_ ? ", params.plane2" : "";
        planes += indicator.plane3_ ? ", params.plane3" : "";
        value =
            "drawPlanes(result, samplePosition, rayDirection, increment " + planes + ",t,tDepth)";

        shader.getFragmentShaderObject()->addShaderDefine("PLANES_ENABLED");
    } else {
        shader.getFragmentShaderObject()->removeShaderDefine("PLANES_ENABLED");
    }
    shader.getFragmentShaderObject()->addShaderDefine(key, value);
}

void setShaderUniforms(Shader& shader, const VolumeIndicatorProperty& indicator, std::string name) {
    if (indicator) {
        if (indicator.plane1_) {
            shader.setUniform(name + ".plane1.position", indicator.plane1_.position_);
            shader.setUniform(name + ".plane1.normal", indicator.plane1_.normal_);
            shader.setUniform(name + ".plane1.color", indicator.plane1_.color_);
        }
        if (indicator.plane2_) {
            shader.setUniform(name + ".plane2.position", indicator.plane2_.position_);
            shader.setUniform(name + ".plane2.normal", indicator.plane2_.normal_);
            shader.setUniform(name + ".plane2.color", indicator.plane2_.color_);
        }
        if (indicator.plane3_) {
            shader.setUniform(name + ".plane3.position", indicator.plane3_.position_);
            shader.setUniform(name + ".plane3.normal", indicator.plane3_.normal_);
            shader.setUniform(name + ".plane3.color", indicator.plane3_.color_);
        }
    }
}

void addShaderDefines(Shader& shader, const StipplingProperty& property) {
    addShaderDefines(shader, property.mode_.get());
}

void addShaderDefines(Shader& shader, const StipplingProperty::Mode& mode) {
    std::string value;
    switch (mode) {
        case StipplingProperty::Mode::ScreenSpace:
            value = "1";
            break;
        case StipplingProperty::Mode::WorldSpace:
            value = "2";
            break;
        case StipplingProperty::Mode::None:
        default:
            break;
    }

    auto fragShader = shader.getFragmentShaderObject();
    if (mode != StipplingProperty::Mode::None) {
        fragShader->addShaderDefine("ENABLE_STIPPLING");
    } else {
        fragShader->removeShaderDefine("ENABLE_STIPPLING");
    }
    fragShader->addShaderDefine("STIPPLE_MODE", value);
}

void setShaderUniforms(Shader& shader, const StipplingProperty& property, std::string name) {
    shader.setUniform(name + ".length", property.length_.get());
    shader.setUniform(name + ".spacing", property.spacing_.get());
    shader.setUniform(name + ".offset", property.offset_.get());
    shader.setUniform(name + ".worldScale", property.worldScale_.get());
}

int getLogLineNumber(const std::string& compileLogLine) {
    int result = -1;
    std::istringstream input(compileLogLine);
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
    std::vector<std::string> elems;
    std::stringstream ss(compileLogLine);
    std::string item;
    while (std::getline(ss, item, ':')) {
        elems.push_back(item);
    }
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

std::shared_ptr<const ShaderResource> findShaderResource(const std::string& fileName) {
    auto resource = ShaderManager::getPtr()->getShaderResource(fileName);
    if (!resource) {
        throw OpenGLException(
            "Shader file: " + fileName + " not found in shader search paths or shader resources.",
            IVW_CONTEXT_CUSTOM("ShaderUtils"));
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
