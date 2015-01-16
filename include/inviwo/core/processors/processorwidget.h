/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#ifndef IVW_PROCESSORWIDGET_H
#define IVW_PROCESSORWIDGET_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processorwidgetobserver.h>

namespace inviwo {

class Processor;
class ProcessorWidgetMetaData;
    
class IVW_CORE_API ProcessorWidget : public ProcessorWidgetObservable {

public:
    ProcessorWidget();
    virtual ~ProcessorWidget();

    virtual void setProcessor(Processor* processor);
    virtual Processor* getProcessor();

    virtual ProcessorWidget* create() const = 0;
    virtual void initialize();
    virtual void deinitialize();

    virtual bool isVisible();
    virtual void setVisible(bool visible);
    virtual void show();
    virtual void hide();
    
    virtual glm::ivec2 getDimensions();
    virtual void setDimensions(ivec2);
    virtual glm::ivec2 getPosition();
    virtual void setPosition(ivec2);
    
protected:
    Processor* processor_; //< non owning reference.
    ProcessorWidgetMetaData* metaData_; //< non owning reference.
};

} // namespace

#endif // IVW_PROCESSORWIDGET_H
