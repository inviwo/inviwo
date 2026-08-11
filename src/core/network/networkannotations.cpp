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

#include <inviwo/core/network/networkannotations.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/io/serialization/serializer.h>
#include <inviwo/core/io/serialization/deserializer.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/colorconversion.h>
#include <inviwo/core/util/zip.h>

#include <random>
#include <fmt/std.h>

namespace inviwo {

namespace {

vec3 randomColor() {
    static std::mt19937 generator{std::random_device{}()};
    static std::uniform_real_distribution<float> distribution{0.0, 1.0};

    return color::hsv2rgb({distribution(generator), 0.8f, 0.92f});
}

}  // namespace

void NetworkAnnotations::serialize(Serializer& s) const {
    s.serializeRange("NetworkAnnotations", "Annotation", annotations_,
                     [](Serializer& nested, const NetworkAnnotation& a) {
                         if (a.title.has_value()) {
                             nested.serialize("title", *a.title, SerializationTarget::Attribute);
                         }
                         if (a.description.markdown.has_value()) {
                             nested.serialize("markdown", *a.description.markdown);
                         }
                         nested.serialize("Processors", a.processors, "item");
                         nested.serialize("color", a.color);
                         nested.serialize("alignment", a.description.alignment);
                         nested.serialize("width", a.description.width);
                     });
}

void NetworkAnnotations::deserialize(Deserializer& d) {
    clear();

    try {
        d.deserializeRange("NetworkAnnotations", "Annotation",
                           [this](Deserializer& nested, [[maybe_unused]] size_t index) {
                               NetworkAnnotation annotation;

                               if (auto title = nested.attribute("title"); title.has_value()) {
                                   annotation.title = title;
                               }
                               if (nested.hasElement("markdown")) {
                                   std::string value;
                                   nested.deserialize("markdown", value);
                                   annotation.description.markdown = value;
                               }
                               nested.deserialize("color", annotation.color);
                               nested.deserialize("alignment", annotation.description.alignment);
                               nested.deserialize("width", annotation.description.width);

                               std::pmr::vector<std::pmr::string> processorIds;
                               nested.deserialize("Processors", processorIds, "item");
                               annotation.processors.assign(processorIds.begin(),
                                                            processorIds.end());

                               annotations_.push_back(annotation);
                           });

    } catch (const Exception& exception) {
        clear();
        throw AbortException(exception.getContext(), "Deserialization error: {}",
                             exception.getMessage());
    } catch (const std::exception& exception) {
        clear();
        throw AbortException(SourceContext{}, "Deserialization error: {}", exception.what());
    } catch (...) {
        clear();
        throw AbortException("Unknown Exception during deserialization.");
    }

    for (auto&& [index, annotation] : util::enumerate(annotations_)) {
        Observable<NetworkAnnotationsObserver>::forEachObserver(
            [&](NetworkAnnotationsObserver* o) { o->onNetworkAnnotationAdded(annotation, index); });
    }
}

size_t NetworkAnnotations::add(std::span<const Processor* const> processors) {
    const size_t index = annotations_.size();
    annotations_.emplace_back(NetworkAnnotation{
        .processors = processors |
                      std::views::transform([](auto* p) { return p->getIdentifier(); }) |
                      std::ranges::to<std::vector<std::string>>(),
        .color = randomColor(),
    });
    Observable<NetworkAnnotationsObserver>::forEachObserver(
        [this, index](NetworkAnnotationsObserver* o) {
            o->onNetworkAnnotationAdded(annotations_.back(), index);
        });
    return index;
}

size_t NetworkAnnotations::add(NetworkAnnotation&& annotation) {
    const size_t index = annotations_.size();
    annotations_.push_back(std::move(annotation));
    Observable<NetworkAnnotationsObserver>::forEachObserver(
        [this, index](NetworkAnnotationsObserver* o) {
            o->onNetworkAnnotationAdded(annotations_.back(), index);
        });
    return index;
}

void NetworkAnnotations::remove(size_t index) {
    if (index >= annotations_.size()) {
        throw RangeException{SourceContext{}, "index out or range {}", index};
    }
    Observable<NetworkAnnotationsObserver>::forEachObserver(
        [this, index](NetworkAnnotationsObserver* o) {
            o->onNetworkAnnotationWillBeRemoved(annotations_[index], index);
        });
    annotations_.erase(annotations_.begin() + index);
}

void NetworkAnnotations::clear() {
    for (auto&& [index, annotation] : util::enumerate(annotations_)) {
        Observable<NetworkAnnotationsObserver>::forEachObserver([&](NetworkAnnotationsObserver* o) {
            o->onNetworkAnnotationWillBeRemoved(annotation, index);
        });
    }
    annotations_.clear();
}

size_t NetworkAnnotations::size() const { return annotations_.size(); }

void NetworkAnnotations::removeProcessor(Processor* processor) {
    for (auto&& [index, annotation] : util::enumerate(annotations_)) {
        if (auto count = std::erase(annotation.processors, processor->getIdentifier())) {
            Observable<NetworkAnnotationsObserver>::forEachObserver(
                [&](NetworkAnnotationsObserver* o) {
                    o->onNetworkAnnotationChanged(annotation, index);
                });
        }
    }
}

bool NetworkAnnotations::matches(size_t index, const std::vector<Processor*>& processors) const {
    return std::ranges::all_of(getAnnotation(index).processors, [&](const auto& id) {
        return std::ranges::contains(processors, id,
                                     [](const Processor* p) { return p->getIdentifier(); });
    });
}

NetworkAnnotation& NetworkAnnotations::getAnnotation(size_t index) {
    if (index >= annotations_.size()) {
        throw RangeException{SourceContext{}, "index out or range {}", index};
    }
    return annotations_[index];
}

const NetworkAnnotation& NetworkAnnotations::getAnnotation(size_t index) const {
    if (index >= annotations_.size()) {
        throw RangeException{SourceContext{}, "index out or range {}", index};
    }
    return annotations_[index];
}

void NetworkAnnotations::setModified(size_t index) {
    Observable<NetworkAnnotationsObserver>::forEachObserver(
        [this, index](NetworkAnnotationsObserver* o) {
            o->onNetworkAnnotationChanged(annotations_[index], index);
        });
}

}  // namespace inviwo
