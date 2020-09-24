/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2020 Inviwo Foundation
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

#include <ivwdiscretedata/ivwdiscretedata.h>

#include <modules/python3/python3module.h>
#include <modules/python3/pybindutils.h>
#include <modules/python3/pythoninterpreter.h>

#include <ivwdiscretedata/pychannels.h>
#include <ivwdiscretedata/pyconnectivities.h>
#include <ivwdiscretedata/pydataset.h>

namespace py = pybind11;

PYBIND11_MODULE(ivwdiscretedata, mod) {

#ifdef IVW_ENABLE_MSVC_MEM_LEAK_TEST
    VLDDisable();
#endif

    py::module::import("inviwopy");

    using namespace inviwo;
    mod.doc() =
        "Python interface for the DiscreteDatamodule:"
        "Channels and a Connectivity are collected in a DataSet.";

    exposeChannels(mod);
    exposeConnectivities(mod);
    exposeDataSet(mod);

#ifdef IVW_ENABLE_MSVC_MEM_LEAK_TEST
    VLDEnable();
#endif
}
