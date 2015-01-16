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

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/processors/processorfactory.h>
#include <inviwo/core/io/serialization/ivwserializable.h>
#include <inviwo/core/util/stringconversion.h>

namespace inviwo {

ProcessorFactory::ProcessorFactory() {}

ProcessorFactory::~ProcessorFactory() {}

void ProcessorFactory::registerObject(ProcessorFactoryObject* processor) {
    if (processorClassMap_.find(processor->getClassIdentifier()) == processorClassMap_.end())
        processorClassMap_.insert(std::make_pair(processor->getClassIdentifier(), processor));
    else
        LogWarn("Processor with class name: " << processor->getClassIdentifier() << " is already registerd");

    if(splitString(processor->getClassIdentifier(), '.').size()<3){
        LogWarn("All processor classIdentifiers should be named using reverse DNS (org.inviwo.processor) not like: " << processor->getClassIdentifier())
    }
}

IvwSerializable* ProcessorFactory::create(const std::string &classIdentifier) const {
    std::map<std::string, ProcessorFactoryObject*>::iterator it = processorClassMap_.find(classIdentifier);

    if (it != processorClassMap_.end()) {
        return it->second->create();
    } 
    // Temp fallback
    std::vector<std::string> list = splitString(classIdentifier, '.');
    it = processorClassMap_.find(list[list.size()-1]);
    if (it != processorClassMap_.end())
        return it->second->create();

    return NULL;
}

bool ProcessorFactory::isValidType(const std::string &classIdentifier) const {
    std::map<std::string, ProcessorFactoryObject*>::iterator it = processorClassMap_.find(classIdentifier);

    if (it != processorClassMap_.end())
        return true;
    
    // Temp fallback
    std::vector<std::string> list = splitString(classIdentifier, '.');
    it = processorClassMap_.find(list[list.size() - 1]);
    if (it != processorClassMap_.end())
       return true;

    return false;
}

} // namespace
