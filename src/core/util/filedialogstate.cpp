/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2023 Inviwo Foundation
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

#include <inviwo/core/util/filedialogstate.h>
#include <inviwo/core/util/exception.h>

#include <ostream>

namespace inviwo {

std::string_view enumToStr(AcceptMode mode) {
    switch (mode) {
        case AcceptMode::Open:
            return "Open";
        case AcceptMode::Save:
            return "Save";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumName"), "Found invalid AcceptMode enum value '{}'",
                    static_cast<int>(mode));
}
std::string_view enumToStr(FileMode mode) {
    switch (mode) {
        case FileMode::AnyFile:
            return "Any File";
        case FileMode::ExistingFile:
            return "Existing File";
        case FileMode::Directory:
            return "Directory";
        case FileMode::ExistingFiles:
            return "Existing Files";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumName"), "Found invalid FileMode enum value '{}'",
                    static_cast<int>(mode));
}

std::ostream& operator<<(std::ostream& ss, AcceptMode& mode) { return ss << enumToStr(mode); }
std::ostream& operator<<(std::ostream& ss, FileMode& mode) { return ss << enumToStr(mode); }

}  // namespace inviwo
