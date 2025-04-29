/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#include <modules/base/processors/meshcreator.h>

#include <inviwo/core/algorithm/boundingbox.h>                          // for boundingBox
#include <inviwo/core/datastructures/buffer/buffer.h>                   // for Buffer, BufferBase
#include <inviwo/core/datastructures/buffer/bufferram.h>                // for BufferRAMPrecision
#include <inviwo/core/datastructures/camera/camera.h>                   // for mat4
#include <inviwo/core/datastructures/geometry/geometrytype.h>           // for BufferType, Buffe...
#include <inviwo/core/datastructures/geometry/mesh.h>                   // for Mesh, Mesh::Buffe...
#include <inviwo/core/datastructures/geometry/simplemeshcreator.h>      // for SimpleMeshCreator
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/interaction/events/interactionevent.h>            // for InteractionEvent
#include <inviwo/core/interaction/events/mousebuttons.h>                // for MouseButton, Mous...
#include <inviwo/core/interaction/events/mouseevent.h>                  // for MouseEvent
#include <inviwo/core/interaction/events/pickingevent.h>                // for PickingEvent
#include <inviwo/core/interaction/events/touchevent.h>                  // for TouchEvent, Touch...
#include <inviwo/core/interaction/events/touchstate.h>                  // for TouchState, Touch...
#include <inviwo/core/interaction/pickingmapper.h>                      // for PickingMapper
#include <inviwo/core/interaction/pickingstate.h>                       // for PickingState, Pic...
#include <inviwo/core/ports/meshport.h>                                 // for MeshOutport
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tags, Tags::CPU
#include <inviwo/core/properties/boolproperty.h>                        // for BoolProperty
#include <inviwo/core/properties/cameraproperty.h>                      // for CameraProperty
#include <inviwo/core/properties/invalidationlevel.h>                   // for InvalidationLevel
#include <inviwo/core/properties/optionproperty.h>                      // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>                     // for FloatVec3Property
#include <inviwo/core/properties/propertysemantics.h>                   // for PropertySemantics
#include <inviwo/core/util/glmvec.h>                                    // for vec3, vec4, vec2
#include <inviwo/core/util/staticstring.h>                              // for operator+
#include <inviwo/core/util/utilities.h>                                 // for show, hide
#include <modules/base/algorithm/meshutils.h>                           // for arrow, boundingBo...
#include <modules/base/properties/basisproperty.h>                      // for BasisProperty

#include <algorithm>      // for fill
#include <cstdint>        // for uint32_t
#include <type_traits>    // for remove_extent_t
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set

#include <flags/flags.h>                 // for operator&, flags
#include <glm/ext/matrix_transform.hpp>  // for scale, translate
#include <glm/vec2.hpp>                  // for vec, vec<>::(anon...
#include <glm/vec3.hpp>                  // for operator+, vec
#include <glm/vec4.hpp>                  // for operator*, operator+

namespace inviwo {

const ProcessorInfo MeshCreator::processorInfo_{"org.inviwo.MeshCreator",  // Class identifier
                                                "Mesh Creator",            // Display name
                                                "Mesh Creation",           // Category
                                                CodeState::Stable,         // Code state
                                                Tags::CPU,                 // Tags
                                                R"(
    Generate various meshes_
     * __Sphere__,
     * __ColorSphere__,
     * __CubeBasicMesh__,
     * __CubeSimpleMesh__,
     * __CubeIndicator__,
     * __LineCube__,
     * __LineCubeAdjacency__,
     * __Plane__,
     * __Disk__,
     * __Cone__,
     * __Cylinder__,
     * __Arrow__,
     * __CoordAxes__,
     * __Torus
    )"_unindentHelp};

const ProcessorInfo& MeshCreator::getProcessorInfo() const { return processorInfo_; }

MeshCreator::MeshCreator()
    : Processor()
    , outport_("outport", "The generated mesh"_help)
    , position1_("position1", "Start Position",
                 util::ordinalSymmetricVector(vec3{0.0f}, vec3{50.f})
                     .set("Start position of several mesh types"_help))
    , position2_("position2", "Stop Position",
                 util::ordinalSymmetricVector(vec3{1.0f, 0.0f, 0.0f}, vec3{50.f})
                     .set("Stop position of several mesh types"_help))
    , basis_("Basis", "Basis and offset")
    , normal_("normal", "Normal", "Normal direction for planes, etc."_help, vec3(0.0f, 0.0f, 1.0f),
              {vec3(-50.0f), ConstraintBehavior::Editable},
              {vec3(50.0f), ConstraintBehavior::Editable})
    , color_("color", "Color",
             util::ordinalColor(vec4(1.0f, 1.0f, 1.0f, 1.0f)).set("Color parameter"_help))
    , torusRadius1_("torusRadius1_", "Torus Radius 1", 1.0f)
    , torusRadius2_("torusRadius2_", "Torus Radius 2", 0.3f)
    , meshScale_("scale", "Size scaling",
                 util::ordinalScale(1.0f).set("Scale parameter for various mesh types"_help))
    , meshRes_("res", "Mesh resolution",
               util::ordinalCount(ivec2{16}).setMin(ivec2{1}).set(
                   "Mesh resolution parameter for some meshes"_help))
    , meshType_{"meshType",
                "Mesh Type",
                "The type of mesh to generate"_help,
                {{"sphere", "Sphere", MeshType::Sphere},
                 {"colorsphere", "Color Sphere", MeshType::ColorSphere},
                 {"cube_basic_mesh", "Cube (Basic Mesh)", MeshType::CubeBasicMesh},
                 {"cube", "Cube (Simple Mesh)", MeshType::CubeSimpleMesh},
                 {"cubeIndicator", "Cube Indicator", MeshType::CubeIndicator},
                 {"linecube", "Line cube", MeshType::LineCube},
                 {"linecubeadjacency", "Line cube adjacency", MeshType::LineCubeAdjacency},
                 {"plane", "Plane", MeshType::Plane},
                 {"disk", "Disk", MeshType::Disk},
                 {"cone", "Cone", MeshType::Cone},
                 {"cylinder", "Cylinder", MeshType::Cylinder},
                 {"arrow", "Arrow", MeshType::Arrow},
                 {"coordaxes", "Coordinate Indicator", MeshType::CoordAxes},
                 {"torus", "Torus", MeshType::Torus},
                 {"sphereopt", "Sphere with Position", MeshType::SphereOpt}},
                0}
    , enablePicking_("enablePicking", "Enable Picking", false)
    , picking_(this, 1,
               [&](PickingEvent* p) {
                   if (enablePicking_) handlePicking(p);
               })
    , camera_("camera", "Camera", util::boundingBox(outport_))
    , pickingUpdate_{[](PickingEvent*) {}} {

    addPort(outport_);

    util::hide(position1_, position2_, normal_, basis_, color_, torusRadius1_, torusRadius2_);
    util::show(meshScale_, meshRes_);

    meshType_.onChange([this]() {
        auto updateNone = [](PickingEvent*) {};

        auto getDelta = [this](PickingEvent* p) {
            auto currNDC = p->getNDC();
            auto prevNDC = p->getPreviousNDC();

            // Use depth of initial press as reference to move in the image plane.
            auto refDepth = p->getPressedDepth();
            currNDC.z = refDepth;
            prevNDC.z = refDepth;

            auto corrWorld =
                camera_.getWorldPosFromNormalizedDeviceCoords(static_cast<vec3>(currNDC));
            auto prevWorld =
                camera_.getWorldPosFromNormalizedDeviceCoords(static_cast<vec3>(prevNDC));
            return (corrWorld - prevWorld);
        };

        auto updatePosition1 = [this, getDelta](PickingEvent* p) {
            position1_.set(position1_.get() + getDelta(p));
        };

        auto updatePosition1and2 = [this, getDelta](PickingEvent* p) {
            auto delta = getDelta(p);
            position1_.set(position1_.get() + delta);
            position2_.set(position2_.get() + delta);
        };
        auto updateBasis = [this, getDelta](PickingEvent* p) {
            basis_.offset_.set(basis_.offset_.get() + getDelta(p));
        };

        util::hide(position1_, position2_, normal_, basis_, meshScale_, meshRes_, color_,
                   torusRadius1_, torusRadius2_);

        switch (meshType_.get()) {
            case MeshType::Sphere: {
                pickingUpdate_ = updateNone;
                util::show(meshScale_, meshRes_);
                break;
            }
            case MeshType::ColorSphere: {
                pickingUpdate_ = updatePosition1;
                util::show(position1_, meshScale_);
                break;
            }
            case MeshType::CubeBasicMesh: {
                pickingUpdate_ = updatePosition1and2;
                util::show(position1_, position2_, color_);
                break;
            }
            case MeshType::CubeSimpleMesh: {
                pickingUpdate_ = updatePosition1and2;
                util::show(position1_, position2_);
                break;
            }
            case MeshType::LineCube: {
                pickingUpdate_ = updateBasis;
                util::show(basis_, color_);
                break;
            }
            case MeshType::LineCubeAdjacency: {
                pickingUpdate_ = updateBasis;
                util::show(basis_, color_);
                break;
            }
            case MeshType::Plane: {
                pickingUpdate_ = updatePosition1;
                util::show(position1_, normal_, meshScale_, meshRes_, color_);
                break;
            }
            case MeshType::Disk: {
                pickingUpdate_ = updatePosition1;
                util::show(position1_, normal_, meshScale_, meshRes_, color_);
                break;
            }
            case MeshType::Cone: {
                pickingUpdate_ = updatePosition1and2;
                util::show(position1_, position2_, meshScale_, meshRes_, color_);
                break;
            }
            case MeshType::Cylinder: {
                pickingUpdate_ = updatePosition1and2;
                util::show(position1_, position2_, meshScale_, meshRes_, color_);
                break;
            }
            case MeshType::Arrow: {
                pickingUpdate_ = updatePosition1and2;
                util::show(position1_, position2_, meshScale_, meshRes_, color_);
                break;
            }
            case MeshType::CoordAxes: {
                pickingUpdate_ = updatePosition1;
                util::show(position1_, meshScale_);
                break;
            }
            case MeshType::Torus: {
                pickingUpdate_ = updatePosition1;
                util::show(position1_, torusRadius1_, torusRadius2_, meshRes_, color_);
                break;
            }
            case MeshType::SphereOpt: {
                pickingUpdate_ = updatePosition1;
                util::show(position1_, meshScale_, color_);
                break;
            }
            default: {
                pickingUpdate_ = updateNone;
                util::show(meshScale_, meshRes_);
                break;
            }
        }
    });

    addProperty(meshType_);
    addProperty(position1_);
    addProperty(position2_);
    addProperty(normal_);
    addProperty(basis_);
    addProperty(torusRadius1_);
    addProperty(torusRadius2_);
    addProperty(color_);
    addProperty(meshScale_);
    addProperty(meshRes_);

    addProperty(enablePicking_);
    addProperty(camera_);
    camera_.setInvalidationLevel(InvalidationLevel::Valid);
    camera_.setCollapsed(true);
}

MeshCreator::~MeshCreator() {}

std::shared_ptr<Mesh> MeshCreator::createMesh() {
    switch (meshType_.get()) {
        case MeshType::Sphere: {
            auto mesh = SimpleMeshCreator::sphere(1.0f, meshRes_.get().y, meshRes_.get().x);
            mesh->setModelMatrix(glm::scale(vec3{meshScale_.get()}));
            return mesh;
        }
        case MeshType::ColorSphere:
            return meshutil::colorsphere(position1_, meshScale_.get());
        case MeshType::CubeBasicMesh: {
            vec3 posLLF = position1_;
            vec3 posURB = position2_;

            mat4 m = glm::translate(mat4(1.0f), posLLF);
            m = glm::scale(m, posURB - posLLF);

            return meshutil::cube(m, color_.get());
        }
        case MeshType::CubeSimpleMesh: {
            vec3 posLLF = position1_;
            vec3 posURB = position2_;

            return SimpleMeshCreator::rectangularPrism(posLLF, posURB, posLLF, posURB,
                                                       vec4(posLLF, 1.f), vec4(posURB, 1.f));
        }
        case MeshType::CubeIndicator:
            return meshutil::cubeIndicator(basis_.getBasisAndOffset());
        case MeshType::LineCube:
            return meshutil::boundingbox(basis_.getBasisAndOffset(), color_);

        case MeshType::LineCubeAdjacency:
            return meshutil::boundingBoxAdjacency(basis_.getBasisAndOffset(), color_);

        case MeshType::Plane: {
            return meshutil::square(position1_, normal_, vec2(1.0f, 1.0f) * meshScale_.get(),
                                    color_, meshRes_.get());
        }
        case MeshType::Disk:
            return meshutil::disk(position1_, normal_, color_, meshScale_.get(), meshRes_.get().x);
        case MeshType::Cone:
            return meshutil::cone(position1_, position2_, color_, meshScale_.get(),
                                  meshRes_.get().x);
        case MeshType::Cylinder:
            return meshutil::cylinder(position1_, position2_, color_, meshScale_.get(),
                                      meshRes_.get().x);
        case MeshType::Arrow:
            return meshutil::arrow(position1_, position2_, color_, meshScale_.get(), 0.15f,
                                   meshScale_.get() * 2, meshRes_.get().x);
        case MeshType::CoordAxes:
            return meshutil::coordindicator(position1_, meshScale_.get());
        case MeshType::Torus:
            return meshutil::torus(position1_, vec3(0, 0, 1), torusRadius1_, torusRadius2_,
                                   meshRes_, color_);
        case MeshType::SphereOpt:
            return meshutil::sphere(position1_, meshScale_, color_);
        default:
            return SimpleMeshCreator::sphere(0.1f, meshRes_.get().x, meshRes_.get().y);
    }
}

void MeshCreator::handlePicking(PickingEvent* p) {
    if (p->getState() == PickingState::Updated && p->getEvent()->hash() == MouseEvent::chash()) {
        auto me = p->getEventAs<MouseEvent>();
        if ((me->buttonState() & MouseButton::Left) && me->state() == MouseState::Move) {
            pickingUpdate_(p);
            p->markAsUsed();
        }
    } else if (p->getState() == PickingState::Updated &&
               p->getEvent()->hash() == TouchEvent::chash()) {

        auto te = p->getEventAs<TouchEvent>();
        if (!te->touchPoints().empty() && te->touchPoints()[0].state() == TouchState::Updated) {
            pickingUpdate_(p);
            p->markAsUsed();
        }
    }
}

void MeshCreator::process() {
    auto mesh = createMesh();

    if (enablePicking_) {
        // Add picking ids
        auto bufferRAM =
            std::make_shared<BufferRAMPrecision<uint32_t>>(mesh->getBuffer(0)->getSize());
        auto pickBuffer = std::make_shared<Buffer<uint32_t>>(bufferRAM);
        auto& data = bufferRAM->getDataContainer();
        std::fill(data.begin(), data.end(), static_cast<uint32_t>(picking_.getPickingId(0)));
        mesh->addBuffer(Mesh::BufferInfo(BufferType::PickingAttrib), pickBuffer);
    }

    outport_.setData(mesh);
}

}  // namespace inviwo
