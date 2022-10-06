/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <inviwo/sgct/sgctmoduledefine.h>

#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/filesystem.h>

#include <sgct/sgct.h>

#include <optional>
#include <string>
#include <vector>
#include <variant>

namespace inviwo {

namespace command {
struct Nop {};
struct AddProcessor {
    std::string data;
};
struct RemoveProcessor {
    std::string data;
};
struct AddConnection {
    std::string data;
};
struct RemoveConnection {
    std::string data;
};
struct AddLink {
    std::string data;
};
struct RemoveLink {
    std::string data;
};
struct Update {
    std::string data;
};
struct Stats {
    bool show = false;
};
}  // namespace command

using SgctCommand =
    std::variant<command::Nop, command::AddProcessor, command::RemoveProcessor,
                 command::AddConnection, command::RemoveConnection, command::AddLink,
                 command::RemoveLink, command::Update, command::Stats>;

namespace util {

inline auto encode(const std::vector<SgctCommand>& commands) -> std::vector<std::byte> {
    std::vector<std::byte> bytes;
    for (const auto& command : commands) {
        sgct::serializeObject(bytes, command.index());
        std::visit(
            util::overloaded{
                [](const command::Nop&) {},
                [&](const command::AddProcessor& update) { sgct::serializeObject(bytes, update.data); },
                [&](const command::RemoveProcessor& update) { sgct::serializeObject(bytes, update.data); },
                [&](const command::AddConnection& update) { sgct::serializeObject(bytes, update.data); },
                [&](const command::RemoveConnection& update) { sgct::serializeObject(bytes, update.data); },
                [&](const command::AddLink& update) { sgct::serializeObject(bytes, update.data); },
                [&](const command::RemoveLink& update) { sgct::serializeObject(bytes, update.data); },
                [&](const command::Update& update) { sgct::serializeObject(bytes, update.data); },
                [&](const command::Stats& stats) { sgct::serializeObject(bytes, stats.show); }},
            command);
    }
    return bytes;
};

inline auto decode(const std::vector<std::byte>& bytes, unsigned int pos)
    -> std::vector<SgctCommand> {
    std::vector<SgctCommand> commands;

    while (pos < bytes.size()) {
        size_t index = 0;
        sgct::deserializeObject(bytes, pos, index);

        switch (index) {
            case 0:  // Nop
                break;
            case 1: {  // AddProcessor
                std::string data;
                sgct::deserializeObject(bytes, pos, data);
                commands.push_back(command::AddProcessor{data});
                break;
            }
            case 2: {  // RemoveProcessor
                std::string data;
                sgct::deserializeObject(bytes, pos, data);
                commands.push_back(command::RemoveProcessor{data});
                break;
            }
            case 3: {  // AddConnection
                std::string data;
                sgct::deserializeObject(bytes, pos, data);
                commands.push_back(command::AddConnection{data});
                break;
            }
            case 4: {  // RemoveConnection
                std::string data;
                sgct::deserializeObject(bytes, pos, data);
                commands.push_back(command::RemoveConnection{data});
                break;
            }
            case 5: {  // AddLink
                std::string data;
                sgct::deserializeObject(bytes, pos, data);
                commands.push_back(command::AddLink{data});
                break;
            }
            case 6: {  // RemoveLink
                std::string data;
                sgct::deserializeObject(bytes, pos, data);
                commands.push_back(command::RemoveLink{data});
                break;
            }
            case 7: {  // Update
                std::string data;
                sgct::deserializeObject(bytes, pos, data);
                commands.push_back(command::Update{data});
                break;
            }
            case 8: {  // Stats
                bool show;
                sgct::deserializeObject(bytes, pos, show);
                commands.push_back(command::Stats{show});
                break;
            }
            default: {
                throw Exception(IVW_CONTEXT_CUSTOM("decode"), "Decode error");
            }
        }
    }
    return commands;
};

}  // namespace util

}  // namespace inviwo
