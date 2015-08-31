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

namespace inviwo {

MeshDrawerGL::MeshDrawerGL() : meshToDraw_(nullptr) {}

MeshDrawerGL::MeshDrawerGL(const Mesh* mesh)
    : meshToDraw_(mesh) {

    initialize(mesh->getDefaultAttributesInfo());
}

MeshDrawerGL::MeshDrawerGL(const Mesh* mesh, Mesh::AttributesInfo ai)
    : meshToDraw_(mesh) {

    initialize(ai);
}

MeshDrawerGL::MeshDrawerGL(const Mesh* mesh, GeometryEnums::DrawType dt, GeometryEnums::ConnectivityType ct)
    : meshToDraw_(mesh) {

    initialize(Mesh::AttributesInfo(dt, ct));
}

MeshDrawerGL::MeshDrawerGL(MeshDrawerGL&& other)
        : MeshDrawer(std::move(other))
        , meshToDraw_{ other.meshToDraw_ }
{
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

MeshDrawerGL::~MeshDrawerGL() {
}

void MeshDrawerGL::draw() {
    const MeshGL* meshGL = getMeshGL();
    meshGL->enable();
    // If default is indices, render all index lists
    if (!drawMethods_[0].elementBufferList.empty()) {
        for (int i = 1; i < GeometryEnums::NUMBER_OF_DRAW_TYPES; i++) {
            if (!drawMethods_[i].elementBufferList.empty())
                (this->*drawMethods_[i].drawFunc)(static_cast<GeometryEnums::DrawType>(i));
        }
    } else {
        // Render just default one
        (this->*drawMethods_[0].drawFunc)(GeometryEnums::NOT_SPECIFIED);
    }
    meshGL->disable();
}

void MeshDrawerGL::draw(GeometryEnums::DrawType dt) {
    const MeshGL* meshGL = getMeshGL();
    meshGL->enable();
    (this->*drawMethods_[dt].drawFunc)(dt);
    meshGL->disable();
}

const MeshGL* MeshDrawerGL::getMeshGL() const {
    return meshToDraw_->getRepresentation<MeshGL>();
}

GLenum MeshDrawerGL::getDefaultDrawMode() {
    return drawMethods_[0].drawMode;
}

GLenum MeshDrawerGL::getDrawMode(GeometryEnums::DrawType dt, GeometryEnums::ConnectivityType ct) {
    switch (dt)
    {
        case GeometryEnums::TRIANGLES:
            switch (ct)
            {
                case GeometryEnums::NONE:
                    return GL_TRIANGLES;

                case GeometryEnums::STRIP:
                    return GL_TRIANGLE_STRIP;

                case GeometryEnums::FAN:
                    return GL_TRIANGLE_FAN;

                case GeometryEnums::ADJACENCY:
                    return GL_TRIANGLES_ADJACENCY;

                case GeometryEnums::STRIP_ADJACENCY:
                    return GL_TRIANGLE_STRIP_ADJACENCY;

                case GeometryEnums::LOOP:
                case GeometryEnums::NUMBER_OF_CONNECTIVITY_TYPES:
                default:
                    return GL_POINTS;
            }

        case GeometryEnums::LINES:
            switch (ct)
            {
                case GeometryEnums::NONE:
                    return GL_LINES;

                case GeometryEnums::STRIP:
                    return GL_LINE_STRIP;

                case GeometryEnums::LOOP:
                    return GL_LINE_LOOP;

                case GeometryEnums::ADJACENCY:
                    return GL_LINES_ADJACENCY;

                case GeometryEnums::STRIP_ADJACENCY:
                    return GL_LINE_STRIP_ADJACENCY;

                case GeometryEnums::FAN:
                case GeometryEnums::NUMBER_OF_CONNECTIVITY_TYPES:
                default:
                    return GL_POINTS;
            }

        case GeometryEnums::POINTS:
        case GeometryEnums::NOT_SPECIFIED:
        case GeometryEnums::NUMBER_OF_DRAW_TYPES:
        default:
            return GL_POINTS;
    }
}

void MeshDrawerGL::drawArray(GeometryEnums::DrawType dt) const {
    glDrawArrays(drawMethods_[dt].drawMode,
                 0,
                 static_cast<GLsizei>(meshToDraw_->getAttributes(0)->getSize()));
}

void MeshDrawerGL::drawElements(GeometryEnums::DrawType dt) const {
    
    std::vector<const Buffer*>::const_iterator it = drawMethods_[dt].elementBufferList.begin();
    while (it != drawMethods_[dt].elementBufferList.end()) {
        const ElementBufferGL* elementBufferGL = (*it)->getRepresentation<ElementBufferGL>();
        elementBufferGL->bind();
        glDrawElements(drawMethods_[dt].drawMode, static_cast<GLsizei>(elementBufferGL->getSize()),
                       elementBufferGL->getFormatType(), nullptr);
        ++it;
    }
}

void MeshDrawerGL::initialize(Mesh::AttributesInfo ai)
{
    drawMethods_[0].drawFunc = &MeshDrawerGL::emptyFunc;
    drawMethods_[0].drawMode = getDrawMode(ai.dt, ai.ct);
    drawMethods_[0].elementBufferList.clear();

    for (int i=1; i<GeometryEnums::NUMBER_OF_DRAW_TYPES; i++) {
        drawMethods_[i].drawFunc = drawMethods_[0].drawFunc;
        drawMethods_[i].drawMode = drawMethods_[0].drawMode;
        drawMethods_[i].elementBufferList.clear();
    }

    drawMethods_[ai.dt].drawFunc = &MeshDrawerGL::drawArray;
    drawMethods_[GeometryEnums::NOT_SPECIFIED].drawFunc = &MeshDrawerGL::drawArray;
    //drawMethods_[POINTS].drawFunc = &MeshDrawer::renderArray;
    //drawMethods_[POINTS].drawMode = GL_POINTS;

    for (size_t i=0; i < meshToDraw_->getNumberOfIndicies(); ++i) {
        if (meshToDraw_->getIndicies(i)->getSize() > 0)
            initializeIndexBuffer(meshToDraw_->getIndicies(i), meshToDraw_->getIndexAttributesInfo(i));
    }
}
void MeshDrawerGL::initializeIndexBuffer(const Buffer* indexBuffer, Mesh::AttributesInfo ai) {
    // check draw mode if there exists another indexBuffer
    if (drawMethods_[ai.dt].elementBufferList.size() != 0) {
        if (getDrawMode(ai.dt, ai.ct) != drawMethods_[ai.dt].drawMode) {
            LogWarn("draw mode mismatch (element buffer " << ai.dt << ")");
        }
    } 
    else {
        drawMethods_[ai.dt].drawFunc = &MeshDrawerGL::drawElements;
        drawMethods_[ai.dt].drawMode = getDrawMode(ai.dt, ai.ct);
    }
    drawMethods_[ai.dt].elementBufferList.push_back(indexBuffer);

    // Specify first element buffer as default rendering method
    if(drawMethods_[GeometryEnums::NOT_SPECIFIED].elementBufferList.size() == 0) {
        drawMethods_[GeometryEnums::NOT_SPECIFIED].drawFunc = drawMethods_[ai.dt].drawFunc;
        drawMethods_[GeometryEnums::NOT_SPECIFIED].drawMode = drawMethods_[ai.dt].drawMode;
        drawMethods_[GeometryEnums::NOT_SPECIFIED].elementBufferList.push_back(drawMethods_[ai.dt].elementBufferList.at(0));
    }
}


} // namespace

