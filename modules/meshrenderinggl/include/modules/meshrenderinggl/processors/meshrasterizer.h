/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2023 Inviwo Foundation
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

#include <modules/meshrenderinggl/meshrenderingglmoduledefine.h>  // for IVW_MODULE_MESHRENDE...

#include <inviwo/core/ports/meshport.h>                              // for MeshFlatMultiInport
#include <inviwo/core/processors/processor.h>                        // for Processor
#include <inviwo/core/processors/processorinfo.h>                    // for ProcessorInfo
#include <inviwo/core/properties/boolcompositeproperty.h>            // for BoolCompositeProperty
#include <inviwo/core/properties/boolproperty.h>                     // for BoolProperty
#include <inviwo/core/properties/buttonproperty.h>                   // for ButtonProperty
#include <inviwo/core/properties/compositeproperty.h>                // for CompositeProperty
#include <inviwo/core/properties/optionproperty.h>                   // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>                  // for FloatProperty, Float...
#include <inviwo/core/properties/simplelightingproperty.h>           // for SimpleLightingProperty
#include <inviwo/core/properties/transferfunctionproperty.h>         // for TransferFunctionProp...
#include <inviwo/core/util/document.h>                               // for Document
#include <inviwo/core/util/glmmat.h>                                 // for mat4
#include <inviwo/core/util/glmvec.h>                                 // for ivec2
#include <inviwo/core/util/staticstring.h>                           // for operator+
#include <modules/base/properties/transformlistproperty.h>           // for TransformListProperty
#include <modules/meshrenderinggl/algorithm/calcnormals.h>           // for CalculateMeshNormals...
#include <modules/meshrenderinggl/datastructures/rasterization.h>    // for Rasterization
#include <modules/meshrenderinggl/ports/rasterizationport.h>         // for RasterizationOutport
#include <modules/meshrenderinggl/rendering/fragmentlistrenderer.h>  // for FragmentListRenderer

#include <array>        // for array
#include <functional>   // for __base, function
#include <memory>       // for shared_ptr
#include <string>       // for operator==, operator+
#include <string_view>  // for operator==, string_view
#include <vector>       // for operator!=, vector

namespace inviwo {
class Layer;
class Mesh;
class Shader;

/**
 * \brief Mesh Renderer specialized for rendering highly layered and transparent surfaces.
 *
 * Its settings will be used to add fragments to a FragmentListRenderer for the rendering of the
 * transparent mesh. Many alpha computation modes, shading modes, color modes can be combined and
 * even selected individually for the front- and back face.
 */
class IVW_MODULE_MESHRENDERINGGL_API MeshRasterizer : public RasterizationProcessor {
    friend class MeshRasterization;

public:
    MeshRasterizer();
    virtual ~MeshRasterizer() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void initializeResources() override;

    virtual void rasterize(const ivec2& imageSize, const mat4& worldMatrixTransform,
                           std::function<void(Shader&)> setUniforms,
                           std::function<void(Shader&)> initializeShader) override;

    virtual bool usesFragmentLists() const override {
        return !forceOpaque_ && FragmentListRenderer::supportsFragmentLists();
    }

    virtual std::optional<mat4> boundingBox() const override;

    virtual Document getInfo() const override;

protected:
    /**
     * \brief Update the mesh drawer.
     * This is called when the inport is changed or when a property requires preprocessing steps on
     * the mesh, e.g. for silhouettes or special alpha features.
     */
    void updateMeshes();

    MeshFlatMultiInport inport_;

    SimpleLightingProperty lightingProperty_;

    BoolProperty forceOpaque_;

    BoolProperty drawSilhouette_;
    FloatVec4Property silhouetteColor_;

    enum class NormalSource : int { InputVertex, GenerateVertex, GenerateTriangle };
    OptionProperty<NormalSource> normalSource_;
    OptionProperty<meshutil::CalculateMeshNormalsMode> normalComputationMode_;

    /**
     * \brief Settings to assemble the equation for the alpha values.
     * All individual factors are clamped to [0,1].
     */
    struct AlphaSettings : public CompositeProperty {
        AlphaSettings();

        BoolProperty enableUniform_;
        FloatProperty uniformScaling_;
        FloatProperty minimumAlpha_;
        // IRIS
        BoolProperty enableAngleBased_;
        FloatProperty angleBasedExponent_;
        BoolProperty enableNormalVariation_;
        FloatProperty normalVariationExponent_;
        // Smoke surfaces
        BoolProperty enableDensity_;
        FloatProperty baseDensity_;  // k in the paper
        FloatProperty densityExponent_;
        BoolProperty enableShape_;
        FloatProperty shapeExponent_;  // s in the paper

        void setUniforms(Shader& shader, std::string_view prefix) const;
    };
    AlphaSettings alphaSettings_;

    /**
     * \brief Settings controlling how edges are highlighted.
     */
    struct EdgeSettings : public CompositeProperty {
        EdgeSettings();
        FloatProperty edgeThickness_;
        BoolProperty depthDependent_;
        BoolProperty smoothEdges_;
    };
    EdgeSettings edgeSettings_;

    enum class ColorSource : int { VertexColor, TransferFunction, ExternalColor };
    enum class ShadingMode : int {
        Off,  // no light, no reflection, just diffuse
        Phong
    };
    enum class HatchingMode : char { U, V, UV };
    enum class HatchingBlendingMode : char { Multiplicative, Additive };
    /**
     * \brief Hatching settings. These are exactly the parameters from the IRIS-paper
     */
    struct HatchingSettings {
        HatchingSettings();
        BoolCompositeProperty hatching_;

        OptionProperty<HatchingMode> mode_;
        IntProperty steepness_;
        IntProperty baseFrequencyU_;
        IntProperty baseFrequencyV_;

        BoolCompositeProperty modulation_;
        OptionProperty<HatchingMode> modulationMode_;
        FloatProperty modulationAnisotropy_;
        FloatProperty modulationOffset_;
        FloatVec3Property color_;
        FloatProperty strength_;
        OptionProperty<HatchingBlendingMode> blendingMode_;
    };
    /**
     * \brief The render settings per face.
     * faceSettings_[0]=front face, faceSettings_[1]=back face
     */
    struct FaceSettings {
        FaceSettings(bool frontFace);

        bool frontFace_;
        BoolCompositeProperty show_;
        BoolProperty sameAsFrontFace_;
        ButtonProperty copyFrontToBack_;

        TransferFunctionProperty transferFunction_;
        FloatVec3Property externalColor_;
        OptionProperty<ColorSource> colorSource_;

        BoolProperty separateUniformAlpha_;
        FloatProperty uniformAlpha_;
        OptionProperty<ShadingMode> shadingMode_;

        BoolProperty showEdges_;
        FloatVec3Property edgeColor_;
        FloatProperty edgeOpacity_;

        HatchingSettings hatching_;

        // to copy front to back:
        FaceSettings* frontPart_;

        void copyFrontToBack();

        void setUniforms(Shader& shader, std::string_view prefix) const;

        bool lastOpaque_;
    };

    std::array<FaceSettings, 2> faceSettings_;

    std::vector<std::shared_ptr<const Mesh>> enhancedMeshes_;
    std::shared_ptr<Shader> shader_;

    /**
     * \brief This flag is set to true if adjacency information is available in the shader.
     */
    bool meshHasAdjacency_;
    bool supportsFragmentLists_;
};

}  // namespace inviwo
