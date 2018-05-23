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

#include <inviwo/core/rendering/datavisualizermanager.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/fileextension.h>

namespace inviwo {

DataVisualizerManager::DataVisualizerManager() {}

void DataVisualizerManager::registerObject(DataVisualizer* visualizer) {
    visualizers_.push_back(visualizer);
}

void DataVisualizerManager::unRegisterObject(DataVisualizer* visualizer) {
    util::erase_remove(visualizers_, visualizer);
}

std::vector<FileExtension> DataVisualizerManager::getSupportedFileExtensions() const {
    std::vector<FileExtension> res;
    for (auto& visualizer : visualizers_) {
        auto exts = visualizer->getSupportedFileExtensions();
        res.insert(res.end(), exts.begin(), exts.end());
    }
    return res;
}

std::vector<DataVisualizer*> DataVisualizerManager::getDataVisualizersForExtension(
    const std::string& ext) const {
    return util::copy_if(visualizers_, [&](auto& visualizer) {
        return util::contains_if(visualizer->getSupportedFileExtensions(), [&](const auto& item) {
            return item.extension_ == ext;
        });
    });
}

std::vector<DataVisualizer*> DataVisualizerManager::getDataVisualizersForOutport(
    const Outport* port) const {
    return util::copy_if(visualizers_,
                         [&](auto& visualizer) { return visualizer->isOutportSupported(port); });
}

}  // namespace inviwo
