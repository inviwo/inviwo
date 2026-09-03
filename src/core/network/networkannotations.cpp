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
#include <inviwo/core/network/workspacemanager.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/io/serialization/serializer.h>
#include <inviwo/core/io/serialization/deserializer.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/colorconversion.h>
#include <inviwo/core/util/zip.h>

#include <random>
#include <algorithm>

namespace inviwo {

namespace {

vec3 randomColor() {
    static std::mt19937 generator{std::random_device{}()};
    static std::uniform_real_distribution<float> distribution{0.0, 1.0};

    return color::hsv2rgb({distribution(generator), 0.8f, 0.92f});
}

}  // namespace

void Description::serialize(Serializer& s) const {
    s.serialize("markdown", markdown);
    s.serialize("alignment", alignment);
    s.serialize("width", width);
}

void Description::deserialize(Deserializer& d) {
    d.deserialize("markdown", markdown);
    d.deserialize("alignment", alignment);
    d.deserialize("width", width);
}

void NetworkAnnotation::serialize(Serializer& s) const {
    if (title.has_value()) {
        s.serialize("title", *title, SerializationTarget::Attribute);
    }
    if (description.has_value()) {
        s.serialize("Description", *description);
    }
    s.serialize("Processors", processors, "item");
    s.serialize("color", color);
}

void NetworkAnnotation::deserialize(Deserializer& d) {
    if (auto value = d.attribute("title"); value.has_value()) {
        title = value;
    } else {
        title.reset();
    }
    if (d.hasElement("Description")) {
        description.emplace();
        d.deserialize("Description", description.value());
    } else {
        description.reset();
    }
    d.deserialize("color", color);
    d.deserialize("Processors", processors, "item");
}

void NetworkAnnotation::addProcessors(const std::vector<Processor*>& selected) {
    const auto dist = std::distance(processors.begin(), processors.end());
    for (auto* p : selected) {
        if (!std::ranges::contains(processors.begin(), processors.begin() + dist,
                                   p->getIdentifier())) {
            processors.emplace_back(p->getIdentifier());
        }
    }
}

void NetworkAnnotation::removeProcessors(const std::vector<Processor*>& selected) {
    for (auto* p : selected) {
        if (auto it = std::ranges::find(processors, p->getIdentifier()); it != processors.end()) {
            processors.erase(it);
        }
    }
}

void NetworkAnnotationsObservable::notifyObserversAnnotationAdded(NetworkAnnotation& annotation,
                                                                  size_t index) {
    forEachObserver([&annotation, index](NetworkAnnotationsObserver* o) {
        o->onNetworkAnnotationAdded(annotation, index);
    });
}

void NetworkAnnotationsObservable::notifyObserversAnnotationWasRemoved(
    NetworkAnnotation& annotation, size_t index) {
    forEachObserver([&annotation, index](NetworkAnnotationsObserver* o) {
        o->onNetworkAnnotationWasRemoved(annotation, index);
    });
}

void NetworkAnnotationsObservable::notifyObserversAnnotationChanged(NetworkAnnotation& annotation,
                                                                    size_t index) {
    forEachObserver([&annotation, index](NetworkAnnotationsObserver* o) {
        o->onNetworkAnnotationChanged(annotation, index);
    });
}

NetworkAnnotations::NetworkAnnotations() : workspaceManager_{nullptr} {}

void NetworkAnnotations::setWorkspaceManager(WorkspaceManager* manager) {
    workspaceManager_ = manager;
}

void NetworkAnnotations::serialize(Serializer& s) const {
    s.serialize("Annotations", annotations_, "Annotation");
}

void NetworkAnnotations::deserialize(Deserializer& d) {
    try {
        d.deserialize("Annotations", annotations_, "Annotation",
                      deserializer::IndexFunctions{
                          .makeNew = []() { return NetworkAnnotation{}; },
                          .onNew =
                              [this](auto& annotation, size_t index) {
                                  notifyObserversAnnotationAdded(annotation, index);
                              },
                          .onRemove =
                              [this](auto& annotation, size_t index) {
                                  notifyObserversAnnotationWasRemoved(annotation, index);
                              },
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
        notifyObserversAnnotationChanged(annotation, index);
    }
}

size_t NetworkAnnotations::add(std::span<const Processor* const> processors) {
    const size_t index = annotations_.size();
    annotations_.emplace_back(NetworkAnnotation{
        .processors = processors |
                      std::views::transform([](auto* p) { return p->getIdentifier(); }) |
                      std::ranges::to<std::vector<std::string>>(),
        .title = std::nullopt,
        .description = std::nullopt,
        .color = randomColor(),
    });
    notifyObserversAnnotationAdded(annotations_.back(), index);
    if (workspaceManager_) {
        workspaceManager_->setModified();
    }
    return index;
}

size_t NetworkAnnotations::add(NetworkAnnotation&& annotation) {
    const size_t index = annotations_.size();
    annotations_.push_back(std::move(annotation));
    notifyObserversAnnotationAdded(annotations_.back(), index);
    if (workspaceManager_) {
        workspaceManager_->setModified();
    }
    return index;
}

void NetworkAnnotations::remove(size_t index) {
    if (index >= annotations_.size()) {
        throw RangeException{SourceContext{}, "index out or range {}", index};
    }
    auto annotation = std::move(annotations_[index]);
    annotations_.erase(annotations_.begin() + static_cast<std::ptrdiff_t>(index));
    notifyObserversAnnotationWasRemoved(annotations_[index], index);

    if (workspaceManager_) {
        workspaceManager_->setModified();
    }
}

void NetworkAnnotations::clear() {
    auto annotations = std::move(annotations_);
    annotations_.clear();

    for (auto&& [index, annotation] :
         std::views::zip(std::views::iota(0zu), annotations) | std::views::reverse) {
        notifyObserversAnnotationWasRemoved(annotation, index);
    }

    if (workspaceManager_) {
        workspaceManager_->setModified();
    }
}

size_t NetworkAnnotations::size() const { return annotations_.size(); }

void NetworkAnnotations::removeProcessor(const Processor* processor) {
    std::vector<size_t> removeLater;

    for (auto&& [index, annotation] : util::enumerate(annotations_)) {
        if (std::erase(annotation.processors, processor->getIdentifier())) {
            if (annotation.processors.empty()) {
                removeLater.push_back(index);
            } else {
                notifyObserversAnnotationChanged(annotation, index);
                if (workspaceManager_) {
                    workspaceManager_->setModified();
                }
            }
        }
    }
    for (const size_t index : removeLater | std::views::reverse) {
        remove(index);
    }
}

bool NetworkAnnotations::matches(size_t index, const std::vector<Processor*>& processors) const {
    return std::ranges::all_of(getAnnotation(index).processors, [&](const auto& id) {
        return std::ranges::contains(processors, id,
                                     [](const Processor* p) { return p->getIdentifier(); });
    });
}

void NetworkAnnotations::update(size_t index, NetworkAnnotation annotation) {
    if (index >= annotations_.size()) {
        throw RangeException{SourceContext{}, "index out or range {}", index};
    }
    if (annotation.processors.empty()) {
        remove(index);
        return;
    }

    annotations_[index] = std::move(annotation);
    notifyObserversAnnotationChanged(annotations_[index], index);
    if (workspaceManager_) {
        workspaceManager_->setModified();
    }
}

const NetworkAnnotation& NetworkAnnotations::getAnnotation(size_t index) const {
    if (index >= annotations_.size()) {
        throw RangeException{SourceContext{}, "index out or range {}", index};
    }
    return annotations_[index];
}

const std::vector<NetworkAnnotation>& NetworkAnnotations::getAnnotations() const {
    return annotations_;
}

void NetworkAnnotations::onProcessorNetworkWillRemoveProcessor(Processor* processor) {
    removeProcessor(processor);
}

}  // namespace inviwo
