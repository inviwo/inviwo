/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2024 Inviwo Foundation
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

#include <inviwo/dataframeqt/dataframeqtmodule.h>

#include <inviwo/core/common/inviwomodule.h>                             // for InviwoModule
#include <inviwo/core/rendering/datavisualizer.h>                        // for DataVisualizer
#include <inviwo/dataframeqt/dataframetableprocessorwidget.h>            // for DataFrameTablePr...
#include <inviwo/dataframeqt/datavisualizer/dataframetablevisualizer.h>  // for DataFrameTableVi...
#include <inviwo/dataframeqt/processors/dataframetable.h>                // for DataFrameTable

#include <memory>  // for make_unique, uni...

namespace inviwo {
class InviwoApplication;

DataFrameQtModule::DataFrameQtModule(InviwoApplication* app) : InviwoModule(app, "DataFrameQt") {
    registerProcessor<DataFrameTable>();
    registerProcessorWidget<DataFrameTableProcessorWidget, DataFrameTable>();
    registerDataVisualizer(std::make_unique<DataFrameTableVisualizer>(app));
}

}  // namespace inviwo
