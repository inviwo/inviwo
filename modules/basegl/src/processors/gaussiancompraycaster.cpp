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












#include <modules/basegl/processors/gaussiancompraycaster.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <modules/opengl/glformats.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/buffer/buffergl.h>
#include <modules/opengl/buffer/bufferobject.h>
#include <modules/opengl/texture/texture3d.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <modules/base/algorithm/dataminmax.h>  // for dataMinMax
#include <inviwo/core/util/glmcomp.h>           // for compMul
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <modules/opengl/image/imagegl.h>                      // for ImageGL
#include <modules/opengl/texture/texture2d.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/shader/stringshaderresource.h>

#include <modules/opengl/openglmodule.h>

#include <inviwo/core/algorithm/boundingbox.h>                          // for boundingBox
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/ports/volumeport.h>                               // for VolumeInport
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tag, Tags::GL, Tags
#include <inviwo/core/properties/isotfproperty.h>                       // for IsoTFProperty
#include <inviwo/core/util/formats.h>                                   // for DataFormatBase
#include <inviwo/core/util/zip.h>                                       // for zipper
#include <modules/basegl/processors/raycasting/volumeraycasterbase.h>   // for VolumeRaycasterBase
#include <modules/basegl/shadercomponents/cameracomponent.h>            // for CameraComponent
#include <modules/basegl/shadercomponents/isotfcomponent.h>             // for IsoTFComponent
#include <modules/basegl/shadercomponents/raycastingcomponent.h>        // for RaycastingComponent
#include <modules/basegl/shadercomponents/volumecomponent.h>            // for VolumeComponent

#include <modules/basegl/properties/linesettingsproperty.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo GaussianCompRayCaster::processorInfo_{
    "org.inviwo.GaussianCompRayCaster",  // Class identifier
    "Gaussian Comp Ray Caster",        // Display name
    "Undefined",                   // Category
    CodeState::Experimental,       // Code state
    Tags::None,                    // Tags
    R"(<Explanation of how to use the processor.>)"_unindentHelp};

const ProcessorInfo& GaussianCompRayCaster::getProcessorInfo() const { return processorInfo_; }


namespace {

constexpr std::string_view copyDepthVertexShader = util::trim(R"(
out Fragment {
    vec2 texCoord;
} out_vert;

void main() {
    out_vert.texCoord = in_TexCoord.xy;
    gl_Position = in_Vertex;
}
)");

constexpr std::string_view copyDepthFragShader = util::trim(R"(
uniform sampler2D depthTex;

in Fragment {
    in vec2 texCoord;
} in_frag;

void main() {
    gl_FragDepth = texture(depthTex, in_frag.texCoord).r;
}
)");

}  // namespace



void GaussianCompRayCaster::initializeResources() {
    // auto* compShader = shaderGaussian_.getComputeShaderObject();

    // compShader->addShaderDefine("MY_DEFINE", toString(2));
    shaderGaussian_.build();
}



GaussianCompRayCaster::GaussianCompRayCaster()
    : Processor{}
    , orbitals_{"orbitals", "Imported orbitals"_help}
    , outport_{"outport"}
    , shaderGaussian_{{{ShaderType::Compute, "gaussianraycaster.comp"}}}
    , copyDepthShader_{{{ShaderType::Vertex, std::make_shared<StringShaderResource>(
                                                 "CopyDepth.vert", copyDepthVertexShader)},
                        {ShaderType::Fragment, std::make_shared<StringShaderResource>(
                                                   "CopyDepth.frag", copyDepthFragShader)}}}
    , dimensions_("dimensions", "Dimensions", size2_t(64), size2_t(1), size2_t(1024))
    , groupSize_("groupsize", "GroupSize", size2_t(16), size2_t(1), size2_t(64))
    , sigma_("sigma", "Sigma", 1.0f, 0.0f, 10.0f)
    , nSteps_("nSamples", "Number of samples", 100, 0, 500)
    , minValue_{"minValue"}
    , maxValue_{"maxvalue"}
    , paddMin_{"paddMinValue", "padd Min Value"_help}
    , paddMax_{"paddMaxValue", "padd Min Value"_help}
    , cam_{"camera", "CameraProperty"}
    , trackball_{&cam_}
    , isotfComposite_("isotfComposite", "TF & Isovalues")
    , reset_{"Reset", "Reset", [&]() {}}
    {

    addProperties(dimensions_, groupSize_, sigma_, nSteps_, cam_, trackball_, isotfComposite_,reset_);
    
    addPorts(orbitals_, outport_,minValue_,maxValue_,paddMin_,paddMax_);

    shaderGaussian_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    copyDepthShader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}



void GaussianCompRayCaster::process() {
    /*
    TextureUnit unit;
    unit.activate();

    auto img = std::make_shared<Image>(dimensions_, DataFormat<float>::get());

    auto layerGL = img->getColorLayer()->getEditableRepresentation<LayerGL>();
    auto texHandle = layerGL->getTexture()->getID();
    glBindImageTexture(unit.getUnitNumber(), texHandle, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

    layerGL->setSwizzleMask(swizzlemasks::luminance);   
    shaderGaussian_.activate();

    shaderGaussian_.setUniform("dest", unit.getUnitNumber());
    utilgl::setUniforms(shaderGaussian_, sigma_);
    auto ptr = points_.getData();
    std::vector<vec4> points;
    points.assign(ptr->begin(), ptr->end());

    size2_t dims = dimensions_;
    size2_t groupSize = groupSize_;
    
    
    
    GLuint buffHandle;
    glGenBuffers(1, &buffHandle);  // (Gives me a free buffer name that is not currently in use)
    // size_t bufferSize{points_.getData()->size() * sizeof(vec4)};
    size_t bufferSize{points.size() * sizeof(vec4)};
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffHandle);
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, points.data(), GL_STATIC_DRAW);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, buffHandle, 0, bufferSize);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    

    uvec2 numWorkGroups{(uvec2(dims) + uvec2(groupSize) - uvec2(1)) / uvec2(groupSize)};


    glDispatchCompute(numWorkGroups.x, numWorkGroups.y, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    shaderGaussian_.deactivate();

    std::vector<float> tmp;
    tmp.resize(glm::compMul(img->getDimensions()));
    layerGL->getTexture()->download(tmp.data());

    outport_.setData(img);

    */

/*----------------------------*/

    // image dims from Canvas
    const size2_t outputDims = outport_.getDimensions();
    
   
    auto img = std::make_shared<Image>(outputDims, DataFormat<vec4>::get());
    img->getColorLayer()->setSwizzleMask(swizzlemasks::rgba);
    auto colorDepthLayer =
        std::make_shared<Layer>(LayerConfig{.dimensions = img->getDimensions(),
                                            .format = DataFormat<float>::get(),
                                            .swizzleMask = swizzlemasks::defaultColor(1)});


    auto colorGL = img->getColorLayer(0)->getRepresentation<LayerGL>();   
    auto colorDepthGL = colorDepthLayer->getRepresentation<LayerGL>();
    
    //auto texHandle = colorGL->getTexture()->getID();
    
    //GL_DEPTH_COMPONENT32F
    glBindImageTexture(0, colorGL->getTexture()->getID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(1, colorDepthGL->getTexture()->getID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);


    //auto depthLayerGL = img->getDepthLayer()->getEditableRepresentation<LayerGL>();
    //auto depthTexHandle = depthLayerGL->getTexture()->getID();
    //glBindImageTexture(1, depthTexHandle, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_DEPTH_COMPONENT32F);

    shaderGaussian_.activate();
    utilgl::setUniforms(shaderGaussian_, sigma_);
    utilgl::setUniforms(shaderGaussian_, nSteps_);

    
    auto ptr = orbitals_.getData();
    auto points = *ptr;
    
    
    
    //std::vector<vec4> points;
    //points.assign(ptr->begin(), ptr->end());
    shaderGaussian_.setUniform("numPoints", static_cast<unsigned>(points.size()));
    shaderGaussian_.setUniform("minValue_",*minValue_.getData().get());
    shaderGaussian_.setUniform("maxValue_", *maxValue_.getData().get());

    shaderGaussian_.setUniform("paddMin", *paddMin_.getData());
    shaderGaussian_.setUniform("paddMax", *paddMax_.getData());

    GLuint buffHandle;
    glGenBuffers(1, &buffHandle);  // (Gives me a free buffer name that is not currently in use)
    // size_t bufferSize{points_.getData()->size() * sizeof(vec4)};
    size_t bufferSize{points.size() * sizeof(GaussianOrbital)};
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffHandle);
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, points.data(), GL_STATIC_DRAW);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 2, buffHandle, 0, bufferSize);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER,2);
    
    


    //vec3 cameraPos = cam_.getLookFrom();
    //vec3 cameraRight = cam_.getLookRight();
    //vec3 cameraUp = cam_.getLookUp();
    mat4 pMatrix = cam_.projectionMatrix();
    mat4 vMatrix = cam_.viewMatrix();
    mat4 pMatrixInv = glm::inverse(pMatrix);
    mat4 vMatrixInv = glm::inverse(pMatrix);


    //shaderGaussian_.setUniform("cameraPos", cameraPos);
    
    //shaderGaussian_.setUniform("upU", cameraUp);
    //shaderGaussian_.setUniform("cameraRight", cameraRight);
    //shaderGaussian_.setUniform("cameraUp", cameraUp);
    shaderGaussian_.setUniform("projectionMatrix", pMatrix);
    shaderGaussian_.setUniform("viewMatrix", vMatrix);
    
    //shaderGaussian_.setUniform("cameraPos", cameraPos);
    TextureUnitContainer units;
    utilgl::bindAndSetUniforms(shaderGaussian_, units, isotfComposite_);
    utilgl::setUniforms(shaderGaussian_, isotfComposite_,cam_);
    //shaderGaussian_.setUniform("projectionMatrixInv", pMatrixInv);
    //shaderGaussian_.setUniform("viewMatrixInv", vMatrixInv);

    size2_t g{groupSize_};
    size2_t d{dimensions_};

    const uvec2 groupSize{g};
    const uvec2 dims{d};
    uvec2 numGroups{(dims + groupSize - uvec2{1}) / groupSize};




    glDispatchCompute(numGroups.x, numGroups.y, 1);  // 512^2 threads in blocks of 16^2

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    shaderGaussian_.deactivate();

    std::vector<float> tmp;
    tmp.resize(glm::compMul(img->getDimensions()));
    //layerGL->getTexture()->download(tmp.data());
    
    utilgl::activateTarget(*img, ImageType::ColorDepth);
    // disable writing to color
    utilgl::ColorMaskState colorMask{true, false, false, false};

    TextureUnit depthUnit;
    colorDepthGL->bindTexture(depthUnit);

    copyDepthShader_.activate();
    copyDepthShader_.setUniform("depthTex", depthUnit.getUnitNumber());
    utilgl::singleDrawImagePlaneRect();
    copyDepthShader_.deactivate();

    utilgl::deactivateCurrentTarget();

    outport_.setData(img);

    
    

    /*
    uvec2 numWorkGroups{(uvec2(dims) + uvec2(groupSize) - uvec2(1)) /
                        uvec2(groupSize)};  // Divide by work group size (e.g., 4)
    
    
    
    

   

    glDispatchCompute(numWorkGroups.x, numWorkGroups.y, 1);

    
    
    

    shaderGaussian_.deactivate();

    
    
    outport_.setData(img);
    */


    // outport_.setData(std::make_shared<SomeOtherData>(position_.get()));


    /* auto ptr = points_.getData();

    std::vector<vec4> points;
    points.assign(ptr->begin(), ptr->end());





    GLuint buffHandle;
    glGenBuffers(1, &buffHandle);  // (Gives me a free buffer name that is not currently in use)
    // size_t bufferSize{points_.getData()->size() * sizeof(vec4)};
    size_t bufferSize{points.size() * sizeof(vec4)};
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffHandle);
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, points.data(), GL_STATIC_DRAW);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, buffHandle, 0, bufferSize);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    */
}

}  // namespace inviwo
