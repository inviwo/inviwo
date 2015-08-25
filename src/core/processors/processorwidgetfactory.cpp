/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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

bool ProcessorWidgetFactory::registerObject(std::pair<std::string, ProcessorWidget* > widget) {
    if (util::insert_unique(map_, widget.first, widget.second)) {
        return true;
    } else {
        LogWarn("Processor Widget for class name: " << widget.first << " is already registerd");
        return false;
    }        
}

std::unique_ptr<ProcessorWidget> ProcessorWidgetFactory::create(const std::string& key) const {
    return std::unique_ptr<ProcessorWidget>(
        util::map_find_or_null(map_, key, [](ProcessorWidget* o) { return o->create(); }));
}

std::unique_ptr<ProcessorWidget> ProcessorWidgetFactory::create(Processor* processor) const {
    return ProcessorWidgetFactory::create(processor->getClassIdentifier());
}

bool ProcessorWidgetFactory::hasKey(const std::string& processorClassName) const {
    return util::has_key(map_, processorClassName);
}


} // namespace
