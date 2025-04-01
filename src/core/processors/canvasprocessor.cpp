/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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
#include <inviwo/core/io/datawriterutil.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/fileextension.h>
#include <inviwo/core/util/filedialog.h>
#include <inviwo/core/io/imagewriterutil.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/util/rendercontext.h>

namespace inviwo {

CanvasProcessor::CanvasProcessor(InviwoApplication* app)
    : Processor()
    , inport_{"inport", "Image to render in the processor widget window"_help}
    , inputSize_{"inputSize", "Input Dimension Parameters",
                 "Settings for the canvas (window) and image size"_help}
    , dimensions_{"dimensions",
                  "Canvas Size",
                  "The size of the processor widget window"_help,
                  size2_t(256, 256),
                  {size2_t(1, 1), ConstraintBehavior::Immutable},
                  {size2_t(10000, 10000), ConstraintBehavior::Ignore},
                  size2_t(1, 1),
                  InvalidationLevel::Valid}
    , enableCustomInputDimensions_{"enableCustomInputDimensions", "Separate Image Size",
                                   "Specify a different image size to render into than what "
                                   "will be show in the canvas window"_help,
                                   false, InvalidationLevel::Valid}
    , customInputDimensions_{"customInputDimensions",
                             "Image Size",
                             "The size of the image that will be generated. This can be larger "
                             "or smaller than the canvas size. A smaller size will generate a "
                             "blurrier canvas, but render faster. A larger size will render "
                             "slower."_help,
                             size2_t(256, 256),
                             {size2_t(1, 1), ConstraintBehavior::Immutable},
                             {size2_t(10000, 10000), ConstraintBehavior::Ignore},
                             size2_t(1, 1),
                             InvalidationLevel::Valid}
    , keepAspectRatio_{"keepAspectRatio", "Lock Aspect Ratio",
                       "Use the same aspect ratio for separate image size as for the canvas"_help,
                       true, InvalidationLevel::Valid}
    , aspectRatioScaling_{"aspectRatioScaling",
                          "Image Scale",
                          "Scale factor for the separate image size as a factor of canvas size"_help,
                          1.f,
                          {0.1f, ConstraintBehavior::Ignore},
                          {4.f, ConstraintBehavior::Ignore},
                          0.01f,
                          InvalidationLevel::Valid}
    , position_{"position", "Canvas Position",
                util::ordinalSymmetricVector(ivec2(128, 128), 10000)
                    .set(InvalidationLevel::Valid)
                    .set("Position of the canvas on the screen"_help)}
    , visible_{"visible", "Visible", true, InvalidationLevel::Valid}
    , visibleLayer_{"visibleLayer",
                    "Visible Layer",
                    "Select which image layer that should be shown in the canvas. defaults to "
                    "the first color layer"_help,
                    {{"color", "Color layer", LayerType::Color},
                     {"depth", "Depth layer", LayerType::Depth},
                     {"picking", "Picking layer", LayerType::Picking}},
                    0}
    , colorLayer_{"colorLayer_",
                  "Color Layer ID",
                  "Index of the color layer to show in the canvas"_help,
                  0,
                  {0, ConstraintBehavior::Immutable},
                  {0, ConstraintBehavior::Mutable}}
    , imageTypeExt_{"fileExt", "Image Type",
                    [app]() {
                        OptionPropertyState<FileExtension> opts{};
                        const auto exts =
                            app->getDataWriterFactory()->getExtensionsForType<Layer>();
                        std::transform(exts.begin(), exts.end(), std::back_inserter(opts.options),
                                       [](auto& ext) {
                                           return OptionPropertyOption<FileExtension>{
                                               ext.toString(), ext.toString(), ext};
                                       });
                        if (const auto it =
                                std::find_if(exts.begin(), exts.end(),
                                             [](auto& ext) { return ext.extension_ == "png"; });
                            it == exts.end()) {
                            opts.selectedIndex = std::distance(exts.begin(), it);
                        }

                        opts.help = "Select an image type for saving, defaults to png"_help;
                        return opts;
                    }()}
    , saveLayerDirectory_{"layerDir", "Output Directory",
                          "Specify a directory to store snapshots to"_help, "", "image"}
    , saveLayerButton_{"saveLayer", "Save Image Layer",
                       "Save an image snapshot to the specified 'Output Directory' using the "
                       "image format specified by 'Image Type' and the current time as name"_help,
                       [this]() { saveImageLayer(); }, InvalidationLevel::Valid}
    , saveLayerToFileButton_{"saveLayerToFile", "Save Image Layer to File...",
                             "Save the current layer to a file. This will open a save dialog to "
                             "specify the output file"_help,
                             [this]() {
                                 if (auto layer = getVisibleLayer()) {
                                     util::saveLayer(*layer);
                                 } else {
                                     log::error("Could not find visible layer");
                                 }
                             },
                             InvalidationLevel::Valid}
    , fullScreen_{"fullscreen", "Toggle Full Screen",
                  "Show the canvas processor widget in fullscreen"_help, false}
    , fullScreenEvent_{"fullscreenEvent",
                       "FullScreen",
                       "Shortcut to toggle fullscreen"_help,
                       [this](Event*) { fullScreen_.set(!fullScreen_); },
                       IvwKey::F,
                       KeyState::Press,
                       KeyModifier::Shift}
    , saveLayerEvent_{"saveLayerEvent",
                      "Save Image Layer",
                      "Shortcut for saving a snapshot, same as 'Save Image Layer'"_help,
                      [this](Event*) { saveImageLayer(); },
                      IvwKey::Undefined,
                      KeyState::Press}
    , allowContextMenu_{"allowContextMenu", "Allow Context Menu",
                        "Show a default context menu in the canvas, can be used to copy the "
                        "image, fit the view, etc."_help,
                        true}
    , evaluateWhenHidden_{"evaluateWhenHidden", "Evaluate When Hidden",
                          "By default a canvas processor with a hidden canvas will not be "
                          "evaluated. And any of its predecessors will also not be evaluated "
                          "unless thy have other successors that are evaluated. This button "
                          "mean that the processor will always be evaluated even if the result "
                          "is not visible. Can be enable if you only want to save images and "
                          "not show the canvas for example"_help,
                          false}
    , previousImageSize_(customInputDimensions_)
    , widgetMetaData_{
          createMetaData<ProcessorWidgetMetaData>(ProcessorWidgetMetaData::classIdentifier)} {
    addPort(inport_);
    widgetMetaData_->addObserver(this);

    setEvaluateWhenHidden(false);

    // this is serialized in the widget metadata
    dimensions_.setSerializationMode(PropertySerializationMode::None);
    position_.setSerializationMode(PropertySerializationMode::None);
    visible_.setSerializationMode(PropertySerializationMode::None);
    fullScreen_.setSerializationMode(PropertySerializationMode::None);

    dimensions_.onChange([this]() { widgetMetaData_->setDimensions(dimensions_.get()); });
    position_.onChange([this]() { widgetMetaData_->setPosition(position_.get()); });
    visible_.onChange([this]() { widgetMetaData_->setVisible(visible_.get()); });
    fullScreen_.onChange([this]() { widgetMetaData_->setFullScreen(fullScreen_.get()); });

    enableCustomInputDimensions_.onChange([this]() { sizeChanged(); });
    customInputDimensions_.onChange([this]() { sizeChanged(); });
    keepAspectRatio_.onChange([this]() { sizeChanged(); });
    aspectRatioScaling_.onChange([this]() { sizeChanged(); });

    keepAspectRatio_.setVisible(false);
    customInputDimensions_.setVisible(false);
    aspectRatioScaling_.setVisible(false);

    inputSize_.addProperties(dimensions_, enableCustomInputDimensions_, customInputDimensions_,
                             keepAspectRatio_, aspectRatioScaling_);

    colorLayer_.setVisible(false);
    colorLayer_.setSerializationMode(PropertySerializationMode::All);

    visibleLayer_.onChange([&]() {
        if (inport_.hasData()) {
            auto layers = inport_.getData()->getNumberOfColorLayers();
            colorLayer_.setVisible(layers > 1 && visibleLayer_.get() == LayerType::Color);
        }
    });

    evaluateWhenHidden_.onChange([this]() { setEvaluateWhenHidden(evaluateWhenHidden_.get()); });

    imageTypeExt_.setSerializationMode(PropertySerializationMode::None);

    addProperties(inputSize_, position_, visible_, visibleLayer_, colorLayer_, saveLayerDirectory_,
                  imageTypeExt_, saveLayerButton_, saveLayerToFileButton_, fullScreen_,
                  fullScreenEvent_, saveLayerEvent_, allowContextMenu_, evaluateWhenHidden_);

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
        processorWidget_->setVisible(false);
        getCanvas()->setEventPropagator(nullptr);
    }
}

void CanvasProcessor::setProcessorWidget(std::unique_ptr<ProcessorWidget> processorWidget) {
    if (processorWidget && !dynamic_cast<CanvasProcessorWidget*>(processorWidget.get())) {
        throw Exception("Expected CanvasProcessorWidget in CanvasProcessor::setProcessorWidget");
    }
    Processor::setProcessorWidget(std::move(processorWidget));
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
    if (widgetMetaData_->isVisible() != visible_.get()) {
        Property::OnChangeBlocker blocker{visible_};
        visible_.set(widgetMetaData_->isVisible());
    }
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
    RenderContext::getPtr()->activateDefaultRenderContext();

    customInputDimensions_.setVisible(enableCustomInputDimensions_);
    customInputDimensions_.setReadOnly(keepAspectRatio_);
    keepAspectRatio_.setVisible(enableCustomInputDimensions_);
    aspectRatioScaling_.setVisible(enableCustomInputDimensions_ && keepAspectRatio_);

    if (keepAspectRatio_) {
        Property::OnChangeBlocker block{customInputDimensions_};  // avoid recursive onChange
        customInputDimensions_.set(calcScaledSize(dimensions_, aspectRatioScaling_));
    }

    ResizeEvent resizeEvent{enableCustomInputDimensions_ ? customInputDimensions_ : dimensions_,
                            previousImageSize_};
    previousImageSize_ = resizeEvent.size();

    inputSize_.invalidate(InvalidationLevel::Valid, &customInputDimensions_);
    inport_.propagateEvent(&resizeEvent);
}

size2_t CanvasProcessor::calcScaledSize(size2_t size, float scale) {
    const int maxDim = size.x >= size.y ? 0 : 1;
    const int minDim = size.x >= size.y ? 1 : 0;

    const double ratio = static_cast<double>(size[minDim]) / static_cast<double>(size[maxDim]);
    size[maxDim] = static_cast<int>(static_cast<double>(size[maxDim]) * scale);
    size[minDim] = static_cast<int>(static_cast<double>(size[maxDim]) * ratio);

    return size;
}

void CanvasProcessor::saveImageLayer() {
    if (saveLayerDirectory_.get().empty()) saveLayerDirectory_.requestFile();

    const auto snapshotPath(
        saveLayerDirectory_.get() /
        (toLower(getIdentifier()) + "-" + currentDateTime() + "." + imageTypeExt_->extension_));
    saveImageLayer(snapshotPath, imageTypeExt_);
}

void CanvasProcessor::saveImageLayer(const std::filesystem::path& snapshotPath,
                                     const FileExtension& extension) {
    if (auto layer = getVisibleLayer()) {
        util::saveLayer(*layer, snapshotPath, extension);
    } else {
        log::error("Could not find visible layer");
    }
}

std::optional<std::filesystem::path> CanvasProcessor::exportFile(
    const std::filesystem::path& path, std::string_view name,
    const std::vector<FileExtension>& candidateExtensions, Overwrite overwrite) const {
    if (auto layer = getVisibleLayer()) {
        return util::saveData(*layer, path, name, candidateExtensions, overwrite);
    } else {
        throw Exception("Could not find visible layer");
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
    if (processorWidget_ && processorWidget_->isVisible()) {
        if (auto c = getCanvas()) {
            c->render(inport_.getData(), visibleLayer_, colorLayer_);
        }
    }
}

void CanvasProcessor::doIfNotReady() {
    if (processorWidget_ && processorWidget_->isVisible()) {
        if (auto c = getCanvas()) {
            c->render(nullptr, visibleLayer_, colorLayer_);
        }
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
        isReady_.setUpdate(getDefaultIsReadyUpdater(this));
    } else {
        isSink_.setUpdate([this]() { return processorWidget_ && processorWidget_->isVisible(); });
        isReady_.setUpdate(
            [this, defaultCheck = getDefaultIsReadyUpdater(this)]() -> ProcessorStatus {
                if (!processorWidget_ || !processorWidget_->isVisible()) {
                    static constexpr std::string_view reason{"Canvas is not visible"};
                    return {ProcessorStatus::NotReady, reason};
                } else {
                    return defaultCheck();
                }
            });
    }
}

}  // namespace inviwo
