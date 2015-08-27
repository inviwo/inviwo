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
                  ivec2(1, 1), VALID)
    , enableCustomInputDimensions_("enableCustomInputDimensions", "Separate Image Size", false,
                                   VALID)
    , customInputDimensions_("customInputDimensions", "Image Size", ivec2(256, 256),
                             ivec2(128, 128), ivec2(4096, 4096), ivec2(1, 1), VALID)
    , keepAspectRatio_("keepAspectRatio", "Lock Aspect Ratio", true, VALID)
    , aspectRatioScaling_("aspectRatioScaling", "Image Scale", 1.f, 0.1f, 4.f, 0.01f, VALID)
    , visibleLayer_("visibleLayer", "Visible Layer")
    , colorLayer_("colorLayer_", "Color Layer ID", 0, 0, 0)
    , saveLayerDirectory_("layerDir", "Output Directory", "", "image")
    , saveLayerButton_("saveLayer", "Save Image Layer", VALID)
    , inputSize_("inputSize", "Input Dimension Parameters")
    , previousImageSize_(customInputDimensions_)
    , evaluator_(nullptr)
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

    visibleLayer_.addOption("color", "Color layer", COLOR_LAYER);
    visibleLayer_.addOption("depth", "Depth layer", DEPTH_LAYER);
    visibleLayer_.addOption("picking", "Picking layer", PICKING_LAYER);
    visibleLayer_.set(COLOR_LAYER);
    addProperty(visibleLayer_);
    addProperty(colorLayer_);
    addProperty(saveLayerDirectory_);

    saveLayerButton_.onChange(this, &CanvasProcessor::saveImageLayer);
    addProperty(saveLayerButton_);

    colorLayer_.setSerializationMode(PropertySerializationMode::ALL);

    visibleLayer_.onChange([&](){
        if (inport_.hasData()){
            int layers = static_cast<int>(inport_.getData()->getNumberOfColorLayers());
            colorLayer_.setVisible(layers != 0 && visibleLayer_.get() == COLOR_LAYER);
        }
        colorLayer_.setVisible(visibleLayer_.get() == COLOR_LAYER);
    });

    inport_.onChange([&](){
        int layers = static_cast<int>(inport_.getData()->getNumberOfColorLayers());
        colorLayer_.setVisible(layers != 0 && visibleLayer_.get() == COLOR_LAYER);
        colorLayer_.setMaxValue(layers - 1);
    });


    setAllPropertiesCurrentStateAsDefault();
}

CanvasProcessor::~CanvasProcessor() {}


void CanvasProcessor::initialize() {
    Processor::initialize();
    if (processorWidget_) {
        canvasWidget_ = dynamic_cast<CanvasProcessorWidget*>(processorWidget_);
        canvasWidget_->getCanvas()->setEventPropagator(this);
    }
    evaluator_ = InviwoApplication::getPtr()->getProcessorNetworkEvaluator();
    sizeChanged();
}

void CanvasProcessor::deinitialize() {
    if (processorWidget_) {
        processorWidget_->hide();
        canvasWidget_->getCanvas()->setEventPropagator(nullptr);
    }
    canvasWidget_ = nullptr;
    Processor::deinitialize();
}

// Called by dimensions onChange.
void CanvasProcessor::resizeCanvas() {
    NetworkLock lock;
    if (canvasWidget_ && canvasWidget_->getDimensions() != dimensions_.get()) {
        canvasWidget_->setDimensions(dimensions_.get());
    }
    inputSize_.invalidate(VALID, &dimensions_);
}

void CanvasProcessor::setCanvasSize(ivec2 dim) {
    NetworkLock lock;
    dimensions_.set(dim);
    sizeChanged();
}

ivec2 CanvasProcessor::getCanvasSize() const { return dimensions_.get(); }

bool CanvasProcessor::getUseCustomDimensions() const { return enableCustomInputDimensions_; }
ivec2 CanvasProcessor::getCustomDimensions() const { return customInputDimensions_; }

void CanvasProcessor::sizeChanged() {
    NetworkLock lock;

    customInputDimensions_.setVisible(enableCustomInputDimensions_);
    customInputDimensions_.setReadOnly(keepAspectRatio_);
    keepAspectRatio_.setVisible(enableCustomInputDimensions_);
    aspectRatioScaling_.setVisible(enableCustomInputDimensions_ && keepAspectRatio_);

    if (keepAspectRatio_) customInputDimensions_ = calcSize();
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

    inputSize_.invalidate(VALID, &customInputDimensions_);
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
    std::string snapshotPath(saveLayerDirectory_.get() + "/" + toLower(getIdentifier()) + "-" +
                             currentDateTime() + ".png");
    saveImageLayer(snapshotPath);
}

void CanvasProcessor::saveImageLayer(std::string snapshotPath) {
    const Image* image = inport_.getData();
    if (image) {
        const Layer* layer = nullptr;
        if (visibleLayer_.get() == COLOR_LAYER){
            layer = image->getColorLayer(colorLayer_.get());
        }
        else{
            layer = image->getLayer(static_cast<LayerType>(visibleLayer_.get()));
        }
        if (layer) {
            std::string fileExtension = filesystem::getFileExtension(snapshotPath);
            DataWriterType<Layer>* writer = nullptr;
            bool deleteWriter = true;
            if (Canvas::generalLayerWriter_ && fileExtension == "png") {
                writer = Canvas::generalLayerWriter_;
                deleteWriter = false;
            } else {
                writer =
                    DataWriterFactory::getPtr()->getWriterForTypeAndExtension<Layer>(fileExtension).release();
            }

            if (writer) {
                try {
                    writer->setOverwrite(true);
                    writer->writeData(layer, snapshotPath);
                    LogInfo("Canvas layer exported to disk: " << snapshotPath);
                    if (deleteWriter) delete writer;
                }
                catch (DataWriterException const& e) {
                    LogError(e.getMessage());
                }
            } else {
                LogError(
                    "Error: Cound not find a writer for the specified extension and data type");
            }
        } else {
            LogError("Error: Cound not find color layer to write out");
        }
    } else if (snapshotPath.empty()) {
        LogWarn("Error: Please specify a file to write to");
    } else if (!image) {
        LogWarn("Error: Please connect an image to export");
    }
}

std::vector<unsigned char>* CanvasProcessor::getLayerAsCodedBuffer(LayerType layerType,
                                                                   std::string& type,
                                                                   size_t idx) {
    if (!inport_.hasData()) return nullptr;
    const Image* image = inport_.getData();
    const Layer* layer = image->getLayer(layerType, idx);

    if (layer) {
        auto writer = DataWriterFactory::getPtr()->getWriterForTypeAndExtension<Layer>(type);
        if (writer) {
            try {
                return writer->writeDataToBuffer(layer, type);
            } catch (DataWriterException const& e) {
                LogError(e.getMessage());
            }
        } else {
            LogError("Error: Cound not find a writer for the specified data type");
        }
    } else {
        LogError("Error: Cound not find layer to write");
    }

    return nullptr;
}

std::vector<unsigned char>* CanvasProcessor::getColorLayerAsCodedBuffer(std::string& type,
                                                                        size_t idx) {
    return getLayerAsCodedBuffer(LayerType::COLOR_LAYER, type, idx);
}

std::vector<unsigned char>* CanvasProcessor::getDepthLayerAsCodedBuffer(std::string& type) {
    return getLayerAsCodedBuffer(LayerType::DEPTH_LAYER, type);
}

std::vector<unsigned char>* CanvasProcessor::getPickingLayerAsCodedBuffer(std::string& type) {
    return getLayerAsCodedBuffer(LayerType::PICKING_LAYER, type);
}

std::vector<unsigned char>* CanvasProcessor::getVisibleLayerAsCodedBuffer(std::string& type) {
    if (visibleLayer_.get() == COLOR_LAYER){
        getColorLayerAsCodedBuffer(type, colorLayer_.get());
    }
    return getLayerAsCodedBuffer(static_cast<LayerType>(visibleLayer_.get()), type);
}

void CanvasProcessor::process() {
    if (canvasWidget_ && canvasWidget_->getCanvas()) {
        canvasWidget_->getCanvas()->activate();
        LayerType layerType = static_cast<LayerType>(visibleLayer_.get());
        if (visibleLayer_.get() == COLOR_LAYER){
            canvasWidget_->getCanvas()->render(inport_.getData(), COLOR_LAYER, colorLayer_.get());
        }
        else{
            canvasWidget_->getCanvas()->render(inport_.getData(), layerType, 0);
        }//*/
    }
}

void CanvasProcessor::doIfNotReady() {
    if (canvasWidget_ && canvasWidget_->getCanvas()) {
        canvasWidget_->getCanvas()->activate();
        canvasWidget_->getCanvas()->render(nullptr, static_cast<LayerType>(visibleLayer_.get()));
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

bool CanvasProcessor::propagateResizeEvent(ResizeEvent* event, Outport* source) {
    // avoid continues evaluation when port dimensions changes
    NetworkLock lock;

    dimensions_.set(event->size());

    if (enableCustomInputDimensions_) {
        sizeChanged();
    } else {
        inport_.propagateResizeEvent(event);

        // Make sure this processor is invalidated. 
        invalidate(INVALID_OUTPUT);
    }
    
    return false;
}

}  // namespace
