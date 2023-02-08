/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2022 Inviwo Foundation
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

#include <modules/opengl/rendering/meshdrawergl.h>

#include <inviwo/core/datastructures/geometry/geometrytype.h>  // for ConnectivityType, DrawType
#include <inviwo/core/datastructures/geometry/mesh.h>          // for Mesh, Mesh::MeshInfo
#include <inviwo/core/util/exception.h>                        // for RangeException, NullPointe...
#include <inviwo/core/util/sourcecontext.h>                    // for IVW_CONTEXT
#include <modules/opengl/buffer/buffergl.h>                    // for BufferGL
#include <modules/opengl/geometry/meshgl.h>                    // for MeshGL
#include <modules/opengl/openglutils.h>                        // for Enable

#include <string>       // for allocator, operator+, to_s...
#include <string_view>  // for string_view

namespace inviwo {

MeshDrawerGL::MeshDrawerGL() : meshToDraw_(nullptr) {}

MeshDrawerGL::MeshDrawerGL(const Mesh* mesh) : meshToDraw_(mesh) {
    if (mesh == nullptr) throw NullPointerException("input mesh is null", IVW_CONTEXT);
}

MeshDrawerGL::DrawObject MeshDrawerGL::getDrawObject() const {
    return DrawObject(meshToDraw_->getRepresentation<MeshGL>(), meshToDraw_->getDefaultMeshInfo());
}

MeshDrawerGL::DrawObject MeshDrawerGL::getDrawObject(const Mesh* mesh) {
    return DrawObject(mesh->getRepresentation<MeshGL>(), mesh->getDefaultMeshInfo());
}

void MeshDrawerGL::draw() {
    if (meshToDraw_->getNumberOfBuffers() == 0) {
        // empty mesh, do nothing
        return;
    }

    auto meshGL = meshToDraw_->getRepresentation<MeshGL>();
    utilgl::Enable<MeshGL> enable(meshGL);

    std::size_t numIndexBuffers = meshGL->getIndexBufferCount();
    if (numIndexBuffers > 0) {
        // draw mesh using the index buffers
        for (std::size_t i = 0; i < numIndexBuffers; ++i) {
            auto indexBuffer = meshGL->getIndexBuffer(i);
            auto numIndices = indexBuffer->getSize();
            if (numIndices > 0) {
                indexBuffer->bind();
                auto drawMode = getGLDrawMode(meshGL->getMeshInfoForIndexBuffer(i));
                glDrawElements(drawMode, static_cast<GLsizei>(numIndices),
                               indexBuffer->getFormatType(), nullptr);
            }
        }
    } else {
        // the mesh does not contain index buffers, render all vertices
        auto drawMode = getGLDrawMode(meshToDraw_->getDefaultMeshInfo());
        glDrawArrays(drawMode, 0, static_cast<GLsizei>(meshGL->getBufferGL(0)->getSize()));
    }
}

void MeshDrawerGL::draw(DrawMode drawMode) {
    auto meshGL = meshToDraw_->getRepresentation<MeshGL>();
    utilgl::Enable<MeshGL> enable(meshGL);

    auto drawModeGL = getGLDrawMode(drawMode);

    std::size_t numIndexBuffers = meshGL->getIndexBufferCount();
    if (numIndexBuffers > 0) {
        // draw mesh using the index buffers
        for (std::size_t i = 0; i < numIndexBuffers; ++i) {
            auto indexBuffer = meshGL->getIndexBuffer(i);
            auto numIndices = indexBuffer->getSize();
            if (numIndices > 0) {
                indexBuffer->bind();
                glDrawElements(drawModeGL, static_cast<GLsizei>(numIndices),
                               indexBuffer->getFormatType(), nullptr);
            }
        }
    } else {
        // the mesh does not contain index buffers, render all vertices
        glDrawArrays(drawModeGL, 0, static_cast<GLsizei>(meshGL->getBufferGL(0)->getSize()));
    }
}

MeshDrawerGL::DrawMode MeshDrawerGL::getDrawMode(DrawType dt, ConnectivityType ct) {
    switch (dt) {
        case DrawType::Triangles:
            switch (ct) {
                case ConnectivityType::None:
                    return DrawMode::Triangles;

                case ConnectivityType::Strip:
                    return DrawMode::TriangleStrip;

                case ConnectivityType::Fan:
                    return DrawMode::TriangleFan;

                case ConnectivityType::Adjacency:
                    return DrawMode::TrianglesAdjacency;

                case ConnectivityType::StripAdjacency:
                    return DrawMode::TriangleStripAdjacency;

                case ConnectivityType::Loop:
                default:
                    return DrawMode::Points;
            }

        case DrawType::Lines:
            switch (ct) {
                case ConnectivityType::None:
                    return DrawMode::Lines;

                case ConnectivityType::Strip:
                    return DrawMode::LineStrip;

                case ConnectivityType::Loop:
                    return DrawMode::LineLoop;

                case ConnectivityType::Adjacency:
                    return DrawMode::LinesAdjacency;

                case ConnectivityType::StripAdjacency:
                    return DrawMode::LineStripAdjacency;

                case ConnectivityType::Fan:
                default:
                    return DrawMode::Points;
            }

        case DrawType::Points:
        case DrawType::NotSpecified:
        default:
            return DrawMode::Points;
    }
}

GLenum MeshDrawerGL::getGLDrawMode(DrawMode dm) {
    switch (dm) {
        case DrawMode::Points:
            return GL_POINTS;
        case DrawMode::Lines:
            return GL_LINES;
        case DrawMode::LineStrip:
            return GL_LINE_STRIP;
        case DrawMode::LineLoop:
            return GL_LINE_LOOP;
        case DrawMode::LinesAdjacency:
            return GL_LINES_ADJACENCY;
        case DrawMode::LineStripAdjacency:
            return GL_LINE_STRIP_ADJACENCY;
        case DrawMode::Triangles:
            return GL_TRIANGLES;
        case DrawMode::TriangleStrip:
            return GL_TRIANGLE_STRIP;
        case DrawMode::TriangleFan:
            return GL_TRIANGLE_FAN;
        case DrawMode::TrianglesAdjacency:
            return GL_TRIANGLES_ADJACENCY;
        case DrawMode::TriangleStripAdjacency:
            return GL_TRIANGLE_STRIP_ADJACENCY;
        case DrawMode::NumberOfDrawModes:
        case DrawMode::NotSpecified:
        default:
            return GL_POINTS;
    }
}

GLenum MeshDrawerGL::getGLDrawMode(Mesh::MeshInfo meshInfo) {
    return getGLDrawMode(getDrawMode(meshInfo.dt, meshInfo.ct));
}
MeshDrawerGL::DrawObject::DrawObject(const Mesh& mesh)
    : DrawObject(mesh.getRepresentation<MeshGL>(), mesh.getDefaultMeshInfo()) {}

MeshDrawerGL::DrawObject::DrawObject(const MeshGL* mesh, Mesh::MeshInfo arrayMeshInfo)
    : enable_(mesh), meshGL_(mesh), arrayMeshInfo_(arrayMeshInfo) {}

namespace {

void drawElements(const BufferGL& indexBuffer, GLenum drawModeGL) {
    const auto numIndices = indexBuffer.getSize();
    if (numIndices > 0) {
        indexBuffer.bind();
        glDrawElements(drawModeGL, static_cast<GLsizei>(numIndices), indexBuffer.getFormatType(),
                       nullptr);
    }
}
void drawElements(const BufferGL& indexBuffer, GLenum drawModeGL, size_t instances) {
    const auto numIndices = indexBuffer.getSize();
    if (numIndices > 0) {
        indexBuffer.bind();
        glDrawElementsInstanced(drawModeGL, static_cast<GLsizei>(numIndices),
                                indexBuffer.getFormatType(), nullptr,
                                static_cast<GLsizei>(instances));
    }
}

template <typename GetDrawMode>
void drawAll(const MeshGL& meshGL, const GetDrawMode& modeForIB, GLenum drawModeGL) {
    const std::size_t numIndexBuffers = meshGL.getIndexBufferCount();
    if (numIndexBuffers > 0) {
        // draw mesh using the index buffers
        for (std::size_t i = 0; i < numIndexBuffers; ++i) {
            drawElements(*meshGL.getIndexBuffer(i), modeForIB(i));
        }
    } else if (!meshGL.empty()) {
        // the mesh does not contain index buffers, render all vertices
        glDrawArrays(drawModeGL, 0, static_cast<GLsizei>(meshGL.getBufferGL(0)->getSize()));
    }
}
template <typename GetDrawMode>
void drawAll(const MeshGL& meshGL, const GetDrawMode& modeForIB, GLenum drawModeGL,
             size_t instances) {
    const std::size_t numIndexBuffers = meshGL.getIndexBufferCount();
    if (numIndexBuffers > 0) {
        // draw mesh using the index buffers
        for (std::size_t i = 0; i < numIndexBuffers; ++i) {
            drawElements(*meshGL.getIndexBuffer(i), modeForIB(i), instances);
        }
    } else if (!meshGL.empty()) {
        // the mesh does not contain index buffers, render all vertices
        glDrawArraysInstanced(drawModeGL, 0, static_cast<GLsizei>(meshGL.getBufferGL(0)->getSize()),
                              static_cast<GLsizei>(instances));
    }
}

}  // namespace

void MeshDrawerGL::DrawObject::checkIndex(size_t index) const {
    const std::size_t numIndexBuffers = meshGL_->getIndexBufferCount();
    if (index >= numIndexBuffers) {
        throw RangeException(IVW_CONTEXT, "Index ({}) for indexbuffer of size {} is out-of-range",
                             index, numIndexBuffers);
    }
}

void MeshDrawerGL::DrawObject::draw() {
    drawAll(
        *meshGL_, [this](size_t i) { return getGLDrawMode(meshGL_->getMeshInfoForIndexBuffer(i)); },
        getGLDrawMode(arrayMeshInfo_));
}

void MeshDrawerGL::DrawObject::draw(DrawMode drawMode) {
    drawAll(
        *meshGL_, [dm = getGLDrawMode(drawMode)](size_t) { return dm; }, getGLDrawMode(drawMode));
}

void MeshDrawerGL::DrawObject::draw(std::size_t index) {
    checkIndex(index);
    drawElements(*meshGL_->getIndexBuffer(index),
                 getGLDrawMode(meshGL_->getMeshInfoForIndexBuffer(index)));
}

void MeshDrawerGL::DrawObject::draw(DrawMode drawMode, std::size_t index) {
    checkIndex(index);
    drawElements(*meshGL_->getIndexBuffer(index), getGLDrawMode(drawMode));
}

void MeshDrawerGL::DrawObject::drawInstanced(size_t instances) {
    drawAll(
        *meshGL_, [this](size_t i) { return getGLDrawMode(meshGL_->getMeshInfoForIndexBuffer(i)); },
        getGLDrawMode(arrayMeshInfo_), instances);
}
void MeshDrawerGL::DrawObject::drawInstanced(DrawMode drawMode, size_t instances) {
    drawAll(
        *meshGL_, [dm = getGLDrawMode(drawMode)](size_t) { return dm; }, getGLDrawMode(drawMode),
        instances);
}
void MeshDrawerGL::DrawObject::drawInstanced(std::size_t index, size_t instances) {
    checkIndex(index);
    drawElements(*meshGL_->getIndexBuffer(index),
                 getGLDrawMode(meshGL_->getMeshInfoForIndexBuffer(index)), instances);
}
void MeshDrawerGL::DrawObject::drawInstanced(DrawMode drawMode, std::size_t index,
                                             size_t instances) {
    checkIndex(index);
    drawElements(*meshGL_->getIndexBuffer(index), getGLDrawMode(drawMode), instances);
}

std::size_t MeshDrawerGL::DrawObject::size() const { return meshGL_->getIndexBufferCount(); }

}  // namespace inviwo
