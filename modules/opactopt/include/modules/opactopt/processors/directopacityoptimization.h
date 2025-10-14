/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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
#include <modules/opactopt/opactoptmoduledefine.h>

#include <modules/opactopt/utils/approximation.h>
#include <modules/opengl/texture/texture1d.h>
#include <modules/opengl/texture/texture2darray.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/buffer/bufferobject.h>
#include <inviwo/core/ports/volumeport.h>
#include <modules/opengl/openglcapabilities.h>

#include <inviwo/core/interaction/cameratrackball.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/simplelightingproperty.h>
#include <modules/opengl/shader/shader.h>
#include <modules/basegl/properties/linesettingsproperty.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/util/glmvec.h>
#include <modules/opengl/glformats.h>
#include <modules/opengl/inviwoopengl.h>
#include <string>

namespace inviwo {

class IVW_MODULE_OPACTOPT_API OpacityOptimization : public Processor {
public:
    OpacityOptimization();

    OpacityOptimization(const OpacityOptimization&) = delete;
    OpacityOptimization& operator=(const OpacityOptimization&) = delete;
    OpacityOptimization(OpacityOptimization&&) = delete;
    OpacityOptimization& operator=(OpacityOptimization&&) = delete;

    virtual ~OpacityOptimization();

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void initializeResources() override;
    virtual void process() override;
    virtual void setNetwork(ProcessorNetwork* network) override;

protected:
    using Type = approximations::Type;
    using enum approximations::Type;

    struct Units {
        TextureUnit importanceSumMain;
        TextureUnit opticalDepth;

        std::optional<TextureUnit> gaussianKernel;
        std::optional<TextureUnit> importanceSumSmooth;

        std::optional<TextureUnit> importanceVolume;
        std::optional<TextureUnit> legendreCoeffs;
        std::optional<TextureUnit> momentSettings;
    };

    enum class Pass : std::uint8_t {
        ProjectImportance = 0,
        ApproximateImportance = 1,
        ApproximateBlending = 2
    };

    void setUniforms(Shader& shader, Units& units);
    void buildShaders();
    void renderGeometry(Pass pass, Units& units);
    static void resizeTexture(Texture2DArray& texture, size2_t size, size_t depth);
    void resizeImportanceSumTextures(size2_t screenSize, size_t importanceSumCoefficients);
    void resizeOpticalDepthTexture(size2_t screenSize, size_t opticalDepthCoefficients);
    void generateAndUploadGaussianKernel();
    void generateAndUploadLegendreCoefficients();
    void generateAndUploadMomentSettings();

    MeshFlatMultiInport inport_;
    ImageInport backgroundPort_;
    ImageOutport outport_;
    Image intermediateImage_;

    CameraProperty camera_;

    // Mesh properties
    CompositeProperty meshProperties_;
    BoolProperty overrideColorBuffer_;
    FloatVec4Property overrideColor_;

    // Line properties
    LineSettingsProperty lineSettings_;

    // Point properties
    CompositeProperty pointProperties_;
    FloatProperty pointSize_;
    FloatProperty borderWidth_;
    FloatVec4Property borderColor_;
    FloatProperty antialising_;

    // General properties
    SimpleLightingProperty lightingProperty_;
    CameraTrackball trackball_;

    // Shaders for each rendering pass and primitive type
    std::array<Shader, 3> meshShaders_;
    std::array<Shader, 3> lineShaders_;
    std::array<Shader, 3> lineAdjacencyShaders_;
    std::array<Shader, 3> pointShaders_;

    // Screen space shaders
    std::array<Shader, 2> smooth_;
    Shader clear_;
    Shader normalize_;

    // Optional importance volume
    VolumeInport importanceVolume_;

    // Opacity optimisation settings
    FloatProperty occlusionReduction_;
    FloatProperty clutterReduction_;
    FloatProperty lambda_;
    CompositeProperty approximationProperties_;
    OptionProperty<Type> approximationMethod_;
    IntProperty importanceSumCoefficients_;
    IntProperty opticalDepthCoefficients_;
    BoolProperty normalizedBlending_;
    FloatProperty coeffTexFixedPointFactor_;

    GLFormat imageFormat_;

    std::array<Texture2DArray, 2> importanceSumTexture_;
    Texture2DArray opticalDepthTexture_;

    Texture1D gaussianKernel_;
    BoolCompositeProperty smoothing_;
    static constexpr int gaussianKernelMaxRadius_ = 50;
    IntProperty gaussianRadius_;
    FloatProperty gaussianSigma_;

    Texture1D legendreCoeffs_;
    Texture1D momentSettings_;
};

}  // namespace inviwo
