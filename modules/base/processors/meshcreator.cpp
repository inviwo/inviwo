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

#include "meshcreator.h"
#include <inviwo/core/datastructures/geometry/simplemeshcreator.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>

namespace inviwo {

ProcessorClassIdentifier(MeshCreator, "org.inviwo.MeshCreator");
ProcessorDisplayName(MeshCreator, "Mesh Creator");
ProcessorTags(MeshCreator, Tags::CPU);
ProcessorCategory(MeshCreator, "Geometry Creation");
ProcessorCodeState(MeshCreator, CODE_STATE_STABLE);

MeshCreator::MeshCreator()
    : Processor()
    , outport_("outport")
    , position1_("position1", "Start Position", vec3(0.0f, 0.0f, 0.0f), vec3(-50.0f), vec3(50.0f))
    , position2_("position2", "Stop Position", vec3(1.0f, 0.0f, 0.0f), vec3(-50.0f), vec3(50.0f))
    , normal_("normal", "Normal", vec3(0.0f, 0.0f, 1.0f), vec3(-50.0f), vec3(50.0f))
    , color_("color", "Color", vec4(1.0f, 1.0f, 1.0f, 1.0f), vec4(0.0f), vec4(1.0f), vec4(0.01f),
             INVALID_OUTPUT, PropertySemantics::Color)
    , meshScale_("scale", "Size scaling", 1.f, 0.01f, 10.f)
    , meshRes_("res", "Mesh resolution", vec2(16), vec2(1), vec2(1024))
    , meshType_("meshType", "Mesh Type") {
    addPort(outport_);

    meshType_.addOption("sphere", "Sphere", SPHERE);
    meshType_.addOption("colorsphere", "Color Sphere", COLOR_SPHERE);
    meshType_.addOption("cube", "Cube", CUBE);
    meshType_.addOption("linecube", "Line cube", LINE_CUBE);
    meshType_.addOption("plane", "Plane", PLANE);
    meshType_.addOption("disk", "Disk", DISK);
    meshType_.addOption("cone", "Cone", CONE);
    meshType_.addOption("cylinder", "Cylinder", CYLINDER);
    meshType_.addOption("arrow", "Arrow", ARROW);
    meshType_.addOption("coordaxes", "Coordinate Indicator", COORD_AXES);

    meshType_.set(SPHERE);
    meshType_.setCurrentStateAsDefault();

    addProperty(meshType_);
    addProperty(position1_);
    addProperty(position2_);
    addProperty(normal_);
    addProperty(color_);
    addProperty(meshScale_);
    addProperty(meshRes_);
}

MeshCreator::~MeshCreator() {}

void MeshCreator::initialize() { Processor::initialize(); }

void MeshCreator::deinitialize() { Processor::deinitialize(); }

Mesh* MeshCreator::createMesh() {
    switch (meshType_.getSelectedIndex()) {
        case SPHERE:
            return SimpleMeshCreator::sphere(0.5f * meshScale_.get(), meshRes_.get().y,
                                             meshRes_.get().x);
        case COLOR_SPHERE:
            // TODO: use given mesh resolution!
            return BasicMesh::colorsphere(position1_, meshScale_.get());
        case CUBE: {
            vec3 posLLF = position1_;
            vec3 posURB = position2_;
            return SimpleMeshCreator::rectangularPrism(posLLF, posURB, posLLF, posURB,
                                                       vec4(posLLF, 1.f), vec4(posURB, 1.f));
        }
        case LINE_CUBE:
            return BasicMesh::boundingbox(
                glm::translate(glm::scale(mat4(1.0f), position2_.get()), position1_.get()), color_);
        case PLANE: {
            return BasicMesh::square(position1_, normal_, vec2(1.0f, 1.0f) * meshScale_.get(),
                                     color_, meshRes_.get());
        }
        case DISK:
            return BasicMesh::disk(position1_, normal_, color_, meshScale_.get(), meshRes_.get().x);
        case CONE:
            return BasicMesh::cone(position1_, position2_, color_, meshScale_.get(),
                                   meshRes_.get().x);
        case CYLINDER:
            return BasicMesh::cylinder(position1_, position2_, color_, meshScale_.get(),
                                       meshRes_.get().x);
        case ARROW:
            return BasicMesh::arrow(position1_, position2_, color_, meshScale_.get(), 0.15f,
                                    meshScale_.get() * 2, meshRes_.get().x);
        case COORD_AXES:
            return BasicMesh::coordindicator(position1_, meshScale_.get());
        default:
            return SimpleMeshCreator::sphere(0.1f, meshRes_.get().x, meshRes_.get().y);
    }
}

void MeshCreator::process() { outport_.setData(createMesh()); }

}  // namespace
