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

#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/datastructures/geometry/simplemeshcreator.h>
#include "meshcreator.h"

namespace inviwo {

const ProcessorInfo MeshCreator::processorInfo_{
    "org.inviwo.MeshCreator",  // Class identifier
    "Mesh Creator",            // Display name
    "Geometry Creation",       // Category
    CodeState::Stable,         // Code state
    Tags::CPU,                 // Tags
};
const ProcessorInfo MeshCreator::getProcessorInfo() const { return processorInfo_; }

MeshCreator::MeshCreator()
    : Processor()
    , outport_("outport")
    , position1_("position1", "Start Position", vec3(0.0f, 0.0f, 0.0f), vec3(-50.0f), vec3(50.0f))
    , position2_("position2", "Stop Position", vec3(1.0f, 0.0f, 0.0f), vec3(-50.0f), vec3(50.0f))
    , basis_("Basis", "Basis and offset")
    , normal_("normal", "Normal", vec3(0.0f, 0.0f, 1.0f), vec3(-50.0f), vec3(50.0f))
    , color_("color", "Color", vec4(1.0f, 1.0f, 1.0f, 1.0f), vec4(0.0f), vec4(1.0f), vec4(0.01f),
             InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , meshScale_("scale", "Size scaling", 1.f, 0.01f, 10.f)
    , meshRes_("res", "Mesh resolution", vec2(16), vec2(1), vec2(1024))
    , meshType_("meshType", "Mesh Type") {
    addPort(outport_);

    meshType_.addOption("sphere", "Sphere", MeshType::Sphere);
    meshType_.addOption("colorsphere", "Color Sphere", MeshType::ColorSphere);
    meshType_.addOption("cube_basic_mesh", "Cube (Basic Mesh)", MeshType::CubeBasicMesh);
    meshType_.addOption("cube", "Cube (Simple Mesh)", MeshType::CubeSimpleMesh);
    meshType_.addOption("linecube", "Line cube", MeshType::LineCube);
    meshType_.addOption("linecubeadjacency", "Line cube adjacency", MeshType::LineCubeAdjacency);
    meshType_.addOption("plane", "Plane", MeshType::Plane);
    meshType_.addOption("disk", "Disk", MeshType::Disk);
    meshType_.addOption("cone", "Cone", MeshType::Cone);
    meshType_.addOption("cylinder", "Cylinder", MeshType::Cylinder);
    meshType_.addOption("arrow", "Arrow", MeshType::Arrow);
    meshType_.addOption("coordaxes", "Coordinate Indicator", MeshType::CoordAxes);

    hide(position1_, position2_, normal_, basis_, color_);
    show(meshScale_, meshRes_);

    meshType_.set(MeshType::Sphere);
    meshType_.setCurrentStateAsDefault();

    meshType_.onChange([&]() {
        hide(position1_, position2_, normal_, basis_, meshScale_, meshRes_, color_);
        
        switch (meshType_.get()) {
            case MeshType::Sphere: {
                show(meshScale_, meshRes_);
                break;
            }
            case MeshType::ColorSphere: {
                show(position1_, meshScale_);
                break;
            }
            case MeshType::CubeBasicMesh: {
                show(position1_, position2_, color_);
                break;
            }
            case MeshType::CubeSimpleMesh: {
                show(position1_, position2_);
                break;
            }
            case MeshType::LineCube: {
                show(basis_, color_);
                break;
            }
            case MeshType::LineCubeAdjacency: {
                show(basis_, color_);
                break;
            }
            case MeshType::Plane: {
                show(position1_, normal_, meshScale_, meshRes_, color_);
                break;
            }
            case MeshType::Disk: {
                show(position1_, normal_, meshScale_, meshRes_, color_);
                break;
            }
            case MeshType::Cone: {
                show(position1_, position2_, meshScale_, meshRes_, color_);
                break;
            }
            case MeshType::Cylinder: {
                show(position1_, position2_, meshScale_, meshRes_, color_);
                break;
            }
            case MeshType::Arrow: {
                show(position1_, position2_, meshScale_, meshRes_, color_);
                break;
            }
            case MeshType::CoordAxes: {
                show(position1_, meshScale_);
                break;
            }
            default: {
                show(meshScale_, meshRes_);
                break;
            }
        }

    });

    addProperty(meshType_);
    addProperty(position1_);
    addProperty(position2_);
    addProperty(normal_);
    addProperty(basis_);
    addProperty(color_);
    addProperty(meshScale_);
    addProperty(meshRes_);
}

MeshCreator::~MeshCreator() {}

std::shared_ptr<Mesh> MeshCreator::createMesh() {
    switch (meshType_.get()) {
        case MeshType::Sphere:
            return SimpleMeshCreator::sphere(0.5f * meshScale_.get(), meshRes_.get().y,
                                             meshRes_.get().x);
        case MeshType::ColorSphere:
            // TODO: use given mesh resolution!
            return BasicMesh::colorsphere(position1_, meshScale_.get());
        case MeshType::CubeBasicMesh: {
            vec3 posLLF = position1_;
            vec3 posURB = position2_;

            mat4 m = glm::translate(mat4(1.0f), posLLF);
            m = glm::scale(m, posURB - posLLF);

            return BasicMesh::cube(m, color_.get());
        }
        case MeshType::CubeSimpleMesh: {
            vec3 posLLF = position1_;
            vec3 posURB = position2_;

            return SimpleMeshCreator::rectangularPrism(posLLF, posURB, posLLF, posURB,
                                                       vec4(posLLF, 1.f), vec4(posURB, 1.f));
        }
        case MeshType::LineCube:
            return BasicMesh::boundingbox(basis_.getBasisAndOffset(), color_);

        case MeshType::LineCubeAdjacency:
            return BasicMesh::boundingBoxAdjacency(basis_.getBasisAndOffset(), color_);

        case MeshType::Plane: {
            return BasicMesh::square(position1_, normal_, vec2(1.0f, 1.0f) * meshScale_.get(),
                                     color_, meshRes_.get());
        }
        case MeshType::Disk:
            return BasicMesh::disk(position1_, normal_, color_, meshScale_.get(), meshRes_.get().x);
        case MeshType::Cone:
            return BasicMesh::cone(position1_, position2_, color_, meshScale_.get(),
                                   meshRes_.get().x);
        case MeshType::Cylinder:
            return BasicMesh::cylinder(position1_, position2_, color_, meshScale_.get(),
                                       meshRes_.get().x);
        case MeshType::Arrow:
            return BasicMesh::arrow(position1_, position2_, color_, meshScale_.get(), 0.15f,
                                    meshScale_.get() * 2, meshRes_.get().x);
        case MeshType::CoordAxes:
            return BasicMesh::coordindicator(position1_, meshScale_.get());
        default:
            return SimpleMeshCreator::sphere(0.1f, meshRes_.get().x, meshRes_.get().y);
    }
}

void MeshCreator::process() { outport_.setData(createMesh()); }

}  // namespace
