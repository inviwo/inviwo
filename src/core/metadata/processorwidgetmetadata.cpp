/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#include <inviwo/core/metadata/processorwidgetmetadata.h>

namespace inviwo {

ProcessorWidgetMetaData::ProcessorWidgetMetaData()
    : MetaData()
    , Observable<ProcessorWidgetMetaDataObserver>()
    , position_(0, 0)
    , dimensions_(256, 256)
    , visible_(true)
    , fullScreen_(false)
    , onTop_(true) {}

ProcessorWidgetMetaData* ProcessorWidgetMetaData::clone() const {
    return new ProcessorWidgetMetaData(*this);
}

void ProcessorWidgetMetaData::setPosition(const ivec2& pos,
                                          const ProcessorWidgetMetaDataObserver* source) {
    if (pos != position_) {
        position_ = pos;
        forEachObserver([&](ProcessorWidgetMetaDataObserver* o) {
            if (o != source) o->onProcessorWidgetPositionChange(this);
        });
    }
}

ivec2 ProcessorWidgetMetaData::getPosition() const { return position_; }

void ProcessorWidgetMetaData::setDimensions(const size2_t& dim,
                                            const ProcessorWidgetMetaDataObserver* source) {
    if (dim != dimensions_) {
        dimensions_ = dim;
        forEachObserver([&](ProcessorWidgetMetaDataObserver* o) {
            if (o != source) o->onProcessorWidgetDimensionChange(this);
        });
    }
}

size2_t ProcessorWidgetMetaData::getDimensions() const { return dimensions_; }

void ProcessorWidgetMetaData::setVisible(bool visible,
                                         const ProcessorWidgetMetaDataObserver* source) {
    if (visible != visible_) {
        visible_ = visible;
        forEachObserver([&](ProcessorWidgetMetaDataObserver* o) {
            if (o != source) o->onProcessorWidgetVisibilityChange(this);
        });
    }
}

bool ProcessorWidgetMetaData::isVisible() const { return visible_; }

void ProcessorWidgetMetaData::setFullScreen(bool fullScreen,
                                            const ProcessorWidgetMetaDataObserver* source) {
    if (fullScreen != fullScreen_) {
        fullScreen_ = fullScreen;
        forEachObserver([&](ProcessorWidgetMetaDataObserver* o) {
            if (o != source) o->onProcessorWidgetFullScreenChange(this);
        });
    }
}

bool ProcessorWidgetMetaData::isFullScreen() const { return fullScreen_; }

void ProcessorWidgetMetaData::setOnTop(bool onTop, const ProcessorWidgetMetaDataObserver* source) {
    if (onTop != onTop_) {
        onTop_ = onTop;
        forEachObserver([&](ProcessorWidgetMetaDataObserver* o) {
            if (o != source) o->onProcessorWidgetOnTopChange(this);
        });
    }
}

bool ProcessorWidgetMetaData::isOnTop() const { return onTop_; }

void ProcessorWidgetMetaData::serialize(Serializer& s) const {
    s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);

    if (s.getWorkspaceSaveMode() == WorkspaceSaveMode::Undo) return;

    s.serialize("position", position_);
    s.serialize("dimensions", dimensions_);
    s.serialize("visibility", visible_);
    s.serialize("fullScreen", fullScreen_);
    s.serialize("onTop", onTop_);
}

void ProcessorWidgetMetaData::deserialize(Deserializer& d) {
    ivec2 position{position_};
    d.deserialize("position", position);
    setPosition(position);

    size2_t dimensions{dimensions_};
    d.deserialize("dimensions", dimensions);
    setDimensions(dimensions);

    bool visibility{visible_};
    d.deserialize("visibility", visibility);
    setVisible(visibility);

    bool fullScreen{fullScreen_};
    d.deserialize("fullScreen", fullScreen);
    setFullScreen(fullScreen);

    bool onTop{onTop_};
    d.deserialize("onTop", onTop);
    setOnTop(onTop);
}

bool ProcessorWidgetMetaData::equal(const MetaData& rhs) const {
    if (auto tmp = dynamic_cast<const ProcessorWidgetMetaData*>(&rhs)) {
        return tmp->position_ == position_ && tmp->visible_ == visible_ &&
               tmp->dimensions_ == dimensions_ && tmp->fullScreen_ == fullScreen_ &&
               tmp->onTop_ == onTop_;
    } else {
        return false;
    }
}

}  // namespace inviwo
