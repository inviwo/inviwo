/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/hdf5/hdf5settings.h>

#include <inviwo/core/properties/constraintbehavior.h>
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/properties/propertysemantics.h>
#include <inviwo/core/util/logcentral.h>

#include <algorithm>
#include <thread>

#include <warn/push>
#include <warn/ignore/all>
#include <hdf5.h>
#include <warn/pop>

#if defined(H5_HAVE_THREADS) && H5_VERSION_GE(2, 3, 0)
#define IVW_HDF5_INTERNAL_THREADS 1
#else
#define IVW_HDF5_INTERNAL_THREADS 0
#endif

namespace inviwo::hdf5 {

namespace {

int defaultThreads() {
    if constexpr (IVW_HDF5_INTERNAL_THREADS) {
        return static_cast<int>(std::max(1u, std::thread::hardware_concurrency()));
    } else {
        return 1;
    }
}

}  // namespace

HDF5Settings::HDF5Settings()
    : Settings("HDF5")
    , internalThreads_("internalThreads", "Internal Decompression Threads",
                       "Number of threads HDF5 uses to decompress chunks concurrently within a "
                       "single read. 0 or 1 disables the internal thread pool. Requires HDF5 >= "
                       "2.3.0."_help,
                       defaultThreads(), {0, ConstraintBehavior::Immutable},
                       {256, ConstraintBehavior::Editable}, 1, InvalidationLevel::Valid,
                       PropertySemantics::Text) {

    addProperty(internalThreads_);
    internalThreads_.setReadOnly(!supportsInternalThreads());

    load();
}

bool HDF5Settings::supportsInternalThreads() { return IVW_HDF5_INTERNAL_THREADS != 0; }

void HDF5Settings::apply() const {
#if IVW_HDF5_INTERNAL_THREADS
    const auto threads = static_cast<unsigned>(std::max(0, internalThreads_.get()));
    if (H5TSset_internal_threads(threads) < 0) {
        log::warn("HDF5: failed to set the number of internal threads to {}", threads);
    } else if (threads > 1) {
        log::info("HDF5: using {} internal threads for chunk decompression", threads);
    } else {
        log::info("HDF5: internal multithreading disabled");
    }
#else
    log::info("HDF5: internal multithreading not available (HDF5 {}.{}.{})", H5_VERS_MAJOR,
              H5_VERS_MINOR, H5_VERS_RELEASE);
#endif
}

}  // namespace inviwo::hdf5
