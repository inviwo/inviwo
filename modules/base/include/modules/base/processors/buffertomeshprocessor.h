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

#ifndef IVW_BUFFERTOMESHPROCESSOR_H
#define IVW_BUFFERTOMESHPROCESSOR_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/ports/bufferport.h>
#include <inviwo/core/ports/meshport.h>

namespace inviwo {

/** \docpage{org.inviwo.BufferToMeshProcessor, Buffer To Mesh Processor}
 * ![](org.inviwo.BufferToMeshProcessor.png?classIdentifier=org.inviwo.BufferToMeshProcessor)
 * Takes input buffers and puts them into a mesh without copying data.
 * All buffers must be of same size except the index buffer.
 * Unspecified draw type will render the mesh as points by default.
 * See https://www.khronos.org/opengl/wiki/Primitive for more information about draw types and
 * connectivity.
 *
 * ### Inports
 *   * __Vertices__ Mesh positions of type vec3 or vec2.
 *   * __Indices__ Vertex connectivity, integer type (optional).
 *   * __VertexColors__ Color at each vertex, usually vec4 or uvec4 type (optional).
 *   * __TextureCoordinates__ Texture coordinates at each vertex, usually vec2 type (optional).
 *   * __Normals__ Vertex normals, of vec3 type  (optional).
 *   * __Curvature__ Vertex curvature, usually float type  (optional).
 *
 * ### Outports
 *   * __Mesh__ Mesh with all specified buffers.
 *
 * ### Properties
 *   * __DrawType__ Default way to draw the mesh.
 *   * __Connectivity__ Describes how the indices connect vertices together.
 */

/**
 * \class BufferToMeshProcessor
 * \brief Put buffers into mesh without copying data.
 * Supported buffer types are specified in BufferType
 */
class IVW_MODULE_BASE_API BufferToMeshProcessor : public Processor {
public:
    BufferToMeshProcessor();
    virtual ~BufferToMeshProcessor() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    // See BufferType for supported types
    BufferInport vertices_;
    BufferInport indices_;             // Optional
    BufferInport vertexColors_;        // Optional
    BufferInport textureCoordinates_;  // Optional
    BufferInport normals_;             // Optional
    BufferInport curvature_;           // Optional
    MeshOutport outport_;

    TemplateOptionProperty<DrawType> drawType_;
    TemplateOptionProperty<ConnectivityType> connectivity_;
};

}  // namespace inviwo

#endif  // IVW_BUFFERTOMESHPROCESSOR_H
