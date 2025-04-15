/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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

#include <modules/base/processors/randommeshgenerator.h>

#include <inviwo/core/algorithm/boundingbox.h>                          // for boundingBox
#include <inviwo/core/datastructures/buffer/buffer.h>                   // for Buffer, BufferBase
#include <inviwo/core/datastructures/buffer/bufferram.h>                // for BufferRAMPrecision
#include <inviwo/core/datastructures/camera/camera.h>                   // for mat4
#include <inviwo/core/datastructures/geometry/geometrytype.h>           // for BufferType, Buffe...
#include <inviwo/core/datastructures/geometry/mesh.h>                   // for Mesh
#include <inviwo/core/datastructures/geometry/typedmesh.h>              // for BasicMesh, TypedMesh
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/interaction/events/pickingevent.h>                // for PickingEvent
#include <inviwo/core/interaction/events/interactionevent.h>            // for InteractionEvent
#include <inviwo/core/interaction/events/contextmenuevent.h>            // for ContextMenuEvent
#include <inviwo/core/interaction/events/mouseevent.h>                  // for MouseEvent
#include <inviwo/core/interaction/pickingmapper.h>                      // for PickingMapper
#include <inviwo/core/interaction/pickingstate.h>                       // for PickingPressItem
#include <inviwo/core/ports/meshport.h>                                 // for MeshOutport
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tags, Tags::CPU
#include <inviwo/core/properties/boolproperty.h>                        // for BoolProperty
#include <inviwo/core/properties/buttonproperty.h>                      // for ButtonProperty
#include <inviwo/core/properties/cameraproperty.h>                      // for CameraProperty
#include <inviwo/core/properties/invalidationlevel.h>                   // for InvalidationLevel
#include <inviwo/core/properties/ordinalproperty.h>                     // for IntProperty, Floa...
#include <inviwo/core/util/glmvec.h>                                    // for vec3, ivec2, vec4
#include <modules/base/algorithm/meshutils.h>                           // for cone, cube, cylinder

#include <algorithm>      // for fill
#include <cstdint>        // for uint32_t
#include <memory>         // for shared_ptr, make_...
#include <string>         // for string
#include <string_view>    // for string_view
#include <type_traits>    // for remove_extent_t
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set

#include <glm/ext/matrix_transform.hpp>  // for rotate, scale
#include <glm/geometric.hpp>             // for normalize
#include <glm/gtc/type_precision.hpp>    // for i64
#include <glm/vec3.hpp>                  // for operator*, operator+
#include <glm/vec4.hpp>                  // for operator*, operator+

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo RandomMeshGenerator::processorInfo_{
    "org.inviwo.RandomMeshGenerator",  // Class identifier
    "Random Mesh Generator",           // Display name
    "Mesh Creation",                   // Category
    CodeState::Stable,                 // Code state
    Tags::CPU,                         // Tags
    R"(
    Creates a mesh consisting of randomly created objects. The individual objects can be
    transformed if picking is enabled. This processor also uses a custom context menu to
    randomize the seed.
    )"_unindentHelp,
};

const ProcessorInfo& RandomMeshGenerator::getProcessorInfo() const { return processorInfo_; }

RandomMeshGenerator::RandomMeshGenerator()
    : Processor()
    , mesh_("mesh")
    , rand_()
    , seed_("seed", "Seed", 0, 0, std::mt19937::max())
    , reseed_("reseed_", "Randomize Seed")
    , scale_("scale", "Scale", 1.0f, 0.001f, 1000.0f, 0.1f)
    , size_("size", "Size", 1.0f, 0.001f, 1000.0f, 0.1f)
    , numberOfBoxes_("numberOf_", "Number of Boxes", 1, 0, 100)
    , numberOfSpheres_("numberOfSpheres_", "Number of Spheres", 1, 0, 100)
    , numberOfCylinders_("numberOfCylinders_", "Number of cylinders", 1, 0, 100)
    , numberOfCones_("numberOfCones_", "Number of Cones", 1, 0, 100)
    , numberOfToruses_("numberOfToruses", "Number of Toruses", 1, 0, 100)
    , enablePicking_("enablePicking", "Enable Picking", false)
    , boxPicking_(this, numberOfBoxes_.get(),
                  [&](PickingEvent* p) {
                      handlePicking(
                          p, [&](vec3 delta) { boxes_[p->getPickedId()].position += delta; });
                  })
    , spherePicking_(this, numberOfSpheres_.get(),
                     [&](PickingEvent* p) {
                         handlePicking(
                             p, [&](vec3 delta) { spheres_[p->getPickedId()].center += delta; });
                     })
    , cylinderPicking_(this, numberOfCylinders_.get(),
                       [&](PickingEvent* p) {
                           handlePicking(p, [&](vec3 delta) {
                               cylinders_[p->getPickedId()].start += delta;
                               cylinders_[p->getPickedId()].end += delta;
                           });
                       })
    , conePicking_(this, numberOfCones_.get(),
                   [&](PickingEvent* p) {
                       handlePicking(p, [&](vec3 delta) {
                           cones_[p->getPickedId()].start += delta;
                           cones_[p->getPickedId()].end += delta;
                       });
                   })
    , torusPicking_(this, numberOfToruses_.get(),
                    [&](PickingEvent* p) {
                        handlePicking(
                            p, [&](vec3 delta) { toruses_[p->getPickedId()].center += delta; });
                    })
    , camera_("camera", "Camera", util::boundingBox(mesh_)) {

    addPort(mesh_);

    addProperty(scale_);
    addProperty(size_);
    addProperty(numberOfBoxes_);
    addProperty(numberOfSpheres_);
    addProperty(numberOfCylinders_);
    addProperty(numberOfCones_);
    addProperty(numberOfToruses_);

    addProperty(seed_);
    addProperty(reseed_);

    addProperty(enablePicking_);
    addProperty(camera_);
    camera_.setInvalidationLevel(InvalidationLevel::Valid);
    camera_.setCollapsed(true);

    reseed_.onChange([&]() {
        seed_.set(static_cast<glm::i64>(rand(0.0f, static_cast<float>(seed_.getMaxValue()))));
        rand_.seed(static_cast<std::mt19937::result_type>(seed_.get()));
    });
}

void RandomMeshGenerator::process() {
    rand_.seed(static_cast<std::mt19937::result_type>(seed_.get()));

    auto randPos = [&]() { return randVec3(-scale_.get(), scale_.get()); };
    auto randSize = [&]() { return size_.get() * randVec3(0.1f, 1.0f); };
    auto randColor = [&]() { return vec4(randVec3(0.5f, 1.0f), 1); };
    auto randDir = [&]() { return glm::normalize(randVec3()); };
    auto randScale = [&]() { return size_.get() * rand(0.1f, 1.0f); };

    const bool dirty = seed_.isModified() || size_.isModified() || scale_.isModified();

    if (numberOfBoxes_.isModified() || dirty) {
        boxes_.clear();
        boxPicking_.resize(numberOfBoxes_);
        for (int i = 0; i < numberOfBoxes_.get(); i++) {
            boxes_.push_back({randVec3(0, 6.28f), randPos(), randSize(), randColor()});
        }
    }
    if (numberOfSpheres_.isModified() || dirty) {
        spheres_.clear();
        spherePicking_.resize(numberOfSpheres_);
        for (int i = 0; i < numberOfSpheres_.get(); i++) {
            spheres_.push_back({randPos(), randScale(), randColor()});
        }
    }
    if (numberOfCylinders_.isModified() || dirty) {
        cylinders_.clear();
        cylinderPicking_.resize(numberOfCylinders_);
        for (int i = 0; i < numberOfCylinders_.get(); i++) {
            const auto r = randPos();
            cylinders_.push_back(
                {r, r + 10.0f * randScale() * randDir(), randScale(), randColor()});
        }
    }
    if (numberOfCones_.isModified() || dirty) {
        cones_.clear();
        conePicking_.resize(numberOfCones_);
        for (int i = 0; i < numberOfCones_.get(); i++) {
            const auto r = randPos();
            cones_.push_back({r, r + 10.0f * randScale() * randDir(), randScale(), randColor()});
        }
    }
    if (numberOfToruses_.isModified() || dirty) {
        toruses_.clear();
        torusPicking_.resize(numberOfToruses_);
        for (int i = 0; i < numberOfToruses_.get(); i++) {
            const auto r2 = size_.get() * rand(0.1f, 0.5f);
            const auto r1 = size_.get() * rand(0.1f + r2, 1.0f + r2);
            toruses_.push_back({randPos(), randDir(), r2, r1, randColor()});
        }
    }

    auto mesh = std::make_shared<BasicMesh>();
    if (enablePicking_) addPickingBuffer(*mesh, 0);

    size_t i = 0;
    for (const auto& box : boxes_) {
        mat4 o = glm::translate(mat4(1.0f), box.position);
        o = glm::rotate(o, box.rotation.x, vec3(1, 0, 0));
        o = glm::rotate(o, box.rotation.y, vec3(0, 1, 0));
        o = glm::rotate(o, box.rotation.z, vec3(0, 0, 1));
        o = glm::scale(o, box.scale);
        auto mesh2 = meshutil::cube(o, box.color);
        if (enablePicking_) addPickingBuffer(*mesh2, boxPicking_.getPickingId(i++));
        mesh->Mesh::append(*mesh2);
    }

    i = 0;
    for (const auto& sphere : spheres_) {
        auto mesh2 = meshutil::sphere(sphere.center, sphere.radius, sphere.color);
        if (enablePicking_) addPickingBuffer(*mesh2, spherePicking_.getPickingId(i++));
        mesh->Mesh::append(*mesh2);
    }

    i = 0;
    for (const auto& cylinder : cylinders_) {
        auto mesh2 =
            meshutil::cylinder(cylinder.start, cylinder.end, cylinder.color, cylinder.radius);
        if (enablePicking_) addPickingBuffer(*mesh2, cylinderPicking_.getPickingId(i++));
        mesh->Mesh::append(*mesh2);
    }

    i = 0;
    for (const auto& cone : cones_) {
        auto mesh2 = meshutil::cone(cone.start, cone.end, cone.color, cone.radius);
        if (enablePicking_) addPickingBuffer(*mesh2, conePicking_.getPickingId(i++));
        mesh->Mesh::append(*mesh2);
    }

    i = 0;
    for (const auto& torus : toruses_) {
        auto mesh2 = meshutil::torus(torus.center, torus.up, torus.radius1, torus.radius2,
                                     ivec2(32, 8), torus.color);
        if (enablePicking_) addPickingBuffer(*mesh2, torusPicking_.getPickingId(i++));
        mesh->Mesh::append(*mesh2);
    }

    mesh_.setData(mesh);
}

void RandomMeshGenerator::invokeEvent(Event* event) {
    Processor::invokeEvent(event);
    if (event->hasBeenUsed()) {
        return;
    }

    if (event->hash() == MouseEvent::chash()) {
        auto* mouseEvent = event->getAs<MouseEvent>();
        if (mouseEvent->button() & MouseButton::Right &&
            mouseEvent->state() & MouseState::Release) {

            auto uniqueId = [this](std::string_view str) {
                return fmt::format("{}.{}", getIdentifier(), str);
            };
            std::array entries = {
                ContextMenuEntry{"Randomize Mesh Seed", fmt::format("{}.reseed", getIdentifier())}};
            mouseEvent->showContextMenu(entries,
                                        ContextMenuAction::Custom | ContextMenuAction::View);
            mouseEvent->setUsed(true);
        }
    } else if (event->hash() == ContextMenuEvent::chash()) {
        auto* menuEvent = event->getAs<ContextMenuEvent>();
        if (menuEvent->getId().starts_with(getIdentifier())) {
            if (menuEvent->getId().ends_with(".reseed")) {
                reseed_.pressButton();
            }
            menuEvent->setUsed(true);
        }
    }
}

float RandomMeshGenerator::rand(const float min, const float max) {
    return min + dis_(rand_) * (max - min);
}

vec3 RandomMeshGenerator::randVec3(const float min, const float max) {
    float x = rand(min, max);
    float y = rand(min, max);
    float z = rand(min, max);
    return vec3(x, y, z);
}

void RandomMeshGenerator::addPickingBuffer(Mesh& mesh, size_t id) {
    // Add picking ids
    auto bufferRAM = std::make_shared<BufferRAMPrecision<uint32_t>>(mesh.getBuffer(0)->getSize());
    auto pickBuffer = std::make_shared<Buffer<uint32_t>>(bufferRAM);
    auto& data = bufferRAM->getDataContainer();
    std::fill(data.begin(), data.end(), static_cast<uint32_t>(id));
    mesh.addBuffer(BufferType::PickingAttrib, pickBuffer);
}

void RandomMeshGenerator::handlePicking(PickingEvent* p, std::function<void(vec3)> callback) {
    if (p->getPressState() == PickingPressState::Move &&
        p->getPressItems().count(PickingPressItem::Primary)) {
        callback(vec3{p->getWorldSpaceDeltaAtPressDepth(camera_)});
        p->markAsUsed();
        invalidate(InvalidationLevel::InvalidOutput);
    }
}

}  // namespace inviwo
