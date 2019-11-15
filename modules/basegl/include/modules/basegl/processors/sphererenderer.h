/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/interaction/cameratrackball.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/simplelightingproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <modules/opengl/shader/shader.h>
#include <modules/basegl/datastructures/meshshadercache.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>

#include <inviwo/core/util/zip.h>
#include <unordered_map>

namespace inviwo {

class IVW_MODULE_BASEGL_API SphereRendererSelection {
public:
    SphereRendererSelection(Inport& inport, BrushingAndLinkingInport& brushLinkPort)
        : properties("selection", "Show Selection", true)
        , color("selectionColor", "Color", vec4(1.0f, 0.769f, 0.247f, 1), vec4(0.0f), vec4(1.0f),
                vec4(0.01f), InvalidationLevel::InvalidOutput, PropertySemantics::Color)
        , radiusFactor("selectionRadiusFactor", "Radius Scaling", 1.5f, 0.0f, 10.0f)
        , inport_{inport}
        , brushLinkPort_{brushLinkPort} {

        properties.addProperties(color, radiusFactor);

        inport_.onDisconnect([&]() {
            util::map_erase_remove_if(
                selectionIndices_, [&](auto elem) { return !inport_.isConnectedTo(elem.first); });
        });
    }

    std::vector<uint32_t>* getIndices(const Outport* port, const Mesh& mesh) {
        if (properties.isChecked()) {
            std::vector<uint32_t>& indices = selectionIndices_[port];

            if (properties.isModified() || brushLinkPort_.isChanged() ||
                util::contains(inport_.getChangedOutports(), port)) {
                const auto& selection = brushLinkPort_.getSelectedIndices();

                indices.clear();
                if (auto res = mesh.findBuffer(BufferType::IndexAttrib);
                    res.first &&
                    res.first->getDataFormat()->getId() == DataFormat<uint32_t>::id()) {

                    const auto& indexBuffer =
                        static_cast<const BufferRAMPrecision<uint32_t, BufferTarget::Data>*>(
                            res.first->getRepresentation<BufferRAM>())
                            ->getDataContainer();

                    const auto seq = util::make_sequence(
                        uint32_t{0}, static_cast<uint32_t>(indexBuffer.size()), uint32_t{1});
                    std::copy_if(seq.begin(), seq.end(), std::back_inserter(indices),
                                 [&](uint32_t i) { return selection.count(indexBuffer[i]) > 0; });

                } else {
                    std::transform(selection.begin(), selection.end(), std::back_inserter(indices),
                                   [](size_t i) { return static_cast<uint32_t>(i); });
                }
            }
            if (!indices.empty()) {
                return &indices;
            }
        }
        return nullptr;
    }

    BoolCompositeProperty properties;
    FloatVec4Property color;
    FloatProperty radiusFactor;

private:
    Inport& inport_;
    BrushingAndLinkingInport& brushLinkPort_;
    std::unordered_map<const Outport*, std::vector<uint32_t>> selectionIndices_;
};

/** \docpage{org.inviwo.SphereRenderer, Sphere Renderer}
 * ![](org.inviwo.SphereRenderer.png?classIdentifier=org.inviwo.SphereRenderer)
 * This processor renders a set of point meshes using spherical glyphs in OpenGL.
 * The glyphs are resolution independent and consist only of a single point.
 *
 * ### Inports
 *   * __geometry__ Input meshes
 *       The input mesh uses the following buffers:
 * PositionAttrib vec3
 * ColorAttrib    vec4   (optional will fall-back to use __Custom Color__)
 * RadiiAttrib    float  (optional will fall-back to use __Custom Radius__)
 * PickingAttrib  uint32 (optional will fall-back to not draw any picking)
 *   * __imageInport__ Optional background image
 *
 * ### Outports
 *   * __image__    output image containing the rendered spheres and the optional input image
 *
 * ### Properties
 *   * __Render Mode__               render only input meshes marked as points or everything
 *   * __Clip Mode__                 defines the handling of spheres clipped at the camera
 *   * __Clip Surface Adjustment__   brighten/darken glyph color on clip surface
 *   * __Shaded Clipped Area__       enable illumination computations for the clipped surface
 *   * __Force Radius__              enable a fixed user-defined radius for all spheres
 *   * __Default Radius__            radius of the rendered spheres (in world coordinates)
 *   * __Force Color__               if enabled, all spheres will share the same custom color
 *   * __Default Color__             custom color when overwriting the input colors
 */

/**
 * \class SphereRenderer
 * \brief Renders input geometry with 3D sphere glyphs using OpenGL shaders
 */
class IVW_MODULE_BASEGL_API SphereRenderer : public Processor {
public:
    SphereRenderer();
    virtual ~SphereRenderer() = default;
    SphereRenderer(const SphereRenderer&) = delete;
    SphereRenderer(SphereRenderer&&) = delete;
    SphereRenderer& operator=(const SphereRenderer&) = delete;
    SphereRenderer& operator=(SphereRenderer&&) = delete;

    virtual void process() override;

    virtual void initializeResources() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    void configureShader(Shader& shader);

    enum class RenderMode {
        EntireMesh,  //!< render all vertices of the input mesh as glyphs
        PointsOnly,  //!< render only parts of mesh with DrawType::Points
    };
    /**
     * \enum GlyphClippingMode
     * defines how glyphs are rendering if the first intersection, i.e. the front side,
     * lies behind the near clip plane of the camera.
     */
    enum class GlyphClippingMode {
        Discard,  //!< glyph is not rendered
        Cut,      //!< the cut surface is visible
    };

    MeshFlatMultiInport inport_;
    ImageInport imageInport_;
    BrushingAndLinkingInport brushLinkPort_;
    ImageOutport outport_;

    TemplateOptionProperty<RenderMode> renderMode_;

    CompositeProperty clipping_;
    TemplateOptionProperty<GlyphClippingMode> clipMode_;
    FloatProperty clipShadingFactor_;  //!< multiplied with glyph color for clip surfaces
    BoolProperty shadeClippedArea_;

    CompositeProperty sphereProperties_;
    BoolProperty forceRadius_;
    FloatProperty defaultRadius_;
    BoolProperty forceColor_;
    FloatVec4Property defaultColor_;
    BoolProperty useMetaColor_;
    TransferFunctionProperty metaColor_;

    SphereRendererSelection selection_;

    CameraProperty camera_;
    CameraTrackball trackball_;
    SimpleLightingProperty lighting_;

    MeshShaderCache shaders_;
};

}  // namespace inviwo
