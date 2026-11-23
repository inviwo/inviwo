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

#pragma once

#include <modules/hdf5/hdf5moduledefine.h>

#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/util/settings/settings.h>

namespace inviwo::hdf5 {

class IVW_MODULE_HDF5_API HDF5Settings : public Settings {
public:
    HDF5Settings();

    /**
     * Number of worker threads the HDF5 library may use internally to decompress chunks
     * concurrently within a single read (see H5TSset_internal_threads). 0 or 1 disables the
     * internal thread pool. Requires HDF5 >= 2.3.0, otherwise the setting has no effect.
     */
    IntProperty internalThreads_;

    /**
     * Returns true if the linked HDF5 library supports internal multithreading.
     */
    static bool supportsInternalThreads();

    /**
     * Apply the current internalThreads_ value to the HDF5 library. Must be called while holding
     * Handle::globalMutex().
     */
    void apply() const;
};

}  // namespace inviwo::hdf5
