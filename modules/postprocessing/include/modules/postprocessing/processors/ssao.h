/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#ifndef IVW_SSAO_H
#define IVW_SSAO_H

#include <modules/postprocessing/postprocessingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/ports/imageport.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/shader/shader.h>

namespace inviwo {

/** \docpage{org.inviwo.SSAO, SSAO}
 * ![](org.inviwo.SSAO.png?classIdentifier=org.inviwo.SSAO)
 * Use any image with a proper depth channel as input. It will compute Screen space ambient
 * occlusion (SSAO) using the depth and then apply the occlusion to the color-layer.
 *
 * ### Inports
 *   * __ImageInport__ Input Image.
 *
 * ### Outports
 *   * __ImageOutport__ Output Image.
 *
 * ### Properties
 *   * __Technique__ SSAO Technique.
 *   * __Radius__ Radius of hemisphere used to compute the ambient occlusion.
 *   * __Intensity__ Intensity of the ambient occlusion.
 *   * __Angle Bias__ Offsets the minimum angle of samples used in the computation. (Good for hiding
 * AO effects on low-tess geometry)
 *   * __Directions__ Number of directions used to sample the hemisphere.
 *   * __Steps/Dir__ Number of samples used for each direction.
 *   * __Use Normal__ Orients the hemisphere using an approximated surface normal.
 *   * __Enable Blur__ Apply a bilateral blur filter.
 *   * __Blur Sharpness__ Controls the sharpness of the blur, small number -> large filter
 */

/**
 * \class SSAO
 * \brief Screen space ambient occlusion post process. (Computed using depth layer)
 */
class IVW_MODULE_POSTPROCESSING_API SSAO : public Processor {
public:
    SSAO();
    virtual ~SSAO();

    virtual void initializeResources() override;
    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    static constexpr int AO_RANDOM_TEX_SIZE = 4;
    static constexpr int HBAO_RANDOM_SIZE = AO_RANDOM_TEX_SIZE;
    static constexpr int HBAO_RANDOM_ELEMENTS = HBAO_RANDOM_SIZE * HBAO_RANDOM_SIZE;
    static constexpr int MAX_SAMPLES = 1;  // CHANGE TO 8 WHEN IMPLEMENTING MSAA

private:
    struct ProjectionParam {
        float nearplane;
        float farplane;
        float fov;
        float orthoheight;
        bool ortho;
        mat4 matrix;
    };

    struct HBAOData {
        float RadiusToScreen;
        float R2;
        float NegInvR2;
        float NDotVBias;

        vec2 InvFullResolution;
        vec2 InvQuarterResolution;

        float AOMultiplier;
        float PowExponent;
        vec2 _pad0;

        vec4 projInfo;
        vec2 projScale;
        int projOrtho;
        int _pad1;

        vec4 float2Offsets[AO_RANDOM_TEX_SIZE * AO_RANDOM_TEX_SIZE];
        vec4 jitters[AO_RANDOM_TEX_SIZE * AO_RANDOM_TEX_SIZE];
    };

    void initHbao();
    void initFramebuffers(int width, int height);
    void prepareHbaoData(const ProjectionParam& proj, int width, int height);
    void drawLinearDepth(GLuint texDepth, const ProjectionParam& proj);
    void drawHbaoClassic(GLuint fboOut, GLuint texDepth, const ProjectionParam& proj, int width,
                         int height);
    void drawHbaoBlur(GLuint fboOut, const ProjectionParam& proj, int width, int height);

    ImageInport inport_;
    ImageOutport outport_;

    BoolProperty enable_;
    OptionPropertyInt technique_;
    FloatProperty radius_;
    FloatProperty intensity_;
    FloatProperty bias_;
    IntProperty directions_;
    IntProperty steps_;
    BoolProperty useNormal_;
    BoolProperty enableBlur_;
    FloatProperty blurSharpness_;
    CameraProperty camera_;

    Shader depthLinearize_;
    Shader hbaoCalc_;
    Shader hbaoCalcBlur_;
    Shader hbaoBlurHoriz_;
    Shader hbaoBlurVert_;

    /*
    // For future things
    Shader viewNormal_;
    Shader hbao2Deinterleave_;
    Shader hbao2Calc_;
    Shader hbao2CalcBlur_;
    Shader hbao2Reinterleave_;
    Shader hbao2ReinterleaveBlur_;
    */

    GLuint hbaoUbo_;

    struct {
        GLuint depthLinear = 0;
        GLuint hbaoCalc = 0;
        // GLuint viewNormal = 0;

        int width = 0;
        int height = 0;
    } framebuffers_;

    struct {
        GLuint depthLinear = 0;
        GLuint hbaoResult = 0;
        GLuint hbaoBlur = 0;
        GLuint hbaoRandom = 0;

        /*
        // For future things
        GLuint viewNormal = 0;
        GLuint hbao2depthArray = 0;
        GLuint randomView[MAX_SAMPLES];
        */
    } textures_;

    struct {
        GLint depthLinearClipInfo = -1;
        GLint depthLinearInputTexture = -1;

        GLint hbaoControlBuffer = -1;
        GLint hbaoTexLinearDepth = -1;
        GLint hbaoTexRandom = -1;

        GLint hbaoBlurSharpness = -1;
        GLint hbaoBlurInvResolutionDirection = -1;
        GLint hbaoBlurTexSource = -1;

        /*
        // For future things
        GLint viewNormalProjInfo = -1;
        GLint viewNormalProjOrtho = -1;
        GLint viewNormalInvFullResolution = -1;
        GLint viewNormalTexLinearDepth = -1;
        */
    } locations_;

    ProjectionParam projParam_;
    HBAOData hbaoUboData_;
};

}  // namespace inviwo

#endif  // IVW_SSAO_H
