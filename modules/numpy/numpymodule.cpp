/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#include <modules/numpy/numpymodule.h>



#include <modules/python3/pythonincluder.h>

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#define PY_ARRAY_UNIQUE_SYMBOL NUMPY_ARRAY_API
#undef NO_IMPORT_ARRAY 
#undef NO_IMPORT
#include <arrayobject.h>
#include <modules/numpy/processors/numpymandelbrot.h>
#include <modules/numpy/processors/numpyvolume.h>
#include <modules/numpy/processors/numpymeshcreatetest.h>


namespace inviwo {

NumPyModule::NumPyModule(InviwoApplication* app) : InviwoModule(app, "NumPy") {   
    registerProcessor<NumpyMandelbrot>();
    registerProcessor<NumPyVolume>();
    registerProcessor<NumPyMeshCreateTest>();
    


    if (Py_IsInitialized()) {
        if (_import_array() < 0) {
            LogWarn("Numpy failed to initialize");
        }
    }else{
        LogError("Python is not initialized");
        throw Exception("Python is not initialized",IvwContext);
    }
}

} // namespace
