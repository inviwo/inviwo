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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/observer.h>
#include <inviwo/core/util/glmvec.h>

#include <optional>
#include <string>
#include <vector>
#include <span>

namespace inviwo {

class Processor;
class Serializer;
class Deserializer;

struct IVW_CORE_API Description {
    enum class TextAlignment : std::uint8_t { Left, Top, Right, Bottom };

    std::optional<std::string> markdown;
    TextAlignment alignment = TextAlignment::Left;
    int width = 100;
};

struct IVW_CORE_API NetworkAnnotation {
    std::vector<std::string> processors;
    std::optional<std::string> title;
    Description description;
    vec3 color;
};

class IVW_CORE_API NetworkAnnotationsObserver : public Observer {
public:
    virtual void onNetworkAnnotationAdded(NetworkAnnotation&, size_t) {}
    virtual void onNetworkAnnotationWillBeRemoved(NetworkAnnotation&, size_t) {}
    virtual void onNetworkAnnotationChanged(NetworkAnnotation&, size_t) {}
};

class IVW_CORE_API NetworkAnnotations : public Observable<NetworkAnnotationsObserver> {
public:
    void serialize(Serializer& s) const;
    void deserialize(Deserializer& d);

    size_t add(std::span<const Processor* const> processors);
    size_t add(NetworkAnnotation&& annotation);
    void remove(size_t index);
    void clear();

    size_t size() const;

    void removeProcessor(Processor* processor);
    bool matches(size_t index, const std::vector<Processor*>& processors) const;

    NetworkAnnotation& getAnnotation(size_t index);
    const NetworkAnnotation& getAnnotation(size_t index) const;

    void setModified(size_t index);

private:
    std::vector<NetworkAnnotation> annotations_;
};

}  // namespace inviwo
