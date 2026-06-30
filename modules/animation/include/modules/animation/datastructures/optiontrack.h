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
#pragma once

#include <modules/animation/animationmoduledefine.h>

#include <inviwo/core/properties/optionproperty.h>

#include <modules/animation/datastructures/animationstate.h>
#include <modules/animation/datastructures/animationtime.h>
#include <modules/animation/datastructures/basetrack.h>
#include <modules/animation/datastructures/optionkeyframe.h>
#include <modules/animation/datastructures/optionkeyframesequence.h>
#include <modules/animation/datastructures/propertytrack.h>
#include <modules/animation/interpolation/interpolation.h>

#include <memory>
#include <string_view>

namespace inviwo {

class Deserializer;
class ProcessorNetwork;
class Property;
class Serializer;

namespace animation {

class Keyframe;
class Track;

/**
 * A Track for animating any OptionProperty<T> through its BaseOptionProperty interface.
 *
 * Unlike PropertyTrack, which stores the selected value and therefore has to be registered for
 * each value type @c T, OptionTrack stores the selected @em index. The selected index is the only
 * thing needed from the option property and is available through the non-templated
 * BaseOptionProperty base class. This allows a single OptionTrack to animate any OptionProperty,
 * regardless of its value type, including option properties with custom or unregistered enum value
 * types.
 *
 * The index is held constant between keyframes since option properties only have discrete states.
 *
 * @see BaseOptionProperty
 * @see OptionKeyframe
 * @see OptionKeyframeSequence
 * @see PropertyTrack
 */
class IVW_MODULE_ANIMATION_API OptionTrack : public BaseTrack<OptionKeyframeSequence>,
                                             public BasePropertyTrack {
public:
    OptionTrack(ProcessorNetwork* network);
    OptionTrack(BaseOptionProperty* property);
    OptionTrack(BaseOptionProperty* property, ProcessorNetwork* network);

    /**
     * Remove all keyframe sequences and call TrackObserver::notifyKeyframeSequenceRemoved
     */
    virtual ~OptionTrack();

    virtual OptionTrack* clone() const override;

    static constexpr std::string_view classIdentifier() {
        return "org.inviwo.animation.OptionTrack";
    }
    virtual std::string_view getClassIdentifier() const override;

    virtual AnimationTimeState operator()(Seconds from, Seconds to,
                                          AnimationState state) const override;

    virtual const BaseOptionProperty* getProperty() const override;
    virtual BaseOptionProperty* getProperty() override;
    virtual void setProperty(Property* property) override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    virtual OptionKeyframe* addKeyFrameUsingPropertyValue(
        const Property* property, Seconds time,
        std::unique_ptr<Interpolation> interpolation = nullptr) override;
    virtual OptionKeyframe* addKeyFrameUsingPropertyValue(
        Seconds time, std::unique_ptr<Interpolation> interpolation = nullptr) override;
    virtual OptionKeyframeSequence* addSequenceUsingPropertyValue(
        Seconds time, std::unique_ptr<Interpolation> interpolation = nullptr) override;

    // BasePropertyTrack overload
    virtual Track* toTrack() override;

    /**
     * @brief Helper function to set a property from a keyframe
     *
     * Called from inviwo::animation::KeyframeEditorWidget when creating the widget
     *
     * @param dstProperty The option property to set, must be a BaseOptionProperty
     * @param keyframe The keyframe to set from
     */
    void setPropertyFromKeyframe(Property* dstProperty, const Keyframe* keyframe) const override;

    /**
     * @brief Helper function to update the index of a keyframe from a property (other than the
     * property owned by the track)
     *
     * Called from inviwo::animation::KeyframeEditorWidget when the value of the keyframe is updated
     * through the widget
     *
     * @param srcProperty The option property to set from, must be a BaseOptionProperty
     * @param keyframe The keyframe to set
     */
    void setKeyframeFromProperty(const Property* srcProperty, Keyframe* keyframe) override;

    /*
     * Create a Keyframe using the current selected index of the property.
     */
    virtual std::unique_ptr<OptionKeyframe> createKeyframe(Seconds time) const override;

protected:
    OptionTrack(const OptionTrack& other) = default;
    OptionTrack(OptionTrack&& other) = default;

    /*
     * Create a Keyframe using the selected index of the provided property.
     */
    std::unique_ptr<OptionKeyframe> createKeyframe(const BaseOptionProperty* property,
                                                   Seconds time) const;

private:
    BaseOptionProperty* property_;  ///< non-owning reference
    ProcessorNetwork* network_;
};

}  // namespace animation

}  // namespace inviwo
