/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2022 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>  // for IVW_MODULE_BASEGL...

#include <inviwo/core/datastructures/bitset.h>                          // for BitSet
#include <inviwo/core/datastructures/buffer/buffer.h>                   // for BufferBase
#include <inviwo/core/datastructures/buffer/bufferram.h>                // for BufferRAMPrecision
#include <inviwo/core/datastructures/geometry/geometrytype.h>           // for BufferTarget, Buf...
#include <inviwo/core/datastructures/geometry/mesh.h>                   // for Mesh
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/interaction/cameratrackball.h>                    // for CameraTrackball
#include <inviwo/core/ports/imageport.h>                                // for ImageInport, Imag...
#include <inviwo/core/ports/inport.h>                                   // for Inport
#include <inviwo/core/ports/meshport.h>                                 // for MeshFlatMultiInport
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/properties/boolcompositeproperty.h>               // for BoolCompositeProp...
#include <inviwo/core/properties/boolproperty.h>                        // for BoolProperty
#include <inviwo/core/properties/cameraproperty.h>                      // for CameraProperty
#include <inviwo/core/properties/compositeproperty.h>                   // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>                   // for InvalidationLevel
#include <inviwo/core/properties/optionproperty.h>                      // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>                     // for FloatProperty
#include <inviwo/core/properties/propertysemantics.h>                   // for PropertySemantics
#include <inviwo/core/properties/simplelightingproperty.h>              // for SimpleLightingPro...
#include <inviwo/core/properties/transferfunctionproperty.h>            // for TransferFunctionP...
#include <inviwo/core/properties/selectioncolorproperty.h>
#include <inviwo/core/util/formats.h>                                   // for DataFormat, DataF...
#include <inviwo/core/util/glmvec.h>                                    // for vec4
#include <inviwo/core/util/staticstring.h>                              // for operator+
#include <inviwo/core/util/stdextensions.h>                             // for contains, map_era...
#include <inviwo/core/util/zip.h>                                       // for make_sequence
#include <modules/basegl/datastructures/meshshadercache.h>              // for MeshShaderCache
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>   // for BrushingAndLinkin...
#include <modules/opengl/texture/buffertexture.h>
#include <modules/opengl/texture/texture2d.h>

#include <modules/fontrendering/textrenderer.h>
#include <modules/fontrendering/properties/fontfaceoptionproperty.h>

#include <algorithm>      // for copy_if, transform
#include <cstddef>        // for size_t
#include <cstdint>        // for uint32_t
#include <functional>     // for __base
#include <iterator>       // for back_insert_iterator
#include <memory>         // for unique_ptr
#include <string>         // for operator==, string
#include <string_view>    // for operator==, strin...
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set
#include <utility>        // for pair
#include <vector>         // for vector, operator!=

namespace inviwo {
class Outport;
class Shader;

/**
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
    ImageInport sphereTexture_;
    BrushingAndLinkingInport brushLinkPort_;
    ImageOutport outport_;

    OptionProperty<RenderMode> renderMode_;

    CompositeProperty clipping_;
    OptionProperty<GlyphClippingMode> clipMode_;
    FloatProperty clipShadingFactor_;  //!< multiplied with glyph color for clip surfaces
    BoolProperty shadeClippedArea_;

    CompositeProperty sphereProperties_;
    BoolProperty forceRadius_;
    FloatProperty defaultRadius_;
    BoolProperty forceColor_;
    FloatVec4Property defaultColor_;
    BoolProperty useMetaColor_;
    TransferFunctionProperty metaColor_;

    BoolCompositeProperty showLabels_;
    FontFaceOptionProperty labelFont_;
    IntProperty labelFontSize_;
    FloatVec4Property labelColor_;
    FloatProperty labelSize_;
    float labelAspect_;

    SelectionColorProperty showHighlighted_;
    SelectionColorProperty showSelected_;
    SelectionColorProperty showFiltered_;

    CameraProperty camera_;
    CameraTrackball trackball_;
    SimpleLightingProperty lighting_;
    
    BoolCompositeProperty periodicity_;
    FloatMat4Property basis_;
    FloatVec3Property shift_;
    IntVec3Property repeat_;
    FloatProperty duplicateCutoff_;

    MeshShaderCache shaders_;
    BufferTexture<std::uint8_t, GL_R8UI> bnlBuffer;

    std::shared_ptr<Texture2D> atlas_;
    TextRenderer textRenderer_;

};

}  // namespace inviwo
