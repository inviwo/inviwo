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

#include <modules/base/processors/randomspheregenerator.h>
#include <inviwo/core/util/indexmapper.h>

#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/touchevent.h>
#include <inviwo/core/interaction/events/wheelevent.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/algorithm/boundingbox.h>

#include <numeric>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo RandomSphereGenerator::processorInfo_{
    "org.inviwo.RandomSphereGenerator",  // Class identifier
    "Random Sphere Generator",           // Display name
    "Mesh Creation",                     // Category
    CodeState::Stable,                   // Code state
    Tags::CPU,                           // Tags
};
const ProcessorInfo RandomSphereGenerator::getProcessorInfo() const { return processorInfo_; }

RandomSphereGenerator::RandomSphereGenerator()
    : Processor()
    , meshOut_("mesh")
    , seed_("seed", "Seed", 0, 0, std::mt19937::max())
    , reseed_("reseed_", "Seed")
    , scale_("scale", "Scale", 12.0f, 0.001f, 100.0f, 0.1f)
    , size_("size", "Size", 1.0f, 0.001f, 10.0f, 0.1f)
    , gridDim_("gridDim", "Grid Dimension", ivec3(12, 12, 12), ivec3(1), ivec3(128))
    , jigglePos_("jigglePos", "Jiggle Positions", true)
    , enablePicking_("enablePicking", "Enable Picking", false)
    , camera_("camera", "Camera", util::boundingBox(meshOut_))
    , spherePicking_(
          this, gridDim_.get().x * gridDim_.get().y * gridDim_.get().z, [&](PickingEvent* p) {
              handlePicking(p, [&](vec3 delta) {
                  if (positionBuffer_) {
                      auto& pos = positionBuffer_->getDataContainer();
                      pos[p->getPickedId()] += delta;
                      positionBuffer_->getOwner()->invalidateAllOther(positionBuffer_.get());
                  }
              });
          }) {
    addPort(meshOut_);

    addProperty(scale_);
    addProperty(size_);
    addProperty(gridDim_);
    addProperty(jigglePos_);

    addProperty(seed_);
    addProperty(reseed_);

    addProperty(enablePicking_);
    addProperty(camera_);
    camera_.setInvalidationLevel(InvalidationLevel::Valid);
    camera_.setCollapsed(true);

    seed_.setSemantics(PropertySemantics::Text);

    reseed_.onChange([&]() {
        seed_.set(static_cast<glm::i64>(rand(0.0f, static_cast<float>(seed_.getMaxValue()))));
        rand_.seed(static_cast<std::mt19937::result_type>(seed_.get()));
    });
}

void RandomSphereGenerator::process() {
    rand_.seed(static_cast<std::mt19937::result_type>(seed_.get()));

    const vec3 bboxMin(-scale_.get());
    const vec3 extent(scale_.get() * 2.0f);
    const vec3 delta(extent / vec3(gridDim_.get()));

    auto randPos = [&]() { return randVec3(-0.5f, 0.5f); };
    auto randColor = [&]() { return vec4(randVec3(0.5, 1.0), 1); };
    auto randRadius = [&]() { return size_.get() * rand(0.1f, 1.0f); };

    const bool dirty = seed_.isModified() || size_.isModified() || scale_.isModified() ||
                       enablePicking_.isModified();

    if (gridDim_.isModified() || dirty || !positionBuffer_) {
        const auto dim = gridDim_.get();
        const int numSpheres = dim.x * dim.y * dim.z;
        spherePicking_.resize(numSpheres);

        auto mesh = std::make_shared<Mesh>(DrawType::Points, ConnectivityType::None);

        auto vertexRAM = std::make_shared<BufferRAMPrecision<vec3>>(numSpheres);
        auto colorRAM = std::make_shared<BufferRAMPrecision<vec4>>(numSpheres);
        auto radiiRAM = std::make_shared<BufferRAMPrecision<float>>(numSpheres);

        // keep a reference to vertex position buffer for picking
        positionBuffer_ = vertexRAM;
        radiiBuffer_ = radiiRAM;

        mesh->addBuffer(Mesh::BufferInfo(BufferType::PositionAttrib),
                        std::make_shared<Buffer<vec3>>(vertexRAM));
        mesh->addBuffer(Mesh::BufferInfo(BufferType::ColorAttrib),
                        std::make_shared<Buffer<vec4>>(colorRAM));
        mesh->addBuffer(Mesh::BufferInfo(BufferType::RadiiAttrib),
                        std::make_shared<Buffer<float>>(radiiRAM));
        if (enablePicking_.get()) {
            auto pickingRAM = std::make_shared<BufferRAMPrecision<uint32_t>>(numSpheres);
            auto& data = pickingRAM->getDataContainer();
            // fill in picking IDs
            std::iota(data.begin(), data.end(),
                      static_cast<uint32_t>(spherePicking_.getPickingId(0)));

            mesh->addBuffer(Mesh::BufferInfo(BufferType::PickingAttrib),
                            std::make_shared<Buffer<uint32_t>>(pickingRAM));
        }

        auto& vertices = vertexRAM->getDataContainer();
        auto& colors = colorRAM->getDataContainer();
        auto& radii = radiiRAM->getDataContainer();

        util::IndexMapper<3, int> indexmapper(dim);
        if (jigglePos_.get()) {
            for (int i = 0; i < numSpheres; i++) {
                vertices[i] = (vec3(indexmapper(i)) + randPos()) * delta + bboxMin;
                colors[i] = randColor();
                radii[i] = randRadius();
            }
        } else {
            for (int i = 0; i < numSpheres; i++) {
                vertices[i] = vec3(indexmapper(i)) * delta + bboxMin;
                colors[i] = randColor();
                radii[i] = randRadius();
            }
        }
        meshOut_.setData(mesh);
    }
}

float RandomSphereGenerator::rand(const float min, const float max) const {
    return min + dis_(rand_) * (max - min);
}

vec3 RandomSphereGenerator::randVec3(const float min, const float max) const {
    float x = rand(min, max);
    float y = rand(min, max);
    float z = rand(min, max);
    return vec3(x, y, z);
}

vec3 RandomSphereGenerator::randVec3(const vec3& min, const vec3& max) const {
    float x = rand(min.x, max.x);
    float y = rand(min.y, max.y);
    float z = rand(min.z, max.z);
    return vec3(x, y, z);
}

void RandomSphereGenerator::handlePicking(PickingEvent* p, std::function<void(vec3)> callback) {
    if (enablePicking_) {
        if (p->getState() == PickingState::Updated &&
            p->getEvent()->hash() == MouseEvent::chash()) {
            auto me = p->getEventAs<MouseEvent>();
            if ((me->buttonState() & MouseButton::Left) && me->state() == MouseState::Move) {
                auto delta = getDelta(camera_, p);
                callback(delta);
                invalidate(InvalidationLevel::InvalidOutput);
                p->markAsUsed();
            }
        } else if (p->getState() == PickingState::Updated &&
                   p->getEvent()->hash() == TouchEvent::chash()) {

            auto te = p->getEventAs<TouchEvent>();
            if (!te->touchPoints().empty() && te->touchPoints()[0].state() == TouchState::Updated) {
                auto delta = getDelta(camera_, p);
                callback(delta);
                invalidate(InvalidationLevel::InvalidOutput);
                p->markAsUsed();
            }
        } else if (p->getState() == PickingState::Updated &&
                   p->getEvent()->hash() == WheelEvent::chash()) {
            auto we = p->getEventAs<WheelEvent>();
            if (radiiBuffer_) {
                auto& radii = radiiBuffer_->getDataContainer();

                auto radius =
                    radii[p->getPickedId()] * (1.0f - 0.05f * static_cast<float>(-we->delta().y));
                radii[p->getPickedId()] = glm::clamp(radius, scale_ * 0.05f, scale_ * 20.0f);

                radiiBuffer_->getOwner()->invalidateAllOther(radiiBuffer_.get());
            }

            invalidate(InvalidationLevel::InvalidOutput);
            p->markAsUsed();
        }
    }
}

vec3 RandomSphereGenerator::getDelta(const Camera& camera, PickingEvent* p) {
    auto currNDC = p->getNDC();
    auto prevNDC = p->getPreviousNDC();

    // Use depth of initial press as reference to move in the image plane.
    auto refDepth = p->getPressedDepth();
    currNDC.z = refDepth;
    prevNDC.z = refDepth;

    auto corrWorld = camera.getWorldPosFromNormalizedDeviceCoords(static_cast<vec3>(currNDC));
    auto prevWorld = camera.getWorldPosFromNormalizedDeviceCoords(static_cast<vec3>(prevNDC));
    return (corrWorld - prevWorld);
}

}  // namespace inviwo
