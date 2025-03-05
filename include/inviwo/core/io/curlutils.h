/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <string_view>
#include <filesystem>

namespace inviwo::net {

enum class ResponseCode : long {
    Continue = 100,
    Switching_Protocols = 101,
    Processing = 102,
    Early_Hints = 103,
    OK = 200,
    Created = 201,
    Accepted = 202,
    Non_Authoritative_Information = 203,
    No_Content = 204,
    Reset_Content = 205,
    Partial_Content = 206,
    Multi_Status = 207,
    Already_Reported = 208,
    This_is_fine = 218,
    IM_Used = 226,
    Multiple_Choices = 300,
    Moved_Permanently = 301,
    Found = 302,
    See_Other = 303,
    Not_Modified = 304,
    Switch_Proxy = 306,
    Temporary_Redirect = 307,
    Resume_Incomplete = 308,
    Bad_Request = 400,
    Unauthorized = 401,
    Payment_Required = 402,
    Forbidden = 403,
    Not_Found = 404,
    Method_Not_Allowed = 405,
    Not_Acceptable = 406,
    Proxy_Authentication_Required = 407,
    Request_Timeout = 408,
    Conflict = 409,
    Gone = 410,
    Length_Required = 411,
    Precondition_Failed = 412,
    Request_Entity_Too_Large = 413,
    Request_URI_Too_Long = 414,
    Unsupported_Media_Type = 415,
    Requested_Range_Not_Satisfiable = 416,
    Expectation_Failed = 417,
    Im_a_teapot = 418,
    Page_Expired = 419,
    Method_Failure = 420,
    Misdirected_Request = 421,
    Unprocessable_Entity = 422,
    Locked = 423,
    Failed_Dependency = 424,
    Upgrade_Required = 426,
    Precondition_Required = 428,
    Too_Many_Requests = 429,
    Request_Header_Fields_Too_Large = 431,
    Login_Time_out = 440,
    Connection_Closed_Without_Response = 444,
    Retry_With = 449,
    Blocked_by_Windows_Parental_Controls = 450,
    Unavailable_For_Legal_Reasons = 451,
    Request_Header_Too_Large = 494,
    SSL_Certificate_Error = 495,
    SSL_Certificate_Required = 496,
    HTTP_Request_Sent_to_HTTPS_Port = 497,
    Invalid_Token = 498,
    Client_Closed_Request = 499,
    Internal_Server_Error = 500,
    Not_Implemented = 501,
    Bad_Gateway = 502,
    Service_Unavailable = 503,
    Gateway_Timeout = 504,
    HTTP_Version_Not_Supported = 505,
    Variant_Also_Negotiates = 506,
    Insufficient_Storage = 507,
    Loop_Detected = 508,
    Bandwidth_Limit_Exceeded = 509,
    Not_Extended = 510,
    Network_Authentication_Required = 511,
    Unknown_Error = 520,
    Web_Server_Is_Down = 521,
    Connection_Timed_Out = 522,
    Origin_Is_Unreachable = 523,
    A_Timeout_Occurred = 524,
    SSL_Handshake_Failed = 525,
    Invalid_SSL_Certificate = 526,
    Railgun_Listener_to_Origin_Error = 527,
    Origin_DNS_Error = 530,
    Network_Read_Timeout_Error = 598,
};

template <typename Path>
bool isUrl(const Path& path) {
    if constexpr (std::is_same_v<typename Path::value_type, char>) {
        return path.native().starts_with("http://") || path.native().starts_with("https://");
    } else if constexpr (std::is_same_v<typename Path::value_type, wchar_t>) {
        return path.native().starts_with(L"http://") || path.native().starts_with(L"https://");
    }
}

IVW_CORE_API std::string_view format_as(ResponseCode rs);
IVW_CORE_API std::string_view description(ResponseCode rs);

IVW_CORE_API std::filesystem::path downloadAndCacheIfUrl(const std::filesystem::path& url);




}  // namespace inviwo::net
