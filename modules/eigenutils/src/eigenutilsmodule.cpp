/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2024 Inviwo Foundation
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

#include <modules/eigenutils/eigenutilsmodule.h>

#include <inviwo/core/common/inviwomodule.h>                   // for InviwoModule
#include <inviwo/core/common/modulepath.h>                     // for ModulePath, ModulePath::Po...
#include <inviwo/core/ports/dataoutport.h>                     // for DataOutport (ptr only)
#include <inviwo/core/ports/outportiterable.h>                 // for OutportIterable, OutportIt...
#include <inviwo/core/util/exception.h>                        // for Exception
#include <inviwo/core/util/glmvec.h>                           // for uvec3
#include <inviwo/core/util/stringconversion.h>                 // for htmlEncode
#include <modules/eigenutils/processors/eigenmatrixtoimage.h>  // for EigenMatrixToImage
#include <modules/eigenutils/processors/eigenmix.h>            // for EigenMix
#include <modules/eigenutils/processors/eigennormalize.h>      // for EigenNormalize
#include <modules/eigenutils/processors/testmatrix.h>          // for TestMatrix

#include <functional>  // for __base
#include <memory>      // for unique_ptr, shared_ptr
#include <string>      // for string, operator+
#include <vector>      // for vector

#include <Eigen/Core>                // for MatrixXf
#include <fmt/core.h>                // for basic_string_view
#include <glm/ext/vector_uint3.hpp>  // for uvec3

namespace inviwo {
class InviwoApplication;

EigenUtilsModule::EigenUtilsModule(InviwoApplication* app) : InviwoModule(app, "EigenUtils") {
    registerProcessor<EigenMatrixToImage>();
    registerProcessor<EigenMix>();
    registerProcessor<EigenNormalize>();
    registerProcessor<TestMatrix>();

    registerDefaultsForScalarDataType<Eigen::MatrixXf>();

    registerPortInspector(PortTraits<DataOutport<Eigen::MatrixXf>>::classIdentifier(),
                          getPath(ModulePath::PortInspectors) / "eigenmatrix.inv");
}

}  // namespace inviwo
