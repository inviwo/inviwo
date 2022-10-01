/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2022 Inviwo Foundation
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

#include <modules/python3/processors/numpymandelbrot.h>

#include <pybind11/cast.h>     // for cast
#include <pybind11/pytypes.h>  // for object

#include <inviwo/core/common/inviwoapplication.h>      // for InviwoApplication
#include <inviwo/core/common/modulepath.h>             // for ModulePath, ModulePath::Scripts
#include <inviwo/core/datastructures/image/image.h>    // for Image
#include <inviwo/core/ports/imageport.h>               // for ImageOutport
#include <inviwo/core/processors/processor.h>          // for Processor
#include <inviwo/core/processors/processorinfo.h>      // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>     // for CodeState, CodeState::Experimental
#include <inviwo/core/properties/invalidationlevel.h>  // for InvalidationLevel, InvalidationLev...
#include <inviwo/core/properties/minmaxproperty.h>     // for FloatMinMaxProperty
#include <inviwo/core/properties/ordinalproperty.h>    // for IntSize2Property, FloatProperty
#include <inviwo/core/util/formats.h>                  // for DataFloat32
#include <inviwo/core/util/glmvec.h>                   // for size2_t
#include <modules/python3/python3module.h>             // for Python3Module
#include <modules/python3/pythonscript.h>              // for PythonScriptDisk

#include <functional>   // for __base
#include <memory>       // for make_shared, shared_ptr
#include <string>       // for string, hash, operator+, operator==
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t

namespace inviwo {

const ProcessorInfo NumpyMandelbrot::processorInfo_{
    "org.inviwo.NumpyMandelbrot",  // Class identifier
    "NumPy Mandelbrot",            // Display name
    "NumPy",                       // Category
    CodeState::Experimental,       // Code state
    {"Python"},                    // Tags
};
const ProcessorInfo NumpyMandelbrot::getProcessorInfo() const { return processorInfo_; }

NumpyMandelbrot::NumpyMandelbrot(InviwoApplication* app)
    : Processor()
    , outport_("outport", false)
    , size_("size", "Size", size2_t(600, 440), size2_t(32), size2_t(2048))
    , realBounds_("realBounds", "Real bounds", -2.f, 1.f, -3.f, 3.f)
    , imaginaryBound_("imaginaryBound", "Imaginary bounds", -1.1f, 1.1f, -3.f, 3.f)
    , power_("power", "power", 2.f, 0.01f, 10.f, 0.01f)
    , iterations_("iterations", "Iterations", 10, 1, 1000)

    , script_(app->getModuleByType<Python3Module>()->getPath(ModulePath::Scripts) +
              "/mandelbrot.py") {

    addPort(outport_);
    addProperty(size_);
    addProperty(realBounds_);
    addProperty(imaginaryBound_);
    addProperty(power_);
    addProperty(iterations_);

    script_.onChange([&]() { invalidate(InvalidationLevel::InvalidOutput); });
}

void NumpyMandelbrot::process() {
    auto img = std::make_shared<Image>(size_.get(), DataFloat32::get());
    script_.run({{"img", pybind11::cast(img->getColorLayer())},
                 {"p", pybind11::cast(static_cast<Processor*>(this))}});

    outport_.setData(img);
}

}  // namespace inviwo
