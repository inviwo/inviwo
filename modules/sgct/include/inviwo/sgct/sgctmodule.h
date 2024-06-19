/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2024 Inviwo Foundation
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

#include <inviwo/sgct/sgctmoduledefine.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/util/commandlineparser.h>

#include <inviwo/sgct/sgctsettings.h>

#include <memory>

namespace inviwo {

class SGCTWrapper;

class IVW_MODULE_SGCT_API SGCTModule : public InviwoModule {
public:
    explicit SGCTModule(InviwoApplication* app);
    SGCTModule(const SGCTModule&) = delete;
    SGCTModule& operator=(const SGCTModule&) = delete;

    SGCTModule(SGCTModule&&) = delete;
    SGCTModule& operator=(SGCTModule&&) = delete;
    virtual ~SGCTModule();
    SGCTSettings settings;

private:
    std::unique_ptr<SGCTWrapper> sgctWrapper_;
    TCLAP::ValueArg<std::string> configFileArg_;
    CommandLineArgHolder argHolder_;
};

}  // namespace inviwo
