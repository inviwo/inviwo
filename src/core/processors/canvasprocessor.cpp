/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
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

CanvasProcessor::CanvasProcessor()
    : Processor()
    , inport_("inport")
    , dimensions_("dimensions", "Canvas Size", ivec2(256, 256), ivec2(128, 128), ivec2(4096, 4096),
                  ivec2(1, 1), InvalidationLevel::Valid)
    , enableCustomInputDimensions_("enableCustomInputDimensions", "Separate Image Size", false,
                                   InvalidationLevel::Valid)
    , customInputDimensions_("customInputDimensions", "Image Size", ivec2(256, 256),
                             ivec2(128, 128), ivec2(4096, 4096), ivec2(1, 1),
                             InvalidationLevel::Valid)
    , keepAspectRatio_("keepAspectRatio", "Lock Aspect Ratio", true, InvalidationLevel::Valid)
    , aspectRatioScaling_("aspectRatioScaling", "Image Scale", 1.f, 0.1f, 4.f, 0.01f,
                          InvalidationLevel::Valid)
    , position_("position", "Canvas Position", ivec2(128, 128),
                ivec2(std::numeric_limits<int>::lowest()), ivec2(std::numeric_limits<int>::max()),
                ivec2(1, 1), InvalidationLevel::Valid, PropertySemantics::Text)
    , visibleLayer_("visibleLayer", "Visible Layer")
    , colorLayer_("colorLayer_", "Color Layer ID", 0, 0, 0)
    , imageTypeExt_("fileExt", "Image Type")
    , saveLayerDirectory_("layerDir", "Output Directory", "", "image")
    , saveLayerButton_("saveLayer", "Save Image Layer", InvalidationLevel::Valid)
    , saveLayerToFileButton_("saveLayerToFile", "Save Image Layer to File...",
                             InvalidationLevel::Valid)
    , inputSize_("inputSize", "Input Dimension Parameters")
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
    addProperty(inputSize_);

    dimensions_.setSerializationMode(PropertySerializationMode::None);
    dimensions_.onChange([this]() { widgetMetaData_->setDimensions(dimensions_.get()); });
    inputSize_.addProperty(dimensions_);

    enableCustomInputDimensions_.onChange([this]() { sizeChanged(); });
    inputSize_.addProperty(enableCustomInputDimensions_);

    customInputDimensions_.onChange([this]() { sizeChanged(); });
    customInputDimensions_.setVisible(false);
    inputSize_.addProperty(customInputDimensions_);

    keepAspectRatio_.onChange([this]() { sizeChanged(); });
    keepAspectRatio_.setVisible(false);
    inputSize_.addProperty(keepAspectRatio_);

    aspectRatioScaling_.onChange([this]() { sizeChanged(); });
    aspectRatioScaling_.setVisible(false);
    inputSize_.addProperty(aspectRatioScaling_);

    position_.onChange([this]() { widgetMetaData_->setPosition(position_.get()); });
    addProperty(position_);

    visibleLayer_.addOption("color", "Color layer", LayerType::Color);
    visibleLayer_.addOption("depth", "Depth layer", LayerType::Depth);
    visibleLayer_.addOption("picking", "Picking layer", LayerType::Picking);
    visibleLayer_.set(LayerType::Color);

    // add all supported image extensions to option property
    auto wf = InviwoApplication::getPtr()->getDataWriterFactory();
    // save first writer extension matching "png" to be used as default
    std::string defaultExt;
    for (auto ext : wf->getExtensionsForType<Layer>()) {
        imageTypeExt_.addOption(ext.toString(), ext.toString());
        if (defaultExt.empty() && ext.extension_ == "png") {
            defaultExt = ext.toString();
        }
    }
    if (!defaultExt.empty()) {
        imageTypeExt_.setSelectedIdentifier(defaultExt);
    }

    addProperty(visibleLayer_);
    addProperty(colorLayer_);
    addProperty(saveLayerDirectory_);
    addProperty(imageTypeExt_);

    saveLayerButton_.onChange([this]() { saveImageLayer(); });
    addProperty(saveLayerButton_);

    saveLayerToFileButton_.onChange([this]() {
        if (auto layer = getVisibleLayer()) {
            util::saveLayer(*layer);
        } else {
            LogError("Could not find visible layer");
        }
    });
    addProperty(saveLayerToFileButton_);

    colorLayer_.setSerializationMode(PropertySerializationMode::All);
    colorLayer_.setVisible(false);

    visibleLayer_.onChange([&]() {
        if (inport_.hasData()) {
            auto layers = inport_.getData()->getNumberOfColorLayers();
            colorLayer_.setVisible(layers > 1 && visibleLayer_.get() == LayerType::Color);
        }
    });

    addProperty(fullScreen_);
    fullScreen_.onChange([this]() {
        if (auto c = getCanvas()) {
            c->setFullScreen(fullScreen_.get());
        }
    });
    addProperty(fullScreenEvent_);
    addProperty(saveLayerEvent_);
    addProperty(allowContextMenu_);

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

void CanvasProcessor::setCanvasSize(ivec2 dim) {
    NetworkLock lock(this);
    dimensions_.set(dim);
    sizeChanged();
}

ivec2 CanvasProcessor::getCanvasSize() const { return dimensions_; }

bool CanvasProcessor::getUseCustomDimensions() const { return enableCustomInputDimensions_; }
ivec2 CanvasProcessor::getCustomDimensions() const { return customInputDimensions_; }

void CanvasProcessor::sizeChanged() {
    NetworkLock lock(this);

    customInputDimensions_.setVisible(enableCustomInputDimensions_);
    customInputDimensions_.setReadOnly(keepAspectRatio_);
    keepAspectRatio_.setVisible(enableCustomInputDimensions_);
    aspectRatioScaling_.setVisible(enableCustomInputDimensions_ && keepAspectRatio_);

    if (keepAspectRatio_) customInputDimensions_.get() = calcSize();  // avoid triggering on change
    ResizeEvent resizeEvent(uvec2(0));
    if (enableCustomInputDimensions_) {
        resizeEvent.setSize(static_cast<uvec2>(customInputDimensions_.get()));
        resizeEvent.setPreviousSize(static_cast<uvec2>(previousImageSize_));
        previousImageSize_ = customInputDimensions_;
    } else {
        resizeEvent.setSize(static_cast<uvec2>(dimensions_.get()));
        resizeEvent.setPreviousSize(static_cast<uvec2>(previousImageSize_));
        previousImageSize_ = dimensions_;
    }

    inputSize_.invalidate(InvalidationLevel::Valid, &customInputDimensions_);
    inport_.propagateEvent(&resizeEvent);
}

ivec2 CanvasProcessor::calcSize() {
    ivec2 size = dimensions_;

    int maxDim, minDim;

    if (size.x >= size.y) {
        maxDim = 0;
        minDim = 1;
    } else {
        maxDim = 1;
        minDim = 0;
    }

    float ratio = static_cast<float>(size[minDim]) / static_cast<float>(size[maxDim]);
    size[maxDim] = static_cast<int>(static_cast<float>(size[maxDim]) * aspectRatioScaling_);
    size[minDim] = static_cast<int>(static_cast<float>(size[maxDim]) * ratio);

    return size;
}

void CanvasProcessor::saveImageLayer() {
    if (saveLayerDirectory_.get().empty()) saveLayerDirectory_.requestFile();

    auto ext = FileExtension::createFileExtensionFromString(imageTypeExt_.get());
    std::string snapshotPath(saveLayerDirectory_.get() + "/" + toLower(getIdentifier()) + "-" +
                             currentDateTime() + "." + ext.extension_);
    saveImageLayer(snapshotPath, ext);
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

    if (event->hash() == ResizeEvent::chash()) {
        auto resizeEvent = static_cast<ResizeEvent*>(event);

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
