/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

#ifndef IVW_CONTROLKEYFRAMESEQUENCE_H
#define IVW_CONTROLKEYFRAMESEQUENCE_H

#include <modules/animation/animationmoduledefine.h>

#include <modules/animation/datastructures/keyframesequence.h>
#include <modules/animation/datastructures/controlkeyframe.h>

namespace inviwo {

namespace animation {
	
/** \class ControlKeyframeSequence
* KeyframeSequence for Control Keyframes.
* @see KeyframeSequence
*/
class IVW_MODULE_ANIMATION_API ControlKeyframeSequence : public KeyframeSequence {
public:
	ControlKeyframeSequence();

	ControlKeyframeSequence(const std::vector<ControlKeyframe>& keyframes);
	ControlKeyframeSequence(const ControlKeyframeSequence& rhs);
	ControlKeyframeSequence& operator=(const ControlKeyframeSequence& that);
	/**
	* Remove all keyframes and call KeyframeObserver::notifyKeyframeRemoved
	*/
	virtual ~ControlKeyframeSequence();
	/**
	* Return number of keyframes in the sequence.
	*/
	virtual size_t size() const override { return keyframes_.size(); }

	virtual const ControlKeyframe& operator[](size_t i) const override;
	virtual ControlKeyframe& operator[](size_t i) override;

	virtual const ControlKeyframe& getFirst() const override;
	virtual ControlKeyframe& getFirst() override;
	virtual const ControlKeyframe& getLast() const override;
	virtual ControlKeyframe& getLast() override;
	/**
	* Remove Keyframe and call KeyframeObserver::notifyKeyframeRemoved
	*/
	virtual void remove(size_t i) override;
	/**
	* Add Keyframe and call KeyframeObserver::notifyKeyframeAdded
	*/
	virtual void add(const Keyframe& key) override;
	/**
	* Add Keyframe and call KeyframeObserver::notifyKeyframeAdded
	*/
	void add(const ControlKeyframe& key);

	virtual void setInterpolation(std::unique_ptr<Interpolation> interpolation) override {};

	virtual AnimationTimeState operator()(Seconds from, Seconds to, AnimationState state) const;

	virtual void serialize(Serializer& s) const override;
	virtual void deserialize(Deserializer& d) override;

private:
	void addKeyFrame(std::unique_ptr<ControlKeyframe> key);

	virtual void onKeyframeTimeChanged(Keyframe* key, Seconds oldTime) override;

	std::vector<std::unique_ptr<ControlKeyframe>> keyframes_;
};

}  // namespace

}  // namespace

#endif  // IVW_CONTROLKEYFRAMESEQUENCE_H
