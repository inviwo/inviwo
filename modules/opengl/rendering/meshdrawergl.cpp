/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include <modules/opengl/buffer/elementbuffergl.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/openglutils.h>

namespace inviwo {

MeshDrawerGL::MeshDrawerGL() : meshToDraw_(nullptr) {}

MeshDrawerGL::MeshDrawerGL(const Mesh* mesh) : meshToDraw_(mesh) {
    initialize(mesh->getDefaultMeshInfo());
}

MeshDrawerGL::MeshDrawerGL(const Mesh* mesh, Mesh::MeshInfo ai) : meshToDraw_(mesh) {
    initialize(ai);
}

MeshDrawerGL::MeshDrawerGL(const Mesh* mesh, DrawType dt, ConnectivityType ct) : meshToDraw_(mesh) {
    initialize(Mesh::MeshInfo(dt, ct));
}

MeshDrawerGL::MeshDrawerGL(MeshDrawerGL&& other)
    : MeshDrawer(std::move(other)), meshToDraw_{other.meshToDraw_} {
    // move each element
    std::move(std::begin(other.drawMethods_), std::end(other.drawMethods_), &drawMethods_[0]);
    other.meshToDraw_ = nullptr;
}

MeshDrawerGL::~MeshDrawerGL(){}

MeshDrawerGL& MeshDrawerGL::operator=(const MeshDrawerGL& rhs) {
    if (this != &rhs) {
        meshToDraw_ = rhs.meshToDraw_;
        std::copy(std::begin(rhs.drawMethods_), std::end(rhs.drawMethods_), &drawMethods_[0]);
    }
    return *this;
}

void MeshDrawerGL::draw() {
    auto meshGL = meshToDraw_->getRepresentation<MeshGL>();
    utilgl::Enable<MeshGL> enable(meshGL);
    
    // If default is indices, render all index lists
    if (!drawMethods_[0].elementBufferList.empty()) {
        for (int i = 1; i < static_cast<int>(DrawMode::NumberOfDrawModes); i++) {
            if (!drawMethods_[i].elementBufferList.empty())
                (this->*drawMethods_[i].drawFunc)(static_cast<DrawMode>(i));
        }
    } else {// Render just default one
        (this->*drawMethods_[0].drawFunc)(DrawMode::NotSpecified);
    }
}

void MeshDrawerGL::draw(DrawMode dm) {
    auto meshGL = meshToDraw_->getRepresentation<MeshGL>();
    utilgl::Enable<MeshGL> enable(meshGL);   
    (this->*drawMethods_[static_cast<size_t>(dm)].drawFunc)(dm);
}

GLenum MeshDrawerGL::getDefaultDrawMode() { return drawMethods_[0].drawMode; }

void MeshDrawerGL::drawArray(DrawMode dmenum) const {
    const auto dm = static_cast<size_t>(dmenum);
    glDrawArrays(drawMethods_[dm].drawMode, 0,
                 static_cast<GLsizei>(meshToDraw_->getBuffer(0)->getSize()));
}

void MeshDrawerGL::drawElements(DrawMode dmenum) const {
    const auto dm = static_cast<size_t>(dmenum);
    for (auto elem : drawMethods_[dm].elementBufferList) {
        auto elementBufferGL = elem->getRepresentation<ElementBufferGL>();
        elementBufferGL->bind();
        glDrawElements(drawMethods_[dm].drawMode, static_cast<GLsizei>(elementBufferGL->getSize()),
                       elementBufferGL->getFormatType(), nullptr);
    }
}

void MeshDrawerGL::initialize(Mesh::MeshInfo ai) {
    const auto dm = static_cast<size_t>(getDrawMode(ai.dt, ai.ct));
    const auto ns = static_cast<size_t>(DrawMode::NotSpecified);
    
    drawMethods_[ns].drawFunc = &MeshDrawerGL::emptyFunc;
    drawMethods_[ns].drawMode = getGLDrawMode(getDrawMode(ai.dt, ai.ct));
    drawMethods_[ns].elementBufferList.clear();

    for (size_t i = 1; i < static_cast<size_t>(DrawType::NumberOfDrawTypes); i++) {
        drawMethods_[i].drawFunc = drawMethods_[ns].drawFunc;
        drawMethods_[i].drawMode = drawMethods_[ns].drawMode;
        drawMethods_[i].elementBufferList.clear();
    }

    drawMethods_[dm].drawFunc = &MeshDrawerGL::drawArray;
    drawMethods_[ns].drawFunc = &MeshDrawerGL::drawArray;

    for (size_t i = 0; i < meshToDraw_->getNumberOfIndicies(); ++i) {
        if (meshToDraw_->getIndicies(i)->getSize() > 0)
            initializeIndexBuffer(meshToDraw_->getIndicies(i),
                                  meshToDraw_->getIndexMeshInfo(i));
    }
}

void MeshDrawerGL::initializeIndexBuffer(const BufferBase* indexBuffer, Mesh::MeshInfo ai) {
    const auto dm = static_cast<int>(getDrawMode(ai.dt, ai.ct));
    // check draw mode if there exists another indexBuffer
    if (drawMethods_[dm].elementBufferList.size() == 0) {
        drawMethods_[dm].drawFunc = &MeshDrawerGL::drawElements;
        drawMethods_[dm].drawMode = getGLDrawMode(getDrawMode(ai.dt, ai.ct));
    } 
    drawMethods_[dm].elementBufferList.push_back(indexBuffer);

    // Specify first element buffer as default rendering method
    const auto ns = static_cast<int>(DrawType::NotSpecified);
    if (drawMethods_[ns].elementBufferList.size() == 0) {
        drawMethods_[ns].drawFunc = drawMethods_[dm].drawFunc;
        drawMethods_[ns].drawMode = drawMethods_[dm].drawMode;
        drawMethods_[ns].elementBufferList.push_back(drawMethods_[dm].elementBufferList.at(0));
    }
}

MeshDrawerGL::DrawMode MeshDrawerGL::getDrawMode(DrawType dt, ConnectivityType ct) const {
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
                case ConnectivityType::NumberOfConnectivityTypes:
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
                case ConnectivityType::NumberOfConnectivityTypes:
                default:
                    return DrawMode::Points;
            }

        case DrawType::Points:
        case DrawType::NotSpecified:
        case DrawType::NumberOfDrawTypes:
        default:
            return DrawMode::Points;
    }
}

GLenum MeshDrawerGL::getGLDrawMode(DrawMode dm) const {
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

}  // namespace
