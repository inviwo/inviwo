/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <modules/openglqt/processors/canvaswithpropertiesprocessor.h>

#include <modules/openglqt/processors/canvaswithpropertiesprocessorwidgetqt.h>
#include <inviwo/core/metadata/processorwidgetmetadata.h>

#include <modules/openglqt/canvasqopenglwidget.h>

namespace inviwo {

struct PWObserver : ProcessorWidgetMetaDataObserver {
    virtual void onProcessorWidgetPositionChange(ProcessorWidgetMetaData* m) override {
        position(m);
    }
    virtual void onProcessorWidgetDimensionChange(ProcessorWidgetMetaData* m) override {
        dimension(m);
    }
    virtual void onProcessorWidgetVisibilityChange(ProcessorWidgetMetaData* m) override {
        visible(m);
    }
    virtual void onProcessorWidgetFullScreenChange(ProcessorWidgetMetaData* m) override {
        fullScreen(m);
    }
    virtual void onProcessorWidgetOnTopChange(ProcessorWidgetMetaData* m) override { onTop(m); }

    std::function<void(ProcessorWidgetMetaData*)> position;
    std::function<void(ProcessorWidgetMetaData*)> dimension;
    std::function<void(ProcessorWidgetMetaData*)> visible;
    std::function<void(ProcessorWidgetMetaData*)> fullScreen;
    std::function<void(ProcessorWidgetMetaData*)> onTop;
};

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo CanvasWithPropertiesProcessor::processorInfo_{
    "org.inviwo.CanvasWithPropertiesProcessor",  // Class identifier
    "Canvas With Properties",                    // Display name
    "Data Output",                               // Category
    CodeState::Experimental,                     // Code state
    Tags::GL,                                    // Tags
};
const ProcessorInfo CanvasWithPropertiesProcessor::getProcessorInfo() const {
    return processorInfo_;
}

CanvasWithPropertiesProcessor::CanvasWithPropertiesProcessor()
    : Processor()
    , pwObserver_{std::make_unique<PWObserver>()}
    , inport_{"inport"}
    , dimensions_{"dimensions",
                  "Widget Size",
                  size2_t(756, 512),
                  std::pair{size2_t{1}, ConstraintBehavior::Immutable},
                  std::pair{size2_t{10000}, ConstraintBehavior::Ignore},
                  size2_t(1, 1),
                  InvalidationLevel::Valid}
    , position_{"position",
                "Widget Position",
                ivec2(128, 128),
                std::pair{ivec2{-10000}, ConstraintBehavior::Ignore},
                std::pair{ivec2{10000}, ConstraintBehavior::Ignore},
                ivec2(1, 1),
                InvalidationLevel::Valid,
                PropertySemantics::Text}
    , visible_{"visible", "Visible", true, InvalidationLevel::Valid}
    , fullScreen_{"fullScreen", "Full Screen", false, InvalidationLevel::Valid}
    , onTop_{"onTop", "On Top", true, InvalidationLevel::Valid}
    , layerType_{"layerType",
                 "Visible Layer",
                 {{"color", "Color layer", LayerType::Color},
                  {"depth", "Depth layer", LayerType::Depth},
                  {"picking", "Picking layer", LayerType::Picking}},
                 0}
    , layerIndex_{"layerIndex", "Color Layer ID", 0, 0, 0}
    , paths_{"paths", "Paths", "", InvalidationLevel::InvalidOutput, PropertySemantics::Multiline} {

    pwObserver_->position = [this](ProcessorWidgetMetaData* m) {
        if (m->getPosition() != position_.get()) {
            Property::OnChangeBlocker blocker{position_};
            position_.set(m->getPosition());
        }
    };
    pwObserver_->dimension = [this](ProcessorWidgetMetaData* m) {
        if (m->getDimensions() != dimensions_.get()) {
            Property::OnChangeBlocker blocker{dimensions_};
            dimensions_.set(m->getDimensions());
        }
    };
    pwObserver_->visible = [this](ProcessorWidgetMetaData* m) {
        if (m->isVisible() != visible_.get()) {
            Property::OnChangeBlocker blocker{visible_};
            visible_.set(m->isVisible());
        }
        isSink_.update();
        isReady_.update();
        invalidate(InvalidationLevel::InvalidOutput);
    };
    pwObserver_->fullScreen = [this](ProcessorWidgetMetaData* m) {
        if (m->isFullScreen() != fullScreen_.get()) {
            Property::OnChangeBlocker blocker{fullScreen_};
            fullScreen_.set(m->isFullScreen());
        }
    };
    pwObserver_->onTop = [this](ProcessorWidgetMetaData* m) {
        if (m->isOnTop() != onTop_.get()) {
            Property::OnChangeBlocker blocker{onTop_};
            onTop_.set(m->isOnTop());
        }
    };

    auto wmd = createMetaData<ProcessorWidgetMetaData>(ProcessorWidgetMetaData::CLASS_IDENTIFIER);
    wmd->addObserver(pwObserver_.get());
    dimensions_.onChange([this, wmd]() { wmd->setDimensions(dimensions_.get()); });
    position_.onChange([this, wmd]() { wmd->setPosition(position_.get()); });
    visible_.onChange([this, wmd]() { wmd->setVisibile(visible_.get()); });
    fullScreen_.onChange([this, wmd]() { wmd->setFullScreen(fullScreen_.get()); });
    onTop_.onChange([this, wmd]() { wmd->setOnTop(onTop_.get()); });

    addPort(inport_);

    addProperties(dimensions_, position_, visible_, fullScreen_, onTop_, layerType_, layerIndex_,
                  paths_);

    layerIndex_.setSerializationMode(PropertySerializationMode::All);
    layerIndex_.readonlyDependsOn(layerType_, [](const auto& p) { return p != LayerType::Color; });
    inport_.onChange([&]() {
        int layers = static_cast<int>(inport_.getData()->getNumberOfColorLayers());
        layerIndex_.setMaxValue(layers - 1);
    });

    isSink_.setUpdate([this]() { return processorWidget_ && processorWidget_->isVisible(); });
    isReady_.setUpdate([this]() {
        return allInportsAreReady() && processorWidget_ && processorWidget_->isVisible();
    });
}

CanvasWithPropertiesProcessor::~CanvasWithPropertiesProcessor() = default;

void CanvasWithPropertiesProcessor::process() {
    if (auto widget = static_cast<CanvasWithPropertiesProcessorWidgetQt*>(processorWidget_.get())) {
        widget->addProperties(paths_.get());

        if (widget->QMainWindow::isVisible()) {
            if (auto canvas = widget->getCanvas()) {
                canvas->render(inport_.getData(), layerType_, layerIndex_);
            }
        }
    }
}

void CanvasWithPropertiesProcessor::doIfNotReady() {
    if (auto widget = static_cast<CanvasWithPropertiesProcessorWidgetQt*>(processorWidget_.get())) {
        widget->addProperties(paths_.get());

        if (widget->QMainWindow::isVisible()) {
            if (auto canvas = widget->getCanvas()) {
                canvas->render(nullptr, layerType_, layerIndex_);
            }
        }
    }
}

void CanvasWithPropertiesProcessor::setProcessorWidget(
    std::unique_ptr<ProcessorWidget> processorWidget) {
    Processor::setProcessorWidget(std::move(processorWidget));
    isSink_.update();
    isReady_.update();
};

void CanvasWithPropertiesProcessor::propagateEvent(Event* event, Outport*) {
    event->markAsVisited(this);
    inport_.propagateEvent(event);
}

}  // namespace inviwo
