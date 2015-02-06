/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#ifndef IVW_NODEDEBUGGER_H
#define IVW_NODEDEBUGGER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

namespace inviwo {

struct IVW_CORE_API NodeDebugger {
    struct Node {
        Node(std::string k = "", std::string i = "", std::string t = "", int l = 0)
            : key(k), identifier(i), type(t), line(l) {}
        std::string key;
        std::string identifier;
        std::string type;
        int line;
    };
    Node operator[](std::size_t idx) const {
        if (idx < nodes_.size()) {
            return nodes_[idx];
        } else {
            return Node("UnKnown", "UnKnown");
        }
    }
    std::vector<std::string> getPath() const {
        std::vector<std::string> path;
        for (std::vector<Node>::const_reverse_iterator it = nodes_.rbegin(); it != nodes_.rend();
             ++it) {
            if (!it->identifier.empty()) path.push_back(it->identifier);
        }
        return path;
    }
    size_t size() const { return nodes_.size(); }
    NodeDebugger(TxElement* node);
    std::vector<Node> nodes_;
};

} // namespace

#endif // IVW_NODEDEBUGGER_H

