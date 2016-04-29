/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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
#include <inviwo/core/network/processornetworkevaluator.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/fileextension.h>

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
    , visibleLayer_("visibleLayer", "Visible Layer")
    , colorLayer_("colorLayer_", "Color Layer ID", 0, 0, 0)
    , saveLayerDirectory_("layerDir", "Output Directory", "", "image")
    , saveLayerButton_("saveLayer", "Save Image Layer", InvalidationLevel::Valid)
    , inputSize_("inputSize", "Input Dimension Parameters")
    , previousImageSize_(customInputDimensions_)
    , canvasWidget_(nullptr)
    , queuedRequest_(false) {
    addPort(inport_);
    addProperty(inputSize_);

    dimensions_.onChange(this, &CanvasProcessor::resizeCanvas);
    inputSize_.addProperty(dimensions_);

    enableCustomInputDimensions_.onChange(this, &CanvasProcessor::sizeChanged);
    inputSize_.addProperty(enableCustomInputDimensions_);

    customInputDimensions_.onChange(this, &CanvasProcessor::sizeChanged);
    customInputDimensions_.setVisible(false);
    inputSize_.addProperty(customInputDimensions_);

    keepAspectRatio_.onChange(this, &CanvasProcessor::sizeChanged);
    keepAspectRatio_.setVisible(false);
    inputSize_.addProperty(keepAspectRatio_);

    aspectRatioScaling_.onChange(this, &CanvasProcessor::sizeChanged);
    aspectRatioScaling_.setVisible(false);
    inputSize_.addProperty(aspectRatioScaling_);

    visibleLayer_.addOption("color", "Color layer", LayerType::Color);
    visibleLayer_.addOption("depth", "Depth layer", LayerType::Depth);
    visibleLayer_.addOption("picking", "Picking layer", LayerType::Picking);
    visibleLayer_.set(LayerType::Color);
    addProperty(visibleLayer_);
    addProperty(colorLayer_);
    addProperty(saveLayerDirectory_);

    saveLayerButton_.onChange(this, &CanvasProcessor::saveImageLayer);
    addProperty(saveLayerButton_);

    colorLayer_.setSerializationMode(PropertySerializationMode::All);
    colorLayer_.setVisible(false);

    visibleLayer_.onChange([&]() {
        if (inport_.hasData()) {
            auto layers = inport_.getData()->getNumberOfColorLayers();
            colorLayer_.setVisible(layers > 1 && visibleLayer_.get() == LayerType::Color);
        }
    });

    inport_.onChange([&]() {
        int layers = static_cast<int>(inport_.getData()->getNumberOfColorLayers());
        colorLayer_.setVisible(layers > 1 && visibleLayer_.get() == LayerType::Color);
        colorLayer_.setMaxValue(layers - 1);
    });

    inport_.onConnect([&](){
       sizeChanged();
    });

    setAllPropertiesCurrentStateAsDefault();
}

CanvasProcessor::~CanvasProcessor() {
    if (processorWidget_) {
        processorWidget_->hide();
        canvasWidget_->getCanvas()->setEventPropagator(nullptr);
    }
}

void CanvasProcessor::setProcessorWidget(std::unique_ptr<ProcessorWidget> processorWidget) {
    if (auto cw = dynamic_cast<CanvasProcessorWidget*>(processorWidget.get())) {
        canvasWidget_ = cw;
    }
    Processor::setProcessorWidget(std::move(processorWidget));
}

// Called by dimensions onChange.
void CanvasProcessor::resizeCanvas() {
    NetworkLock lock(this);
    if (canvasWidget_ && canvasWidget_->getDimensions() != dimensions_.get()) {
        canvasWidget_->setDimensions(dimensions_.get());
    }
    inputSize_.invalidate(InvalidationLevel::Valid, &dimensions_);
}

void CanvasProcessor::setCanvasSize(ivec2 dim) {
    NetworkLock lock(this);
    dimensions_.set(dim);
    sizeChanged();
}

ivec2 CanvasProcessor::getCanvasSize() const { return dimensions_.get(); }

bool CanvasProcessor::getUseCustomDimensions() const { return enableCustomInputDimensions_; }
ivec2 CanvasProcessor::getCustomDimensions() const { return customInputDimensions_; }

void CanvasProcessor::sizeChanged() {
    NetworkLock lock(this);

    customInputDimensions_.setVisible(enableCustomInputDimensions_);
    customInputDimensions_.setReadOnly(keepAspectRatio_);
    keepAspectRatio_.setVisible(enableCustomInputDimensions_);
    aspectRatioScaling_.setVisible(enableCustomInputDimensions_ && keepAspectRatio_);

    if (keepAspectRatio_) customInputDimensions_.get() = calcSize(); // avoid triggering on change
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
    inport_.propagateResizeEvent(&resizeEvent);
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
    std::string snapshotPath(saveLayerDirectory_.get() + "/" + toLower(getIdentifier()) + "-" +
                             currentDateTime() + ".png");
    saveImageLayer(snapshotPath);
}

void CanvasProcessor::saveImageLayer(std::string snapshotPath) {
    if (auto layer = getSelectedLayer()) {
        if (auto writer = getWriter(filesystem::getFileExtension(snapshotPath))) {
            try {
                writer->setOverwrite(true);
                writer->writeData(layer, snapshotPath);
                LogInfo("Canvas layer exported to disk: " << snapshotPath);
            } catch (DataWriterException const& e) {
                LogError(e.getMessage());
            }
        } else {
            LogError("Error: Could not find a writer for the specified extension and data type");
        }
    } else {
        LogError("Error: Could not find a layer to write out");
    }
}

const Layer* CanvasProcessor::getSelectedLayer() const {
    if (auto image = inport_.getData()) {
        if (visibleLayer_.get() == LayerType::Color) {
            return image->getColorLayer(colorLayer_.get());
        } else {
            return image->getLayer(visibleLayer_.get());
        }
    } else {
        return nullptr;
    }
}

std::shared_ptr<DataWriterType<Layer>> CanvasProcessor::getWriter(
    const std::string& fileExtension) const {
    return std::shared_ptr<DataWriterType<Layer>>(
        getNetwork()->getApplication()->getDataWriterFactory()->getWriterForTypeAndExtension<Layer>(
            fileExtension));
}

std::unique_ptr<std::vector<unsigned char>> CanvasProcessor::getLayerAsCodedBuffer(
    LayerType layerType, std::string& type, size_t idx) {
    if (!inport_.hasData()) return nullptr;
    auto image = inport_.getData();
    
    if (auto layer = image->getLayer(layerType, idx)) {
        if (auto writer = getWriter(type)) {
            try {
                return writer->writeDataToBuffer(layer, type);
            } catch (DataWriterException const& e) {
                LogError(e.getMessage());
            }
        } else {
            LogError("Error: Could not find a writer for the specified data type");
        }
    } else {
        LogError("Error: Could not find layer to write");
    }

    return nullptr;
}

std::unique_ptr<std::vector<unsigned char>> CanvasProcessor::getColorLayerAsCodedBuffer(
    std::string& type, size_t idx) {
    return getLayerAsCodedBuffer(LayerType::Color, type, idx);
}

std::unique_ptr<std::vector<unsigned char>> CanvasProcessor::getDepthLayerAsCodedBuffer(
    std::string& type) {
    return getLayerAsCodedBuffer(LayerType::Depth, type);
}

std::unique_ptr<std::vector<unsigned char>> CanvasProcessor::getPickingLayerAsCodedBuffer(
    std::string& type) {
    return getLayerAsCodedBuffer(LayerType::Picking, type);
}

std::unique_ptr<std::vector<unsigned char>> CanvasProcessor::getVisibleLayerAsCodedBuffer(
    std::string& type) {
    if (visibleLayer_.get() == LayerType::Color) {
        return getColorLayerAsCodedBuffer(type, colorLayer_.get());
    }
    return getLayerAsCodedBuffer(visibleLayer_.get(), type);
}

void CanvasProcessor::process() {
    if (canvasWidget_ && canvasWidget_->getCanvas()) {
        LayerType layerType = visibleLayer_.get();
        if (visibleLayer_.get() == LayerType::Color) {
            canvasWidget_->getCanvas()->render(inport_.getData(), LayerType::Color,
                                               colorLayer_.get());
        } else {
            canvasWidget_->getCanvas()->render(inport_.getData(), layerType, 0);
        }
    }
}

void CanvasProcessor::doIfNotReady() {
    if (canvasWidget_ && canvasWidget_->getCanvas()) {
        canvasWidget_->getCanvas()->render(nullptr, visibleLayer_.get());
    }
}

void CanvasProcessor::triggerQueuedEvaluation() {
    if (queuedRequest_) {
        performEvaluateRequest();
        queuedRequest_ = false;
    }
}

void CanvasProcessor::performEvaluationAtNextShow() { queuedRequest_ = true; }

void CanvasProcessor::performEvaluateRequest() {
    if (processorWidget_) {
        if (processorWidget_->isVisible()) {
            notifyObserversRequestEvaluate(this);
        } else {
            performEvaluationAtNextShow();
        }
    } else {
        notifyObserversRequestEvaluate(this);
    }
}

bool CanvasProcessor::isReady() const {
    return Processor::isReady() && processorWidget_ && processorWidget_->isVisible();
}

void CanvasProcessor::propagateResizeEvent(ResizeEvent* resizeEvent, Outport* source) {
    if (resizeEvent->hasVisitedProcessor(this)) return;
    resizeEvent->markAsVisited(this);
    
    // Avoid continues evaluation when port dimensions changes
    NetworkLock lock(this);

    dimensions_.set(resizeEvent->size());

    if (enableCustomInputDimensions_) {
        sizeChanged();
    } else {
        inport_.propagateResizeEvent(resizeEvent);
        // Make sure this processor is invalidated.
        invalidate(InvalidationLevel::InvalidOutput);
    }
}

}  // namespace
