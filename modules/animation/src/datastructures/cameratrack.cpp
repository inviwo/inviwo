/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <modules/animation/datastructures/cameratrack.h>


namespace inviwo {

namespace animation {

CameraTrack::CameraTrack()
    : BaseTrack<CameraKeyframeSequence>{"", "", 100}, property_(nullptr) {}


CameraTrack::CameraTrack(CameraProperty* property)
    : BaseTrack<CameraKeyframeSequence>{property->getIdentifier(), property->getDisplayName(),
                                            100}
    , property_(property) {}


CameraTrack::~CameraTrack() = default;


bool operator==(const CameraTrack& a, const CameraTrack& b) {
    return std::equal(a.begin(), a.end(), a.begin(), b.end());
}

bool operator!=(const CameraTrack& a, const CameraTrack& b) {
    return !(a == b);
}

Track* CameraTrack::toTrack() {
    return this;
}



std::string CameraTrack::classIdentifier() {
    // Use property class identifier since multiple properties
    // may have the same key (data type)
    std::string id =
        "org.inviwo.animation.PropertyTrack.for. " + PropertyTraits<CameraProperty>::classIdentifier();
    return id;
}


const std::string& CameraTrack::getIdentifier() const {
    return BaseTrack<CameraKeyframeSequence>::getIdentifier();
}


std::string CameraTrack::getClassIdentifier() const {
    return classIdentifier();
}


CameraProperty* CameraTrack::getProperty() {
    return property_;
}


const CameraProperty* CameraTrack::getProperty() const {
    return property_;
}


void CameraTrack::setProperty(Property* property) {
    if (auto prop = dynamic_cast<CameraProperty*>(property)) {
        property_ = prop;
        this->setIdentifier(property_->getIdentifier());
        this->setName(property_->getDisplayName());
    } else {
        throw Exception("Invalid property set to track", IVW_CONTEXT);
    }
}

/**
 * Track of sequences
 * ----------X======X====X-----------X=========X-------X=====X--------
 * |- case 1-|-case 2----------------|-case 2----------|-case 2------|
 *           |-case 2a---|-case 2b---|
 */

AnimationTimeState CameraTrack::operator()(Seconds from, Seconds to,
                                                        AnimationState state) const {
    if (!this->isEnabled() || this->empty()) return {to, state};

    // 'it' will be the first seq. with a first time larger then 'to'.
    auto it = std::upper_bound(this->begin(), this->end(), to,
                               [](const auto& a, const auto& b) { return a < b; });

    if (it == this->begin()) {
        if (from > it->getFirstTime()) {  // case 1
            const auto& cam = it->getFirst().getValue();
            // Do not update aspect ratio, e.g. updateFrom(Camera&)
            property_->setLook(cam.getLookFrom(), cam.getLookTo(), cam.getLookUp());
        }
    } else {  // case 2
        auto& seq1 = *std::prev(it);

        if (to < seq1.getLastTime()) {  // case 2a
            seq1(from, to, property_->get());
        } else {  // case 2b
            if (from < seq1.getLastTime()) {
                // We came from before the previous key
                const auto& cam = seq1.getLast().getValue();
                // Do not update aspect ratio, e.g. updateFrom(Camera&)
                property_->setLook(cam.getLookFrom(), cam.getLookTo(), cam.getLookUp());
            } else if (it != this->end() && from > it->getFirstTime()) {
                // We came form after the next key
                const auto& cam = it->getFirst().getValue();
                // Do not update aspect ratio, e.g. updateFrom(Camera&)
                property_->setLook(cam.getLookFrom(), cam.getLookTo(), cam.getLookUp());
            }
            // we moved in an unmarked region, do nothing.
        }
    }
    return {to, state};
}


Keyframe* CameraTrack::addKeyFrameUsingPropertyValue(
    const Property* property, Seconds time, std::unique_ptr<Interpolation> interpolation) {
    auto prop = dynamic_cast<const CameraProperty*>(property);
    if (!prop) {
        throw Exception("Cannot add key frame from property type " +
                            property->getClassIdentifier() + " for " +
                            property_->getClassIdentifier(),
                        IVW_CONTEXT);
    }
    if (this->empty()) {
        // Use provided interpolation if we can
        if (auto ip = dynamic_cast<CameraInterpolation*>(interpolation.get())) {
            interpolation.release();

            std::vector<std::unique_ptr<CameraKeyframe>> keys;
            keys.push_back(std::make_unique<CameraKeyframe>(time, prop->get()));
            auto sequence = std::make_unique<CameraKeyframeSequence>(
                std::move(keys), std::unique_ptr<CameraInterpolation>(ip));
            if (auto se = add(std::move(sequence))) {
                &se->getFirst();
            }
        } else {
            throw Exception("Invalid interpolation " + interpolation->getClassIdentifier() +
                                " for " + getClassIdentifier(),
                            IVW_CONTEXT);
        }

    } else {
        return this->addToClosestSequence(std::make_unique<CameraKeyframe>(time, prop->get()));
    }
    return nullptr;
}

Keyframe* CameraTrack::addKeyFrameUsingPropertyValue(
    Seconds time, std::unique_ptr<Interpolation> interpolation) {
    return addKeyFrameUsingPropertyValue(property_, time, std::move(interpolation));
}


KeyframeSequence* CameraTrack::addSequenceUsingPropertyValue(
    Seconds time, std::unique_ptr<Interpolation> interpolation) {
    if (auto ip = dynamic_cast<CameraInterpolation*>(interpolation.get())) {
        interpolation.release();

        std::vector<std::unique_ptr<CameraKeyframe>> keys;
        keys.push_back(std::make_unique<CameraKeyframe>(time, property_->get()));
        auto sequence = std::make_unique<CameraKeyframeSequence>(
            std::move(keys), std::unique_ptr<CameraInterpolation>(ip));
        return this->add(std::move(sequence));

    } else {
        throw Exception("Invalid interpolation " + interpolation->getClassIdentifier() + " for " +
                            getClassIdentifier(),
                        IVW_CONTEXT);
    }
    return nullptr;
}


void CameraTrack::serialize(Serializer& s) const {
    BaseTrack<CameraKeyframeSequence>::serialize(s);
    s.serialize("property", property_);
}


void CameraTrack::deserialize(Deserializer& d) {
    BaseTrack<CameraKeyframeSequence>::deserialize(d);
    d.deserializeAs<CameraProperty>("property", property_);
}
}  // namespace animation

}  // namespace inviwo
