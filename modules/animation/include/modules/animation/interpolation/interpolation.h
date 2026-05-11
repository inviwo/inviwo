/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2026 Inviwo Foundation
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

#include <inviwo/core/properties/propertyowner.h>
#include <modules/animation/datastructures/animationtime.h>

#include <memory>
#include <string>
#include <string_view>

namespace inviwo {
class InviwoApplication;

namespace animation {

/**
 * Interface for keyframe interpolations.
 * Inherits PropertyOwner so that subclasses can expose their options as standard Inviwo
 * properties, gaining serialization and GUI rendering for free.
 */
class IVW_MODULE_ANIMATION_API Interpolation : public PropertyOwner {
public:
    /**
     * @param app  The InviwoApplication. May be nullptr if created outside of a full application
     *             context (e.g. for temporary introspection). The InterpolationFactory always
     *             supplies the real application pointer.
     */
    Interpolation(InviwoApplication* app);
    virtual ~Interpolation() = default;

    virtual Interpolation* clone() const = 0;

    virtual std::string_view getDisplayName() const = 0;
    virtual std::string_view getClassIdentifier() const = 0;

    virtual bool equal(const Interpolation& other) const = 0;

    virtual InviwoApplication* getInviwoApplication() override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

protected:
    /**
     * Protected copy constructor for use by clone() in derived classes.
     * Re-populating the property list is the responsibility of the derived class.
     */
    Interpolation(const Interpolation& rhs);
    InviwoApplication* app_;
};

IVW_MODULE_ANIMATION_API bool operator==(const Interpolation& a, const Interpolation& b);
IVW_MODULE_ANIMATION_API bool operator!=(const Interpolation& a, const Interpolation& b);

/**
 *	Base class for interpolation between key frames.
 *  Interpolation will always be performed between at least two key frames.
 *
 *  @see KeyFrame
 *  @see KeyFrameSequence
 */
template <typename Key, typename Result = typename Key::value_type>
class InterpolationTyped : public Interpolation {
public:
    using key_type = Key;
    using result_type = Result;

    using Interpolation::Interpolation;

    virtual InterpolationTyped* clone() const override = 0;  // Needed for covariant return type

    // Override this function to interpolate between key frames
    virtual void operator()(const std::vector<std::unique_ptr<Key>>& keys, Seconds from, Seconds to,
                            Result& out) const = 0;

protected:
    InterpolationTyped(const InterpolationTyped& rhs) : Interpolation(rhs) {}
};

}  // namespace animation

}  // namespace inviwo
