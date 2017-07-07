/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#ifndef IVW_INVIWOPY_H
#define IVW_INVIWOPY_H

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/pytypes.h>

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/canvasprocessor.h>

namespace pybind11 {
namespace detail {
using namespace inviwo;

template <typename T>
using ListCasterBase = pybind11::detail::list_caster<std::vector<T *>, T *>;

template <>
struct type_caster<std::vector<Processor *>> : ListCasterBase<Processor> {
    static handle cast(const std::vector<Processor *> &src, return_value_policy, handle parent) {
        return ListCasterBase<Processor>::cast(src, return_value_policy::reference, parent);
    }
    static handle cast(const std::vector<Processor *> *src, return_value_policy pol,
                       handle parent) {
        return cast(*src, pol, parent);
    }
};

template <>
struct type_caster<std::vector<CanvasProcessor *>> : ListCasterBase<CanvasProcessor> {
    static handle cast(const std::vector<CanvasProcessor *> &src, return_value_policy,
                       handle parent) {
        return ListCasterBase<CanvasProcessor>::cast(src, return_value_policy::reference, parent);
    }
    static handle cast(const std::vector<CanvasProcessor *> *src, return_value_policy pol,
                       handle parent) {
        return cast(*src, pol, parent);
    }
};

template <>
struct type_caster<std::vector<Layer *>> : ListCasterBase<Layer> {
    static handle cast(const std::vector<Layer *> &src, return_value_policy, handle parent) {
        return ListCasterBase<Layer>::cast(src, return_value_policy::reference, parent);
    }
    static handle cast(const std::vector<Layer *> *src, return_value_policy pol, handle parent) {
        return cast(*src, pol, parent);
    }
};
}
}

namespace inviwo {
    class Python3Module;

    void setInviwopyModule(Python3Module *ivwmodule , pybind11::module pymodule);


template <typename T>
pybind11::object propertyToPyObject(T *prop) {
    if (auto cp = dynamic_cast<inviwo::CompositeProperty *>(prop)) {
        return pybind11::cast(cp);
    } else if (auto op = dynamic_cast<inviwo::BaseOptionProperty *>(prop)) {
        return pybind11::cast(op);
    } else {
        return pybind11::cast(prop);
    }
}

template <typename T>
pybind11::object getPropertyById(T &po, std::string key) {
    auto prop = po.getPropertyByIdentifier(key);
    return propertyToPyObject(prop);
}
}

#endif  // IVW_PYPROPERTIES_H
