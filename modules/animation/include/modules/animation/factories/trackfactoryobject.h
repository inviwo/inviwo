/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2022 Inviwo Foundation
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

#include <modules/animation/animationmoduledefine.h>  // for IVW_MODULE_ANIMATION_API

#include <memory>                                     // for unique_ptr, make_unique
#include <string>                                     // for string

namespace inviwo {

class ProcessorNetwork;

namespace animation {
class Track;

class IVW_MODULE_ANIMATION_API TrackFactoryObject {
public:
    TrackFactoryObject(const std::string& classIdentifier);
    virtual ~TrackFactoryObject() = default;

    virtual std::unique_ptr<Track> create(ProcessorNetwork* network) const = 0;
    const std::string& getClassIdentifier() const;

protected:
    const std::string classIdentifier_;
};

template <typename T>
class TrackFactoryObjectTemplate : public TrackFactoryObject {
public:
    // Requires a static classIdentifier() method on T
    TrackFactoryObjectTemplate() : TrackFactoryObject(T::classIdentifier()) {}

    TrackFactoryObjectTemplate(const std::string& classIdentifier)
        : TrackFactoryObject(classIdentifier){};
    virtual ~TrackFactoryObjectTemplate() = default;

    virtual std::unique_ptr<Track> create(
        [[maybe_unused]] ProcessorNetwork* network) const override {

        if constexpr (std::is_constructible_v<T, ProcessorNetwork*>) {
            return std::make_unique<T>(network);
        } else {
            return std::make_unique<T>();
        }
    }
};

}  // namespace animation

}  // namespace inviwo
