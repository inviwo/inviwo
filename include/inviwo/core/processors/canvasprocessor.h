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

#ifndef IVW_CANVASPROCESSOR_H
#define IVW_CANVASPROCESSOR_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/baseoptionproperty.h>
#include <inviwo/core/properties/directoryproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/compositeproperty.h>

namespace inviwo {

class CanvasProcessorWidget;
class ProcessorNetworkEvaluator;

class IVW_CORE_API CanvasProcessor : public Processor {
public:
    CanvasProcessor();
    virtual ~CanvasProcessor();

    virtual void initialize();
    virtual void deinitialize();

    virtual void process() override;
    virtual void doIfNotReady() override;

    void setCanvasSize(ivec2);
    ivec2 getCanvasSize() const;

    bool getUseCustomDimensions() const;
    ivec2 getCustomDimensions() const;

    void saveImageLayer();
    void saveImageLayer(std::string filePath);
    std::vector<unsigned char>* getLayerAsCodedBuffer(LayerType layerType, const std::string& type,
                                                      size_t idx = 0);
    std::vector<unsigned char>* getColorLayerAsCodedBuffer(const std::string& type, size_t idx = 0);
    std::vector<unsigned char>* getDepthLayerAsCodedBuffer(const std::string& type);
    std::vector<unsigned char>* getPickingLayerAsCodedBuffer(const std::string& type);
    std::vector<unsigned char>* getVisibleLayerAsCodedBuffer(const std::string& type);

    void triggerQueuedEvaluation();
    virtual bool isReady() const;

    virtual bool propagateResizeEvent(ResizeEvent* event, Outport* source) override;

protected:
    void performEvaluationAtNextShow();
    void performEvaluateRequest();

    ImageInport inport_;

    IntVec2Property dimensions_;
    BoolProperty enableCustomInputDimensions_;
    IntVec2Property customInputDimensions_;
    BoolProperty keepAspectRatio_;
    FloatProperty aspectRatioScaling_;
    OptionPropertyInt visibleLayer_;
    IntProperty colorLayer_;
    DirectoryProperty saveLayerDirectory_;
    ButtonProperty saveLayerButton_;
    CompositeProperty inputSize_;

private:
    void resizeCanvas();
    void sizeChanged();
    ivec2 calcSize();

    ivec2 previousImageSize_;

    ProcessorNetworkEvaluator* evaluator_;  //< non-owning reference
    CanvasProcessorWidget* canvasWidget_;   //< non-owning reference
    bool queuedRequest_;
};

}  // namespace

#endif  // IVW_CANVASPROCESSOR_H
