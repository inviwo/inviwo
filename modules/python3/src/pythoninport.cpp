/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2024 Inviwo Foundation
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

#include <modules/python3/pythoninport.h>

#include <modules/python3/pythonoutport.h>
#include <inviwo/core/util/document.h>

namespace inviwo {

PythonInport::PythonInport(std::string_view identifier, Document help)
    : Inport{identifier, std::move(help)} {}

bool PythonInport::canConnectTo(const Port* port) const {
    if (dynamic_cast<const PythonOutport*>(port)) {
        // add runtime checks
        return true;
    }
    return false;
}

pybind11::object PythonInport::getData() const {
    if (isConnected()) {
        return static_cast<const PythonOutport*>(connectedOutports_[0])->getData();
    } else {
        return {};
    }
}

bool PythonInport::hasData() const {
    if (isConnected()) {
        return static_cast<const PythonOutport*>(connectedOutports_[0])->hasData();
    } else {
        return false;
    }
}

Document PythonInport::getInfo() const {
    Document doc;
    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;
    auto b = doc.append("html").append("body");
    auto p = b.append("p");
    p.append("b", "PythonInport", {{"style", "color:white;"}});
    utildoc::TableBuilder tb(p, P::end());
    tb(H("Identifier"), getIdentifier());
    tb(H("Class"), getClassIdentifier());
    tb(H("Ready"), isReady());
    tb(H("Connected"), isConnected());

    tb(H("Connections"),
       fmt::format("{} ({})", getNumberOfConnections(), getMaxNumberOfConnections()));
    tb(H("Optional"), isOptional());

    if (hasData()) {
        b.append("p", pybind11::str(getData()).cast<std::string>());
    } else {
        b.append("p", "Port has no data");
    }
    return doc;
}

}  // namespace inviwo
