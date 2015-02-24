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

#include <inviwo/core/processors/processorwidget.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/metadata/processorwidgetmetadata.h>

namespace inviwo {

ProcessorWidget::ProcessorWidget()
    : ProcessorWidgetObservable(), processor_(nullptr), metaData_(nullptr) {}

ProcessorWidget::~ProcessorWidget() {
    if (processor_) {
        processor_->setProcessorWidget(nullptr);
    }
}

void ProcessorWidget::initialize() {
    metaData_ = processor_->createMetaData<ProcessorWidgetMetaData>(
        ProcessorWidgetMetaData::CLASS_IDENTIFIER);
}

void ProcessorWidget::deinitialize() { metaData_ = nullptr; }

void ProcessorWidget::setVisible(bool visible) {
    metaData_->setVisibile(visible);
    if (visible) {
        notifyObserversAboutShow(this);
        if (processor_) processor_->invalidate(INVALID_OUTPUT);
    } else {
        notifyObserversAboutHide(this);
    }
}

bool ProcessorWidget::isVisible() { return metaData_->isVisible(); }

void ProcessorWidget::show() {
    ProcessorWidget::setVisible(true);
}

void ProcessorWidget::hide() {
    ProcessorWidget::setVisible(false);
}
    
Processor* ProcessorWidget::getProcessor() { return processor_; }
void ProcessorWidget::setProcessor(Processor* processor) { processor_ = processor; }

glm::ivec2 ProcessorWidget::getDimensions() { return metaData_->getDimensions(); }
void ProcessorWidget::setDimensions(glm::ivec2 dimensions) { metaData_->setDimensions(dimensions); }

glm::ivec2 ProcessorWidget::getPosition() { return metaData_->getPosition(); }
void ProcessorWidget::setPosition(glm::ivec2 pos) { metaData_->setPosition(pos); }

}  // namespace
