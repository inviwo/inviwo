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
    initialize(mesh->getDefaultAttributesInfo());
}

MeshDrawerGL::MeshDrawerGL(const Mesh* mesh, Mesh::AttributesInfo ai) : meshToDraw_(mesh) {
    initialize(ai);
}

MeshDrawerGL::MeshDrawerGL(const Mesh* mesh, DrawType dt, ConnectivityType ct) : meshToDraw_(mesh) {
    initialize(Mesh::AttributesInfo(dt, ct));
}

MeshDrawerGL::MeshDrawerGL(MeshDrawerGL&& other)
    : MeshDrawer(std::move(other)), meshToDraw_{other.meshToDraw_} {
    // move each element
    std::move(std::begin(other.drawMethods_), std::end(other.drawMethods_), &drawMethods_[0]);
    other.meshToDraw_ = nullptr;
}

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
        for (int i = 1; i < static_cast<int>(DrawType::NUMBER_OF_DRAW_TYPES); i++) {
            if (!drawMethods_[i].elementBufferList.empty())
                (this->*drawMethods_[i].drawFunc)(static_cast<DrawType>(i));
        }
    } else {
        // Render just default one
        (this->*drawMethods_[0].drawFunc)(DrawType::NOT_SPECIFIED);
    }
}

void MeshDrawerGL::draw(DrawType dt) {
    auto meshGL = meshToDraw_->getRepresentation<MeshGL>();
    utilgl::Enable<MeshGL> enable(meshGL);
    
    (this->*drawMethods_[static_cast<size_t>(dt)].drawFunc)(dt);
}

GLenum MeshDrawerGL::getDefaultDrawMode() { return drawMethods_[0].drawMode; }

void MeshDrawerGL::drawArray(DrawType dtenum) const {
    const auto dt = static_cast<size_t>(dtenum);
    glDrawArrays(drawMethods_[dt].drawMode, 0,
                 static_cast<GLsizei>(meshToDraw_->getAttributes(0)->getSize()));
}

void MeshDrawerGL::drawElements(DrawType dtenum) const {
    const auto dt = static_cast<size_t>(dtenum);
    for (auto elem : drawMethods_[dt].elementBufferList) {
        auto elementBufferGL = elem->getRepresentation<ElementBufferGL>();
        elementBufferGL->bind();
        glDrawElements(drawMethods_[dt].drawMode, static_cast<GLsizei>(elementBufferGL->getSize()),
                       elementBufferGL->getFormatType(), nullptr);
    }
}

void MeshDrawerGL::initialize(Mesh::AttributesInfo ai) {
    const auto dt = static_cast<size_t>(ai.dt);
    const auto ns = static_cast<size_t>(DrawType::NOT_SPECIFIED);
    
    drawMethods_[ns].drawFunc = &MeshDrawerGL::emptyFunc;
    drawMethods_[ns].drawMode = getDrawMode(ai.dt, ai.ct);
    drawMethods_[ns].elementBufferList.clear();

    for (size_t i = 1; i < static_cast<size_t>(DrawType::NUMBER_OF_DRAW_TYPES); i++) {
        drawMethods_[i].drawFunc = drawMethods_[ns].drawFunc;
        drawMethods_[i].drawMode = drawMethods_[ns].drawMode;
        drawMethods_[i].elementBufferList.clear();
    }

    drawMethods_[dt].drawFunc = &MeshDrawerGL::drawArray;
    drawMethods_[ns].drawFunc = &MeshDrawerGL::drawArray;

    for (size_t i = 0; i < meshToDraw_->getNumberOfIndicies(); ++i) {
        if (meshToDraw_->getIndicies(i)->getSize() > 0)
            initializeIndexBuffer(meshToDraw_->getIndicies(i),
                                  meshToDraw_->getIndexAttributesInfo(i));
    }
}

void MeshDrawerGL::initializeIndexBuffer(const Buffer* indexBuffer, Mesh::AttributesInfo ai) {
    const auto dt = static_cast<int>(ai.dt);
    // check draw mode if there exists another indexBuffer
    if (drawMethods_[dt].elementBufferList.size() != 0) {
        if (getDrawMode(ai.dt, ai.ct) != drawMethods_[dt].drawMode) {
            LogWarn("draw mode mismatch (element buffer " << dt << ")");
        }
    } else {
        drawMethods_[dt].drawFunc = &MeshDrawerGL::drawElements;
        drawMethods_[dt].drawMode = getDrawMode(ai.dt, ai.ct);
    }
    drawMethods_[dt].elementBufferList.push_back(indexBuffer);

    // Specify first element buffer as default rendering method
    const auto ns = static_cast<int>(DrawType::NOT_SPECIFIED);
    if (drawMethods_[ns].elementBufferList.size() == 0) {
        drawMethods_[ns].drawFunc = drawMethods_[dt].drawFunc;
        drawMethods_[ns].drawMode = drawMethods_[dt].drawMode;
        drawMethods_[ns].elementBufferList.push_back(drawMethods_[dt].elementBufferList.at(0));
    }
}

GLenum MeshDrawerGL::getDrawMode(DrawType dt, ConnectivityType ct) {
    switch (dt) {
        case DrawType::TRIANGLES:
            switch (ct) {
                case ConnectivityType::NONE:
                    return GL_TRIANGLES;

                case ConnectivityType::STRIP:
                    return GL_TRIANGLE_STRIP;

                case ConnectivityType::FAN:
                    return GL_TRIANGLE_FAN;

                case ConnectivityType::ADJACENCY:
                    return GL_TRIANGLES_ADJACENCY;

                case ConnectivityType::STRIP_ADJACENCY:
                    return GL_TRIANGLE_STRIP_ADJACENCY;

                case ConnectivityType::LOOP:
                case ConnectivityType::NUMBER_OF_CONNECTIVITY_TYPES:
                default:
                    return GL_POINTS;
            }

        case DrawType::LINES:
            switch (ct) {
                case ConnectivityType::NONE:
                    return GL_LINES;

                case ConnectivityType::STRIP:
                    return GL_LINE_STRIP;

                case ConnectivityType::LOOP:
                    return GL_LINE_LOOP;

                case ConnectivityType::ADJACENCY:
                    return GL_LINES_ADJACENCY;

                case ConnectivityType::STRIP_ADJACENCY:
                    return GL_LINE_STRIP_ADJACENCY;

                case ConnectivityType::FAN:
                case ConnectivityType::NUMBER_OF_CONNECTIVITY_TYPES:
                default:
                    return GL_POINTS;
            }

        case DrawType::POINTS:
        case DrawType::NOT_SPECIFIED:
        case DrawType::NUMBER_OF_DRAW_TYPES:
        default:
            return GL_POINTS;
    }
}

}  // namespace
