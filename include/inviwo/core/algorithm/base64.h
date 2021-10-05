/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>

#include <string>
#include <string_view>

namespace inviwo {

namespace base64util {

// Copyright © 2004-2017 by René Nyffenegger

// This source code is provided 'as-is', without any express or implied
// warranty. In no event will the author be held liable for any damages
// arising from the use of this software.

// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:

// 1. The origin of this source code must not be misrepresented; you must not
//    claim that you wrote the original source code. If you use this source code
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.

// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original source code.

// 3. This notice may not be removed or altered from any source distribution.

//
//  base64 encoding and decoding with C++.
//  Version: 2.rc.08 (release candidate)
//
//  https://github.com/ReneNyffenegger/cpp-base64
//

IVW_CORE_API std::string base64_encode(std::string const& s, bool url = false);
IVW_CORE_API std::string base64_encode_pem(std::string const& s);
IVW_CORE_API std::string base64_encode_mime(std::string const& s);

IVW_CORE_API std::string base64_decode(std::string const& s, bool remove_linebreaks = false);
IVW_CORE_API std::string base64_encode(unsigned char const*, size_t len, bool url = false);

//
// Interface with std::string_view rather than const std::string&
// Requires C++17
// Provided by Yannic Bonenberger (https://github.com/Yannic)
//
IVW_CORE_API std::string base64_encode(std::string_view s, bool url = false);
IVW_CORE_API std::string base64_encode_pem(std::string_view s);
IVW_CORE_API std::string base64_encode_mime(std::string_view s);

IVW_CORE_API std::string base64_decode(std::string_view s, bool remove_linebreaks = false);

}  // namespace base64util

}  // namespace inviwo
