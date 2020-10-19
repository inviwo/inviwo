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
#pragma once

#include <modules/animation/animationmoduledefine.h>
#include <modules/animation/datastructures/camerakeyframesequence.h>
#include <modules/animation/datastructures/propertytrack.h>
#include <inviwo/core/properties/cameraproperty.h>



namespace inviwo {

namespace animation {

/** \class CameraTrack
 * Implementation of BaseCameraTrack and TrackTyped based on templates parameter types for
 * Property and KeyFrame.
 * Exposes functions for adding a KeyFrame and KeyFrameSequence
 * using the current values of the Property.
 * @see Track
 * @see CameraTrack
 * @see Property
 */
class IVW_MODULE_ANIMATION_API CameraTrack : public BaseTrack<CameraKeyframeSequence>, public BasePropertyTrack {
public:
    
    CameraTrack();
    CameraTrack(CameraProperty* property);
    /**
     * Remove all keyframe sequences and call TrackObserver::notifyKeyframeSequenceRemoved
     */
    virtual ~CameraTrack();

    static std::string classIdentifier();
    virtual std::string getClassIdentifier() const override;

    virtual AnimationTimeState operator()(Seconds from, Seconds to,
                                          AnimationState state) const override;

    virtual const CameraProperty* getProperty() const override;
    virtual CameraProperty* getProperty() override;
    virtual void setProperty(Property* property) override;
    virtual const std::string& getIdentifier() const override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    virtual Keyframe* addKeyFrameUsingPropertyValue(
        const Property* property, Seconds time,
        std::unique_ptr<Interpolation> interpolation) override;
    virtual Keyframe* addKeyFrameUsingPropertyValue(
        Seconds time, std::unique_ptr<Interpolation> interpolation) override;
    virtual KeyframeSequence* addSequenceUsingPropertyValue(
        Seconds time, std::unique_ptr<Interpolation> interpolation) override;

    // BaseCameraTrack overload
    virtual Track* toTrack() override;

    /**
     * \brief Helper function to set a property (other than the property owned by the track) from a
     * keyframe
     *
     * Called from inviwo::animation::KeyframeEditorWidget when creating the widget
     *
     * @param dstProperty The property to set
     * @param keyframe The keyframe to set from
     */
    void setOtherProperty(Property* dstProperty, Keyframe* keyframe) override {
        IVW_ASSERT(dstProperty->getClassIdentifier() == PropertyTraits<CameraProperty>::classIdentifier(),
                   "Incorrect Property type");
        static_cast<CameraProperty*>(dstProperty)->setCamera(std::unique_ptr<Camera>(static_cast<CameraKeyframe*>(keyframe)->getValue().clone()));
    }

    /**
     * \brief Helper function to update the value if a keyframe from a property (other than the
     * property owned by the track)
     *
     * Called from inviwo::animation::KeyframeEditorWidget when the value of the keyframe is updated
     * through the widget
     *
     * @param srcProperty The property to set from
     * @param keyframe The keyframe to set
     */
    void updateKeyframeFromProperty(Property* srcProperty, Keyframe* keyframe) override {
        ivwAssert(srcProperty->getClassIdentifier() == PropertyTraits<CameraProperty>::classIdentifier(),
                  "Incorrect Property type");
        static_cast<CameraKeyframe*>(keyframe)->updateFrom(static_cast<CameraProperty*>(srcProperty)->get());
    }

private:
    CameraProperty* property_;  ///< non-owning reference
};


}  // namespace animation

}  // namespace inviwo
