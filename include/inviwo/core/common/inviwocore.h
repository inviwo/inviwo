/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/util/fileobserver.h>
#include <inviwo/core/util/filesystem.h>

#include <inviwo/core/io/transferfunctionlayerreader.h>
#include <inviwo/core/io/transferfunctionlayerwriter.h>

namespace inviwo {

class InviwoApplication;

/**
 * \class InviwoCore
 * \brief Module which registers all module related functionality available in the core.
 */
class IVW_CORE_API InviwoCore : public InviwoModule {
public:
    InviwoCore(InviwoApplication* app);

    virtual const std::filesystem::path& getPath() const override;

private:
    class Observer : public FileObserver {
    public:
        Observer(InviwoCore& core, InviwoApplication* app);
        virtual void fileChanged(const std::filesystem::path& dir) override;

    private:
        InviwoCore& core_;
    };
    void scanDirForComposites(const std::filesystem::path& dir);

    Observer compositeDirObserver_;
    std::unordered_set<std::filesystem::path, PathHash> addedCompositeFiles_;

    TransferFunctionLayerWriterWrapper tfLayerWriters_;
    TransferFunctionLayerReaderWrapper tfLayerReaders_;
};

}  // namespace inviwo
