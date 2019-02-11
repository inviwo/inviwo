/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#ifndef IVW_INTEGRALLINEOPERATIONS_H
#define IVW_INTEGRALLINEOPERATIONS_H

#include <modules/vectorfieldvisualization/vectorfieldvisualizationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/vectorfieldvisualization/datastructures/integralline.h>
#include <modules/vectorfieldvisualization/datastructures/integrallineset.h>

namespace inviwo {

namespace util {
IVW_MODULE_VECTORFIELDVISUALIZATION_API IntegralLine curvature(const IntegralLine &line,
                                                               dmat4 toWorld);
IVW_MODULE_VECTORFIELDVISUALIZATION_API IntegralLineSet curvature(const IntegralLineSet &lines);

IVW_MODULE_VECTORFIELDVISUALIZATION_API void curvature(IntegralLine &line, dmat4 toWorld);
IVW_MODULE_VECTORFIELDVISUALIZATION_API void curvature(IntegralLineSet &lines);

IVW_MODULE_VECTORFIELDVISUALIZATION_API IntegralLine tortuosity(const IntegralLine &line,
                                                                dmat4 toWorld);
IVW_MODULE_VECTORFIELDVISUALIZATION_API IntegralLineSet tortuosity(const IntegralLineSet &lines);

IVW_MODULE_VECTORFIELDVISUALIZATION_API void tortuosity(IntegralLine &line, dmat4 toWorld);
IVW_MODULE_VECTORFIELDVISUALIZATION_API void tortuosity(IntegralLineSet &lines);
}  // namespace util

}  // namespace inviwo

#endif  // IVW_INTEGRALLINECURVATURE_H
