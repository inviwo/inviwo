/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2025 Inviwo Foundation
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

#include <modules/base/processors/buffertomeshprocessor.h>

#include <inviwo/core/algorithm/markdown.h>                    // for operator""_help, operator"...
#include <inviwo/core/datastructures/buffer/buffer.h>          // for IndexBuffer
#include <inviwo/core/datastructures/geometry/geometrytype.h>  // for ConnectivityType, DrawType
#include <inviwo/core/datastructures/geometry/mesh.h>          // for Mesh, Mesh::MeshInfo
#include <inviwo/core/ports/bufferport.h>                      // for BufferInport
#include <inviwo/core/ports/meshport.h>                        // for MeshOutport
#include <inviwo/core/processors/processor.h>                  // for Processor
#include <inviwo/core/processors/processorinfo.h>              // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>             // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>              // for Tags, Tags::CPU
#include <inviwo/core/properties/optionproperty.h>             // for OptionPropertyOption, Opti...
#include <inviwo/core/util/exception.h>                        // for Exception
#include <inviwo/core/util/sourcecontext.h>                    // for IVW_CONTEXT
#include <inviwo/core/util/staticstring.h>                     // for operator+

#include <memory>       // for const_pointer_cast, shared...
#include <type_traits>  // for remove_extent_t

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo BufferToMeshProcessor::processorInfo_{
    "org.inviwo.BufferToMeshProcessor",  // Class identifier
    "Buffer to mesh",                    // Display name
    "Mesh Creation",                     // Category
    CodeState::Stable,                   // Code state
    Tags::CPU,                           // Tags
    R"(Takes input buffers and puts them into a mesh without copying data. All buffers must be of
    same size except the index buffer. Unspecified draw type will render the mesh as points by
    default. See https://www.khronos.org/opengl/wiki/Primitive for more information about draw
    types and connectivity.)"_unindentHelp};

const ProcessorInfo& BufferToMeshProcessor::getProcessorInfo() const { return processorInfo_; }

BufferToMeshProcessor::BufferToMeshProcessor()
    : Processor()
    , vertices_("Vertices", "Mesh positions of type vec3 or vec2"_help)
    , indices_("Indices", "Vertex connectivity, integer type (optional)"_help)
    , vertexColors_("VertexColors",
                    "Color at each vertex, usually vec4 or uvec4 type (optional)"_help)
    , textureCoordinates_("TextureCoordinates",
                          "Texture coordinates at each vertex, usually vec2 type (optional)"_help)
    , normals_("Normals", "Vertex normals, of vec3 type  (optional)"_help)
    , curvature_("Curvature", "Vertex curvature, usually float type  (optional)"_help)
    , outport_("Mesh", "Mesh with all specified buffers"_help)
    , drawType_("drawType", "Draw type", "Default way to draw the mesh"_help,
                {{"Unspecified", "Unspecified", DrawType::NotSpecified},
                 {"Points", "Points", DrawType::Points},
                 {"Lines", "Lines", DrawType::Lines},
                 {"Triangles", "Triangles", DrawType::Triangles}},
                0)
    , connectivity_("connectivity", "Connectivity",
                    "Describes how the indices connect vertices together"_help,
                    {{"None", "None", ConnectivityType::None},
                     {"Strip", "Strip", ConnectivityType::Strip},
                     {"Loop", "Loop", ConnectivityType::Loop},
                     {"Fan", "Fan", ConnectivityType::Fan},
                     {"Adjacency", "Adjacency", ConnectivityType::Adjacency},
                     {"StripAdjacency", "StripAdjacency", ConnectivityType::StripAdjacency}},
                    0) {

    indices_.setOptional(true);  // Not needed when rendering for example points
    textureCoordinates_.setOptional(true);
    vertexColors_.setOptional(true);
    normals_.setOptional(true);
    curvature_.setOptional(true);

    addPort(vertices_);
    addPort(indices_);
    addPort(vertexColors_);
    addPort(textureCoordinates_);
    addPort(normals_);
    addPort(curvature_);
    addPort(outport_);

    addProperty(drawType_);
    addProperty(connectivity_);
}

void BufferToMeshProcessor::process() {
    auto mesh = std::make_shared<Mesh>(drawType_.get(), connectivity_.get());

    // Note on const_cast of input buffers when adding them to mesh:
    // The buffers are not modified and the mesh will be const in all connected ports,
    // making it ok to use const_cast in this particular case.
    // Be aware that, in general, you should not use const cast on inport data!
    mesh->addBuffer(BufferType::PositionAttrib,
                    std::const_pointer_cast<BufferBase>(vertices_.getData()));
    if (textureCoordinates_.isConnected()) {
        if (textureCoordinates_.getData()->getSize() != vertices_.getData()->getSize()) {
            throw Exception("Number of texture coordinates does not match number of vertices",
                            IVW_CONTEXT);
        }
        mesh->addBuffer(BufferType::TexCoordAttrib,
                        std::const_pointer_cast<BufferBase>(textureCoordinates_.getData()));
    }
    if (vertexColors_.isConnected()) {
        if (vertexColors_.getData()->getSize() != vertices_.getData()->getSize()) {
            throw Exception("Number of colors does not match number of vertices", IVW_CONTEXT);
        }
        mesh->addBuffer(BufferType::ColorAttrib,
                        std::const_pointer_cast<BufferBase>(vertexColors_.getData()));
    }
    if (normals_.isConnected()) {
        if (normals_.getData()->getSize() != vertices_.getData()->getSize()) {
            throw Exception("Number of normals does not match number of vertices", IVW_CONTEXT);
        }
        mesh->addBuffer(BufferType::NormalAttrib,
                        std::const_pointer_cast<BufferBase>(normals_.getData()));
    }
    if (curvature_.isConnected()) {
        if (curvature_.getData()->getSize() != vertices_.getData()->getSize()) {
            throw Exception("Number of curvatures does not match number of vertices", IVW_CONTEXT);
        }
        mesh->addBuffer(BufferType::CurvatureAttrib,
                        std::const_pointer_cast<BufferBase>(curvature_.getData()));
    }

    if (indices_.isConnected()) {
        auto indexBuffer = std::dynamic_pointer_cast<const IndexBuffer>(indices_.getData());
        if (!indexBuffer) {
            throw Exception(
                "Index buffer must be of IndexBuffer type (Buffer<std::uint32_t, "
                "BufferTarget::Index>)",
                IVW_CONTEXT);
        }
        mesh->addIndices(Mesh::MeshInfo(drawType_.get(), connectivity_.get()),
                         std::const_pointer_cast<IndexBuffer>(indexBuffer));
    }

    outport_.setData(mesh);
}

}  // namespace inviwo
