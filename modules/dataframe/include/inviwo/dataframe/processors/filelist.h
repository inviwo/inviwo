/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <inviwo/dataframe/dataframemoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/directoryproperty.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/buttonproperty.h>

#include <inviwo/dataframe/datastructures/dataframe.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>

#include <atomic>

namespace inviwo {

class IVW_MODULE_DATAFRAME_API FileList : public Processor {
public:
    FileList();
    FileList(const FileList&) = delete;
    FileList(FileList&&) = delete;
    FileList& operator=(const FileList&) = delete;
    FileList& operator=(FileList&&) = delete;
    virtual ~FileList();

    virtual void process() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    void cycleFiles();
    static bool running(std::weak_ptr<Processor> pw);
    static std::vector<std::filesystem::directory_entry> getFiles(std::weak_ptr<Processor> pw);
    static void setIndex(std::weak_ptr<Processor> pw, size_t i, const std::filesystem::path& path);

    DataOutport<DataFrame> outport_;
    BrushingAndLinkingInport bnlInport_;
    BrushingAndLinkingOutport bnlOutport_;
    DirectoryProperty directory_;
    ButtonProperty refresh_;
    StringProperty filter_;
    IntSizeTProperty selectedIndex_;
    IntSizeTProperty highlightIndex_;
    std::vector<std::filesystem::directory_entry> files_;

    FileProperty selected_;
    FileProperty highlight_;

    ButtonProperty cycleFiles_;
    std::atomic<bool> running_;
};

}  // namespace inviwo
