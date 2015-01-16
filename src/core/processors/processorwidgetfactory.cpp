/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/processors/processorwidgetfactory.h>


namespace inviwo {

ProcessorWidgetFactory::ProcessorWidgetFactory() {}

ProcessorWidgetFactory::~ProcessorWidgetFactory() {}

void ProcessorWidgetFactory::registerObject(std::pair<std::string, ProcessorWidget* > widget) {
    if (processorWidgetMap_.find(widget.first) == processorWidgetMap_.end())
        processorWidgetMap_.insert(widget);
    else
        LogWarn("Processor Widget for class name: " << widget.first << " is already registerd");
}

ProcessorWidget* ProcessorWidgetFactory::create(std::string processorClassName) const {
    ProcessorWidgetMap::iterator it = processorWidgetMap_.find(processorClassName);

    if (it != processorWidgetMap_.end())
        return it->second->create();
    else
        return NULL;
}

ProcessorWidget* ProcessorWidgetFactory::create(Processor* processor) const {
    return ProcessorWidgetFactory::create(processor->getClassIdentifier());
}

bool ProcessorWidgetFactory::isValidType(std::string processorClassName) const {
    ProcessorWidgetMap::iterator it = processorWidgetMap_.find(processorClassName);

    if (it != processorWidgetMap_.end())
        return true;
    else
        return false;
}


} // namespace
