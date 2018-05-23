/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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

#ifndef IVW_DATAVISUALIZERMANAGER_H
#define IVW_DATAVISUALIZERMANAGER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/rendering/datavisualizer.h>
#include <memory>

namespace inviwo {

/**
 *
 */
class IVW_CORE_API DataVisualizerManager {
public:
    DataVisualizerManager();
    ~DataVisualizerManager() = default;
    DataVisualizerManager(const DataVisualizerManager&) = delete;
    DataVisualizerManager(DataVisualizerManager&&) = default;
    DataVisualizerManager& operator=(const DataVisualizerManager& that) = delete;
    DataVisualizerManager& operator=(DataVisualizerManager&& that) = default;

    void registerObject(DataVisualizer* visualizer);
    void unRegisterObject(DataVisualizer* visualizer);

    std::vector<FileExtension> getSupportedFileExtensions() const;

    std::vector<DataVisualizer*> getDataVisualizersForExtension(const std::string& ext) const;
    std::vector<DataVisualizer*> getDataVisualizersForOutport(const Outport* port) const;

private:
    std::vector<DataVisualizer*> visualizers_;
};

}  // namespace inviwo

#endif  // IVW_DATAVISUALIZERMANAGER_H
