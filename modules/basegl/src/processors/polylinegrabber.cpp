/*********************************************************************************
*
* Inviwo - Interactive Visualization Workshop
*
* Copyright (c) 2013-2018 Inviwo Foundation
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

#include <modules/basegl/processors/polylinegrabber.h>
#include <inviwo/core/interaction/events/mouseevent.h>

#include <fstream>
#include <limits>
#include <algorithm>

namespace inviwo {

    const ProcessorInfo PolylineGrabber::processorInfo_{
        "org.inviwo.PolylineGrabber", // Class identifier
        "Polyline Grabber",           // Display name
        "Input",                      // Category
        CodeState::Experimental,      // Code state
        Tags::CPU,                    // Tags
    };
    const ProcessorInfo PolylineGrabber::getProcessorInfo() const { return processorInfo_; }

    PolylineGrabber::PolylineGrabber()
        : Processor()
        , polyline_(std::make_shared<std::vector<vec3>>())
        , clearPolyline_("clearpolyline", "Clear Points")
        , numPolylinePts_("numPolylinePts", "Num. Points", 0)
        , clip_("clip", "Clip Polyline", 0.0f, 1.0f)
        , pt_("pt", "Point to Add")
        , addOrRemovePolylinePoint_("addOrRemovePolylinePoint", "Add or Remove Polyline Point",
            [this](Event* e) { processClickEvent(e); },
            MouseButton::Left | MouseButton::Right)
        , pointRemovalDistanceThreshold_("pointRemovalDistanceThreshold", "Point Removal Distance Threshold", 0.02f, 0.0f, 1.0f)
        , addCorridor_("add_corridor", "Add Corridor Points", false)
        , includeCorridorCenter_("include_corridor_center", "Include Corridor Center", false)
        , corridorWidth_("corridor_width", "Corridor Width", 0.01f, 0.001f, 1.0f, 0.001f)
        , corridorNormal_("corridor_normal", "Corridor Normal", vec3(0.0f, 0.0f, 1.0f), vec3(-1.0f), vec3(1.0f), vec3(1e-6f))
        , outport_("polylineport")
    {
        outport_.setData(polyline_);
        addPort(outport_);

        clearPolyline_.onChange([this]() {
            polyline_->clear();
            corridor_.clear();
            numPolylinePts_ = polyline_->size();
            invalidate(InvalidationLevel::InvalidOutput);
        });
        addProperty(clearPolyline_);

        numPolylinePts_.setReadOnly(true);
        numPolylinePts_.setSemantics(PropertySemantics::Text);
        addProperty(numPolylinePts_);

        clip_.onChange([this]() {
            const auto from =
                static_cast<size_t>(clip_.getStart() * static_cast<float>(polyline_->size()));
            const size_t to =
                static_cast<size_t>(clip_.getEnd() * static_cast<float>(polyline_->size()));
            std::vector<vec3>::const_iterator first = polyline_->begin() + from;
            std::vector<vec3>::const_iterator last = polyline_->begin() + to;
            auto newVec = std::make_shared<std::vector<vec3>>(first, last);
            outport_.setData(newVec);
            invalidate(InvalidationLevel::InvalidOutput);
        });
        addProperty(clip_);

        pt_.setReadOnly(true);
        addProperty(pt_);

        addProperty(addOrRemovePolylinePoint_);

        addProperty(pointRemovalDistanceThreshold_);

        addProperty(addCorridor_);
        addProperty(includeCorridorCenter_);
        addProperty(corridorWidth_);
        addProperty(corridorNormal_);

        //TODO smoothen the normal

        //TODO implement move and delete points
    }

    void PolylineGrabber::processClickEvent(Event* e)
    {
        const auto mouseEvent = static_cast<MouseEvent*>(e);
        const auto mousePos = vec2(mouseEvent->posNormalized());

        const auto button = mouseEvent->button();
        if (button == MouseButton::Left) {
            addPoint(pt_);
        } else if (button == MouseButton::Right) {
            removePoint(pt_);
        }

        numPolylinePts_ = polyline_->size();
    }

    void PolylineGrabber::addPoint(const vec3& pt)
    {
        // TODO: check if point is already in place
        if (addCorridor_) {
            corridor_.push_back(pt);

            // generate corridor if at least 2 pts are available
            if (corridor_.size() > 1) {
                const auto& curr = corridor_[corridor_.size() - 1]; // current pt
                const auto& prev = corridor_[corridor_.size() - 2]; // previous pt
                const auto diff = curr - prev;
                const auto dist = glm::dot(diff, corridorNormal_.get());
                const auto curr_projected = curr - dist * corridorNormal_.get();
                const auto dir_projected = glm::normalize(curr_projected - prev);
                const auto offset = corridorWidth_.get() * glm::rotate(dir_projected, glm::half_pi<float>(), corridorNormal_.get()); // offset rotated by 90 degrees

                // add previous pt in case of start of corridor
                if (corridor_.size() == 2) {
                    if (includeCorridorCenter_) {
                        polyline_->push_back(prev);
                    }
                    polyline_->push_back(prev + offset);
                    polyline_->push_back(prev - offset);
                }

                // add current pt
                if (includeCorridorCenter_) {
                    polyline_->push_back(curr);
                }
                polyline_->push_back(curr + offset);
                polyline_->push_back(curr - offset);
            }
        }
        else {
            polyline_->push_back(pt);

            // reset corridor if normal points are added
            corridor_.clear();
            // add last point for initial corridor
            corridor_.push_back(pt);
        }

        invalidate(InvalidationLevel::InvalidOutput);
    }

    void PolylineGrabber::removePoint(const vec3& pt) {
        if (!polyline_->empty()) {
            size_t min_idx{0};
            float min_dist{std::numeric_limits<float>::infinity()};
            for (size_t idx = 0; idx < polyline_->size(); ++idx) {
                // only in same slice
                const float dist = glm::distance(polyline_->at(idx), pt);
                if (dist < min_dist) {
                    min_dist = dist;
                    min_idx = idx;
                }
            }

            if (min_dist <= pointRemovalDistanceThreshold_) {
                polyline_->erase(polyline_->begin() + min_idx);
            }
        }

        invalidate(InvalidationLevel::InvalidOutput);
    }

    std::string PolylineGrabber::createPointSerializationName(size_t idx) const {
        return std::string("point") + std::to_string(idx);
    }

    void PolylineGrabber::serialize(Serializer& s) const {
        Processor::serialize(s);

        // save the number of points
        s.serialize("num_pts", polyline_->size());

        // save all points
        for (size_t idx = 0; idx < polyline_->size(); ++idx) {
            s.serialize(createPointSerializationName(idx), polyline_->at(idx));
        }
    }

    void PolylineGrabber::deserialize(Deserializer& d) {
        // load the number of total points
        size_t num_pts{0};
        d.deserialize("num_pts", num_pts);

        // load all points
        polyline_->resize(num_pts);
        for (size_t idx = 0; idx < num_pts; ++idx) {
            d.deserialize(createPointSerializationName(idx), polyline_->at(idx));
        }

        numPolylinePts_ = num_pts;

        Processor::deserialize(d);
    }

}  // namespace inviwo
