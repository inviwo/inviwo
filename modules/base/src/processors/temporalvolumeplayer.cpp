/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/base/processors/temporalvolumeplayer.h>

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/util/indexmapper.h>

#include <glm/common.hpp>

#include <memory>

namespace inviwo {

const ProcessorInfo TemporalVolumePlayer::processorInfo_{
    "org.inviwo.TemporalVolumePlayer",  // Class identifier
    "Temporal Volume Player",           // Display name
    "Volume Operation",                 // Category
    CodeState::Experimental,            // Code state
    Tags::CPU | Tag{"Volume"} | Tag{"Temporal"},
    R"(Resolves the current time of a TemporalVolume into a single Volume, optionally linearly
    interpolating between adjacent frames. Schedules background prefetching of upcoming frames for
    smooth playback.)"_unindentHelp,
};

const ProcessorInfo& TemporalVolumePlayer::getProcessorInfo() const { return processorInfo_; }

namespace {

// Per-voxel linear interpolation between two volumes. Returns nullptr if the volumes are not
// compatible (different dimensions or format), in which case the caller should fall back to the
// nearest frame.
std::shared_ptr<Volume> lerpVolumes(const Volume& a, const Volume& b, double t) {
    if (a.getDimensions() != b.getDimensions() || a.getDataFormat() != b.getDataFormat()) {
        return nullptr;
    }

    std::shared_ptr<Volume> result(a.clone());
    auto* dst = result->getEditableRepresentation<VolumeRAM>();
    const auto* src = b.getRepresentation<VolumeRAM>();

    const size3_t dims = result->getDimensions();
    for (size_t z = 0; z < dims.z; ++z) {
        for (size_t y = 0; y < dims.y; ++y) {
            for (size_t x = 0; x < dims.x; ++x) {
                const size3_t pos{x, y, z};
                dst->setFromDVec4(pos, glm::mix(dst->getAsDVec4(pos), src->getAsDVec4(pos), t));
            }
        }
    }
    return result;
}

}  // namespace

TemporalVolumePlayer::TemporalVolumePlayer()
    : Processor()
    , inport_{"inputVolume", "The input temporal volume"_help}
    , outport_{"outputVolume", "The resolved volume for the current time"_help}
    , time_{"time", "Time",
            util::ordinalCount(0.0, 1.0).set("The time to sample the temporal volume at"_help)}
    , interpolation_{"interpolation",
                     "Interpolation",
                     "How to sample between adjacent frames"_help,
                     {{"nearest", "Nearest", Interpolation::Nearest},
                      {"linear", "Linear", Interpolation::Linear}},
                     1}
    , prefetch_{"prefetch", "Prefetch", "Schedule background loading of upcoming frames"_help, true}
    , prefetchAhead_{
          "prefetchAhead", "Prefetch Ahead",
          util::ordinalCount<size_t>(2u, 16u).set("Number of upcoming frames to prefetch"_help)} {

    addPort(inport_);
    addPort(outport_);
    addProperties(time_, interpolation_, prefetch_, prefetchAhead_);
}

void TemporalVolumePlayer::process() {
    auto temporal = inport_.getData();
    if (!temporal || temporal->empty()) {
        outport_.clear();
        return;
    }

    if (inport_.isChanged()) {
        const auto [first, last] = temporal->timeRange();
        time_.setMinValue(first.count());
        time_.setMaxValue(last.count());
    }

    const Seconds t{time_.get()};

    if (interpolation_.get() == Interpolation::Linear) {
        auto frame = temporal->interpolate(t);
        if (frame.a && frame.b && frame.a != frame.b && frame.t > 0.0) {
            if (auto blended = lerpVolumes(*frame.a, *frame.b, frame.t)) {
                outport_.setData(blended);
            } else {
                // Incompatible frames, fall back to the nearest one.
                outport_.setData(frame.t < 0.5 ? frame.a : frame.b);
            }
        } else if (frame.a) {
            outport_.setData(frame.a);
        } else {
            outport_.clear();
        }
    } else {
        outport_.setData(temporal->get(t));
    }

    if (prefetch_.get()) {
        const size_t current = temporal->nearestIndex(t);
        temporal->prefetch(current + 1, prefetchAhead_.get());
    }
}

}  // namespace inviwo
