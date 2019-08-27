/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/processors/canvasprocessor.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/canvas.h>
#include <inviwo/core/util/datetime.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/processors/canvasprocessorwidget.h>
#include <inviwo/core/processors/processorwidget.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/fileextension.h>
#include <inviwo/core/util/filedialog.h>
#include <inviwo/core/util/dialogfactory.h>
#include <inviwo/core/io/imagewriterutil.h>
#include <inviwo/core/network/networklock.h>

namespace inviwo {

CanvasProcessor::CanvasProcessor(InviwoApplication* app)
    : Processor()
    , inport_("inport")
    , inputSize_("inputSize", "Input Dimension Parameters")
    , dimensions_("dimensions", "Canvas Size", size2_t(256, 256), size2_t(1, 1),
                  size2_t(10000, 10000), size2_t(1, 1), InvalidationLevel::Valid)
    , enableCustomInputDimensions_("enableCustomInputDimensions", "Separate Image Size", false,
                                   InvalidationLevel::Valid)
    , customInputDimensions_("customInputDimensions", "Image Size", size2_t(256, 256),
                             size2_t(1, 1), size2_t(10000, 10000), size2_t(1, 1),
                             InvalidationLevel::Valid)
    , keepAspectRatio_("keepAspectRatio", "Lock Aspect Ratio", true, InvalidationLevel::Valid)
    , aspectRatioScaling_("aspectRatioScaling", "Image Scale", 1.f, 0.1f, 4.f, 0.01f,
                          InvalidationLevel::Valid)
    , position_("position", "Canvas Position", ivec2(128, 128),
                ivec2(std::numeric_limits<int>::lowest()), ivec2(std::numeric_limits<int>::max()),
                ivec2(1, 1), InvalidationLevel::Valid, PropertySemantics::Text)
    , visibleLayer_("visibleLayer", "Visible Layer",
                    {{"color", "Color layer", LayerType::Color},
                     {"depth", "Depth layer", LayerType::Depth},
                     {"picking", "Picking layer", LayerType::Picking}},
                    0)
    , colorLayer_("colorLayer_", "Color Layer ID", 0, 0, 0)
    , imageTypeExt_(
          "fileExt", "Image Type",
          [app]() {
              const auto exts = app->getDataWriterFactory()->getExtensionsForType<Layer>();
              std::vector<OptionPropertyOption<FileExtension>> res;
              std::transform(exts.begin(), exts.end(), std::back_inserter(res), [](auto& ext) {
                  return OptionPropertyOption<FileExtension>{ext.toString(), ext.toString(), ext};
              });
              return res;
          }(),
          [app]() {
              const auto exts = app->getDataWriterFactory()->getExtensionsForType<Layer>();
              const auto it = std::find_if(exts.begin(), exts.end(),
                                           [](auto& ext) { return ext.extension_ == "png"; });
              return it == exts.end() ? 0 : std::distance(exts.begin(), it);
          }())
    , saveLayerDirectory_("layerDir", "Output Directory", "", "image")
    , saveLayerButton_("saveLayer", "Save Image Layer", [this]() { saveImageLayer(); },
                       InvalidationLevel::Valid)
    , saveLayerToFileButton_("saveLayerToFile", "Save Image Layer to File...",
                             [this]() {
                                 if (auto layer = getVisibleLayer()) {
                                     util::saveLayer(*layer);
                                 } else {
                                     LogError("Could not find visible layer");
                                 }
                             },
                             InvalidationLevel::Valid)
    , fullScreen_("fullscreen", "Toggle Full Screen", false)
    , fullScreenEvent_("fullscreenEvent", "FullScreen",
                       [this](Event*) { fullScreen_.set(!fullScreen_); }, IvwKey::F,
                       KeyState::Press, KeyModifier::Shift)
    , saveLayerEvent_("saveLayerEvent", "Save Image Layer", [this](Event*) { saveImageLayer(); },
                      IvwKey::Undefined, KeyState::Press)
    , allowContextMenu_("allowContextMenu", "Allow Context Menu", true)
    , previousImageSize_(customInputDimensions_)
    , widgetMetaData_{
          createMetaData<ProcessorWidgetMetaData>(ProcessorWidgetMetaData::CLASS_IDENTIFIER)} {
    widgetMetaData_->addObserver(this);

    setEvaluateWhenHidden(false);

    addPort(inport_);

    dimensions_.setSerializationMode(PropertySerializationMode::None);
    dimensions_.onChange([this]() { widgetMetaData_->setDimensions(dimensions_.get()); });

    enableCustomInputDimensions_.onChange([this]() { sizeChanged(); });

    customInputDimensions_.onChange([this]() { sizeChanged(); });
    customInputDimensions_.setVisible(false);

    keepAspectRatio_.onChange([this]() { sizeChanged(); });
    keepAspectRatio_.setVisible(false);

    aspectRatioScaling_.onChange([this]() { sizeChanged(); });
    aspectRatioScaling_.setVisible(false);

    position_.onChange([this]() { widgetMetaData_->setPosition(position_.get()); });

    colorLayer_.setSerializationMode(PropertySerializationMode::All);
    colorLayer_.setVisible(false);

    visibleLayer_.onChange([&]() {
        if (inport_.hasData()) {
            auto layers = inport_.getData()->getNumberOfColorLayers();
            colorLayer_.setVisible(layers > 1 && visibleLayer_.get() == LayerType::Color);
        }
    });

    fullScreen_.onChange([this]() {
        if (auto c = getCanvas()) {
            c->setFullScreen(fullScreen_.get());
        }
    });

    imageTypeExt_.setSerializationMode(PropertySerializationMode::None);

    inputSize_.addProperties(dimensions_, enableCustomInputDimensions_, customInputDimensions_,
                             keepAspectRatio_, aspectRatioScaling_);

    addProperties(inputSize_, position_, visibleLayer_, colorLayer_, saveLayerDirectory_,
                  imageTypeExt_, saveLayerButton_, saveLayerToFileButton_, fullScreen_,
                  fullScreenEvent_, saveLayerEvent_, allowContextMenu_);

    inport_.onChange([&]() {
        int layers = static_cast<int>(inport_.getData()->getNumberOfColorLayers());
        colorLayer_.setVisible(layers > 1 && visibleLayer_.get() == LayerType::Color);
        colorLayer_.setMaxValue(layers - 1);
    });

    inport_.onConnect([&]() { sizeChanged(); });

    setAllPropertiesCurrentStateAsDefault();
}

CanvasProcessor::~CanvasProcessor() {
    if (processorWidget_) {
        processorWidget_->hide();
        getCanvas()->setEventPropagator(nullptr);
    }
}

void CanvasProcessor::setProcessorWidget(std::unique_ptr<ProcessorWidget> processorWidget) {
    if (processorWidget && !dynamic_cast<CanvasProcessorWidget*>(processorWidget.get())) {
        throw Exception("Expected CanvasProcessorWidget in CanvasProcessor::setProcessorWidget");
    }
    Processor::setProcessorWidget(std::move(processorWidget));
    // Widget may be set after deserialization
    if (auto c = getCanvas()) {
        c->setFullScreen(fullScreen_.get());
    }
    isSink_.update();
    isReady_.update();
}

void CanvasProcessor::onProcessorWidgetPositionChange(ProcessorWidgetMetaData*) {
    if (widgetMetaData_->getPosition() != position_.get()) {
        Property::OnChangeBlocker blocker{position_};
        position_.set(widgetMetaData_->getPosition());
    }
}

void CanvasProcessor::onProcessorWidgetDimensionChange(ProcessorWidgetMetaData*) {
    if (widgetMetaData_->getDimensions() != dimensions_.get()) {
        Property::OnChangeBlocker blocker{dimensions_};
        dimensions_.set(widgetMetaData_->getDimensions());
    }
}

void CanvasProcessor::onProcessorWidgetVisibilityChange(ProcessorWidgetMetaData*) {
    isSink_.update();
    isReady_.update();
    invalidate(InvalidationLevel::InvalidOutput);
}

void CanvasProcessor::setCanvasSize(size2_t dim) {
    NetworkLock lock(this);
    dimensions_.set(dim);
    sizeChanged();
}

size2_t CanvasProcessor::getCanvasSize() const { return dimensions_; }

bool CanvasProcessor::getUseCustomDimensions() const { return enableCustomInputDimensions_; }
size2_t CanvasProcessor::getCustomDimensions() const { return customInputDimensions_; }

void CanvasProcessor::sizeChanged() {
    NetworkLock lock(this);

    customInputDimensions_.setVisible(enableCustomInputDimensions_);
    customInputDimensions_.setReadOnly(keepAspectRatio_);
    keepAspectRatio_.setVisible(enableCustomInputDimensions_);
    aspectRatioScaling_.setVisible(enableCustomInputDimensions_ && keepAspectRatio_);

    if (keepAspectRatio_) {
        Property::OnChangeBlocker block{customInputDimensions_};  // avoid recursive onChange
        customInputDimensions_ = calcSize();
    }

    ResizeEvent resizeEvent{enableCustomInputDimensions_ ? customInputDimensions_ : dimensions_,
                            previousImageSize_};
    previousImageSize_ = resizeEvent.size();

    inputSize_.invalidate(InvalidationLevel::Valid, &customInputDimensions_);
    inport_.propagateEvent(&resizeEvent);
}

size2_t CanvasProcessor::calcSize() {
    size2_t size = dimensions_;

    int maxDim, minDim;

    if (size.x >= size.y) {
        maxDim = 0;
        minDim = 1;
    } else {
        maxDim = 1;
        minDim = 0;
    }

    double ratio = static_cast<double>(size[minDim]) / static_cast<double>(size[maxDim]);
    size[maxDim] = static_cast<int>(static_cast<double>(size[maxDim]) * aspectRatioScaling_);
    size[minDim] = static_cast<int>(static_cast<double>(size[maxDim]) * ratio);

    return size;
}

void CanvasProcessor::saveImageLayer() {
    if (saveLayerDirectory_.get().empty()) saveLayerDirectory_.requestFile();

    std::string snapshotPath(saveLayerDirectory_.get() + "/" + toLower(getIdentifier()) + "-" +
                             currentDateTime() + "." + imageTypeExt_->extension_);
    saveImageLayer(snapshotPath, imageTypeExt_);
}

void CanvasProcessor::saveImageLayer(std::string snapshotPath, const FileExtension& extension) {
    if (auto layer = getVisibleLayer()) {
        util::saveLayer(*layer, snapshotPath, extension);
    } else {
        LogError("Could not find visible layer");
    }
}

const Layer* CanvasProcessor::getVisibleLayer() const {
    if (auto image = inport_.getData()) {
        return image->getLayer(visibleLayer_, colorLayer_);
    } else {
        return nullptr;
    }
}

std::shared_ptr<const Image> CanvasProcessor::getImage() const { return inport_.getData(); }

Canvas* CanvasProcessor::getCanvas() const {
    if (auto canvasWidget = static_cast<CanvasProcessorWidget*>(processorWidget_.get())) {
        return canvasWidget->getCanvas();
    } else {
        return nullptr;
    }
}

void CanvasProcessor::process() {
    if (auto c = getCanvas()) {
        c->render(inport_.getData(), visibleLayer_, colorLayer_);
    }
}

void CanvasProcessor::doIfNotReady() {
    if (auto c = getCanvas()) {
        c->render(nullptr, visibleLayer_, colorLayer_);
    }
}

void CanvasProcessor::propagateEvent(Event* event, Outport* source) {
    if (event->hasVisitedProcessor(this)) return;
    event->markAsVisited(this);

    invokeEvent(event);
    if (event->hasBeenUsed()) return;

    if (auto resizeEvent = event->getAs<ResizeEvent>()) {
        // Avoid continues evaluation when port dimensions changes
        NetworkLock lock(this);
        dimensions_.set(resizeEvent->size());
        if (enableCustomInputDimensions_) {
            sizeChanged();
        } else {
            inport_.propagateEvent(resizeEvent, nullptr);
            // Make sure this processor is invalidated.
            invalidate(InvalidationLevel::InvalidOutput);
        }
    } else {
        bool used = event->hasBeenUsed();
        for (auto inport : getInports()) {
            if (event->shouldPropagateTo(inport, this, source)) {
                inport->propagateEvent(event);
                used |= event->hasBeenUsed();
                event->markAsUnused();
            }
        }
        if (used) event->markAsUsed();
    }
}

bool CanvasProcessor::isContextMenuAllowed() const { return allowContextMenu_; }

void CanvasProcessor::setEvaluateWhenHidden(bool value) {
    if (value) {
        isSink_.setUpdate([]() { return true; });
        isReady_.setUpdate([this]() { return allInportsAreReady(); });
    } else {
        isSink_.setUpdate([this]() { return processorWidget_ && processorWidget_->isVisible(); });
        isReady_.setUpdate([this]() {
            return allInportsAreReady() && processorWidget_ && processorWidget_->isVisible();
        });
    }
}

}  // namespace inviwo
