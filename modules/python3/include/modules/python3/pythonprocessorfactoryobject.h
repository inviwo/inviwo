/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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
#pragma once

#include <modules/python3/python3moduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/processors/processorfactoryobject.h>
#include <inviwo/core/util/fileobserver.h>

namespace inviwo {

class InviwoApplication;

struct IVW_MODULE_PYTHON3_API PythonProcessorFactoryObjectData {
    ProcessorInfo info;
    std::string name;
    std::string file;
};

class IVW_MODULE_PYTHON3_API PythonProcessorFactoryObjectBase : public ProcessorFactoryObject {
public:
    PythonProcessorFactoryObjectBase(PythonProcessorFactoryObjectData data);
    virtual ~PythonProcessorFactoryObjectBase() = default;

protected:
    std::string name_;
    std::string file_;
};

class IVW_MODULE_PYTHON3_API PythonProcessorFactoryObject : public PythonProcessorFactoryObjectBase,
                                                            public FileObserver {
public:
    PythonProcessorFactoryObject(InviwoApplication* app, const std::string& file);
    virtual ~PythonProcessorFactoryObject() = default;

    virtual std::unique_ptr<Processor> create(InviwoApplication* app) override;

private:
    InviwoApplication* app_;
    virtual void fileChanged(const std::string& filename) override;

    void reloadProcessors();

    static PythonProcessorFactoryObjectData load(const std::string& file);
};

}  // namespace inviwo
