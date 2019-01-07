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

#ifndef IVW_PROCESSORWIDGET_H
#define IVW_PROCESSORWIDGET_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processorwidgetobserver.h>
#include <inviwo/core/metadata/processorwidgetmetadata.h>

namespace inviwo {

class Processor;
/**
 * \ProcessorWidget Base class for widgets connected to a Processor
 */
class IVW_CORE_API ProcessorWidget : public ProcessorWidgetObservable,
                                     public ProcessorWidgetMetaDataObserver {
public:
    ProcessorWidget(Processor* p);
    virtual ~ProcessorWidget() = default;

    virtual Processor* getProcessor() const;

    virtual bool isVisible() const;
    /**
     * Show or hide window.
     * Calls onProcessorWidgetVisibilityChange if state changed.
     * Calls notifyObserversAboutShow if visible = true and then invalidates processor.
     * Calls notifyObserversAboutHide if visible = false.
     */
    virtual void setVisible(bool visible);
    virtual void show();
    virtual void hide();

    virtual glm::ivec2 getDimensions() const;
    virtual void setDimensions(ivec2);
    virtual glm::ivec2 getPosition() const;
    virtual void setPosition(ivec2);

protected:
    virtual void updateVisible(bool visible) = 0;
    virtual void updateDimensions(ivec2) = 0;
    virtual void updatePosition(ivec2) = 0;

    virtual void onProcessorWidgetPositionChange(ProcessorWidgetMetaData*) override;
    virtual void onProcessorWidgetDimensionChange(ProcessorWidgetMetaData*) override;
    virtual void onProcessorWidgetVisibilityChange(ProcessorWidgetMetaData*) override;

    Processor* processor_;               //< non owning reference.
    ProcessorWidgetMetaData* metaData_;  //< non owning reference.
};

}  // namespace inviwo

#endif  // IVW_PROCESSORWIDGET_H
