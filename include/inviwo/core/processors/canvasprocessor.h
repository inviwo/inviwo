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

#ifndef IVW_CANVASPROCESSOR_H
#define IVW_CANVASPROCESSOR_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/directoryproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/eventproperty.h>
#include <inviwo/core/metadata/processorwidgetmetadata.h>
#include <inviwo/core/util/fileextension.h>

namespace inviwo {

class Canvas;
class CanvasProcessorWidget;
class ProcessorNetworkEvaluator;
template <typename T>
class DataWriterType;

class IVW_CORE_API CanvasProcessor : public Processor, public ProcessorWidgetMetaDataObserver {
public:
    CanvasProcessor(InviwoApplication* app);
    virtual ~CanvasProcessor();

    virtual void process() override;
    virtual void doIfNotReady() override;

    void setCanvasSize(size2_t);
    size2_t getCanvasSize() const;

    bool getUseCustomDimensions() const;
    size2_t getCustomDimensions() const;

    void saveImageLayer();
    void saveImageLayer(std::string filePath, const FileExtension& extension = FileExtension());
    const Layer* getVisibleLayer() const;

    std::shared_ptr<const Image> getImage() const;

    virtual void setProcessorWidget(std::unique_ptr<ProcessorWidget> processorWidget) override;
    virtual void propagateEvent(Event* event, Outport* source) override;

    bool isContextMenuAllowed() const;

    /**
     * By default the processor will only evaluate when its canvas is visible.
     * By setting setEvaluateWhenHidden to true, it will be evaluated regardless.
     */
    void setEvaluateWhenHidden(bool option);

protected:
    virtual void onProcessorWidgetPositionChange(ProcessorWidgetMetaData*) override;
    virtual void onProcessorWidgetDimensionChange(ProcessorWidgetMetaData*) override;
    virtual void onProcessorWidgetVisibilityChange(ProcessorWidgetMetaData*) override;

    Canvas* getCanvas() const;

public:
    ImageInport inport_;

    CompositeProperty inputSize_;
    IntSize2Property dimensions_;
    BoolProperty enableCustomInputDimensions_;
    IntSize2Property customInputDimensions_;
    BoolProperty keepAspectRatio_;
    FloatProperty aspectRatioScaling_;
    IntVec2Property position_;
    TemplateOptionProperty<LayerType> visibleLayer_;
    IntProperty colorLayer_;
    TemplateOptionProperty<FileExtension> imageTypeExt_;
    DirectoryProperty saveLayerDirectory_;
    ButtonProperty saveLayerButton_;
    ButtonProperty saveLayerToFileButton_;
    BoolProperty fullScreen_;
    EventProperty fullScreenEvent_;
    EventProperty saveLayerEvent_;

    BoolProperty allowContextMenu_;

private:
    void sizeChanged();
    size2_t calcSize();

    size2_t previousImageSize_;
    ProcessorWidgetMetaData* widgetMetaData_;
};

}  // namespace inviwo

#endif  // IVW_CANVASPROCESSOR_H
