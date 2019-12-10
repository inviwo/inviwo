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

#include <inviwo/dataframeqt/dataframeqtmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/rendering/datavisualizer.h>

namespace inviwo {

class IVW_MODULE_DATAFRAMEQT_API DataFrameTableVisualizer : public DataVisualizer {
public:
    DataFrameTableVisualizer(InviwoApplication* app);
    virtual ~DataFrameTableVisualizer() = default;

    virtual std::string getName() const override;
    virtual Document getDescription() const override;
    virtual std::vector<FileExtension> getSupportedFileExtensions() const override;
    virtual bool isOutportSupported(const Outport* port) const override;

    virtual bool hasSourceProcessor() const override;
    virtual bool hasVisualizerNetwork() const override;

    virtual std::pair<Processor*, Outport*> addSourceProcessor(
        const std::string& filename, ProcessorNetwork* network) const override;
    virtual std::vector<Processor*> addVisualizerNetwork(Outport* outport,
                                                         ProcessorNetwork* network) const override;
    virtual std::vector<Processor*> addSourceAndVisualizerNetwork(
        const std::string& filename, ProcessorNetwork* network) const override;

private:
    InviwoApplication* app_;
};

}  // namespace inviwo
