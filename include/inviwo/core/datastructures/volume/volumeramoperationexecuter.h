/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_VOLUMERAMOPERATIONEXECUTER_H
#define IVW_VOLUMERAMOPERATIONEXECUTER_H

#include <inviwo/core/datastructures/dataoperation.h>
#include <inviwo/core/datastructures/volume/volumeramhistogram.h>
#include <inviwo/core/datastructures/volume/volumeramslice.h>
#include <inviwo/core/datastructures/volume/volumeramsubset.h>
#include <inviwo/core/datastructures/volume/volumeramsubsample.h>

namespace inviwo {

//TODO: Make operation factory
template<typename T, size_t B>
void executeOperationOnVolumeRAMPrecision(DataOperation* dop) {
    VolumeRAMNormalizedHistogram* volNormalizedHistogram = dynamic_cast<VolumeRAMNormalizedHistogram*>(dop);

    if (volNormalizedHistogram) {
        volNormalizedHistogram->evaluate<T, B>();
        return;
    }

    VolumeRAMSubSet* volSubSetDop = dynamic_cast<VolumeRAMSubSet*>(dop);

    if (volSubSetDop) {
        volSubSetDop->evaluate<T, B>();
        return;
    }

    VolumeRAMSubSample* volSubSampleDop = dynamic_cast<VolumeRAMSubSample*>(dop);

    if (volSubSampleDop) {
        volSubSampleDop->evaluate<T, B>();
        return;
    }

    VolumeRAMSlice* volSliceDop = dynamic_cast<VolumeRAMSlice*>(dop);

    if (volSliceDop) {
        volSliceDop->evaluate<T, B>();
        return;
    }
};

} // namespace

#endif // IVW_VOLUMERAMOPERATIONEXECUTER_H
