/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#ifndef IVW_PROCESSORNETWORKMANAGER_H
#define IVW_PROCESSORNETWORKMANAGER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/dispatcher.h>
#include <inviwo/core/util/exception.h>

#include <iostream>

namespace inviwo {

class FactoryBase;

/**
 * The ProcessorNetworkManager handles deserializing and serializing of inviwo workspaces.
 */
class IVW_CORE_API WorkspaceManager { 
    using ClearDispatcher = Dispatcher<void()>;
    using SerializationDispatcher = Dispatcher<void(Serializer&, const ExceptionHandler&)>;
    using DeserializationDispatcher = Dispatcher<void(Deserializer&, const ExceptionHandler&)>;

public:
    using ClearCallback = typename ClearDispatcher::Callback;
    using ClearHandle = typename ClearDispatcher::Handle;
    
    using SerializationCallback = std::function<void(Serializer&)>;
    using SerializationHandle = typename SerializationDispatcher::Handle;
    
    using DeserializationCallback = std::function<void(Deserializer&)>;
    using DeserializationHandle = typename DeserializationDispatcher::Handle;

    WorkspaceManager();
    virtual ~WorkspaceManager();

    void clearWorkspace();

    void saveWorkspace(std::ostream& stream, const std::string& refPath,
                       const ExceptionHandler& exceptionHandler = ExceptionHandler());
    void saveWorkspace(const std::string& path,
                       const ExceptionHandler& exceptionHandler = ExceptionHandler());

    void loadWorkspace(std::istream& stream, const std::string& refPath,
                       const ExceptionHandler& exceptionHandler = ExceptionHandler());
    void loadWorkspace(const std::string& path,
                       const ExceptionHandler& exceptionHandler = ExceptionHandler());

    void registerFactory(FactoryBase* factory);

    ClearHandle addClearCallback(const ClearCallback& callback);
    SerializationHandle addSerializationCallback(const SerializationCallback& callback);
    DeserializationHandle addDeserializationCallback(const DeserializationCallback& callback);

private: 
    std::vector<FactoryBase*> registeredFactories_;

    ClearDispatcher clears_;
    SerializationDispatcher serializers_;
    DeserializationDispatcher deserializers_;
};

} // namespace

#endif // IVW_PROCESSORNETWORKMANAGER_H

