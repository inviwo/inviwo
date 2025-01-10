/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2025 Inviwo Foundation
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

#include <modules/webbrowser/webbrowserutil.h>

#include <tuple>  // for tuple
#include <cmath>

#include <include/cef_base.h>  // for CefBrowserSettings, CefWindowInfo, STATE_ENABLED
#include <include/cef_parser.h>

namespace inviwo::cefutil {

std::tuple<CefWindowInfo, CefBrowserSettings> getDefaultBrowserSettings() {
    CefWindowInfo windowInfo;

#if defined(WIN32) || defined(__APPLE__)
    windowInfo.SetAsWindowless(nullptr);  // nullptr means no transparency (site background colour)
#else
    windowInfo.SetAsWindowless(0);
#endif

#if defined(WIN32)
    windowInfo.shared_texture_enabled = true;
#endif

    CefBrowserSettings browserSettings;

    return std::tuple<CefWindowInfo, CefBrowserSettings>{windowInfo, browserSettings};
}

double percentageToZoomLevel(double percent) { return std::log(percent) / std::log(1.2); }

double zoomLevelToPercentage(double level) { return std::pow(1.2, level); }

LogLevel logLevel(cef_log_severity_t level) {
    switch (level) {
        case LOGSEVERITY_DISABLE:
            return LogLevel::Error;
        case LOGSEVERITY_FATAL:
            return LogLevel::Error;
        case LOGSEVERITY_ERROR:
            return LogLevel::Error;
        case LOGSEVERITY_WARNING:
            return LogLevel::Warn;
        case cef_log_severity_t::LOGSEVERITY_DEBUG:
            return LogLevel::Info;
        case cef_log_severity_t::LOGSEVERITY_INFO:
            return LogLevel::Info;
        case cef_log_severity_t::LOGSEVERITY_DEFAULT:
            return LogLevel::Info;
        default:
            return LogLevel::Info;
    }
}

std::string getDataURI(const std::string& data, const std::string& mime_type) {
    return "data:" + mime_type + ";base64," +
           CefURIEncode(CefBase64Encode(data.data(), data.size()), false).ToString();
}

std::string getCefErrorString(cef_errorcode_t code) {
    switch (code) {
        case ERR_NONE:
            return "ERR_NONE";
        case ERR_FAILED:
            return "ERR_FAILED";
        case ERR_ABORTED:
            return "ERR_ABORTED";
        case ERR_INVALID_ARGUMENT:
            return "ERR_INVALID_ARGUMENT";
        case ERR_INVALID_HANDLE:
            return "ERR_INVALID_HANDLE";
        case ERR_FILE_NOT_FOUND:
            return "ERR_FILE_NOT_FOUND";
        case ERR_TIMED_OUT:
            return "ERR_TIMED_OUT";
        case ERR_FILE_TOO_BIG:
            return "ERR_FILE_TOO_BIG";
        case ERR_UNEXPECTED:
            return "ERR_UNEXPECTED";
        case ERR_ACCESS_DENIED:
            return "ERR_ACCESS_DENIED";
        case ERR_NOT_IMPLEMENTED:
            return "ERR_NOT_IMPLEMENTED";
        case ERR_CONNECTION_CLOSED:
            return "ERR_CONNECTION_CLOSED";
        case ERR_CONNECTION_RESET:
            return "ERR_CONNECTION_RESET";
        case ERR_CONNECTION_REFUSED:
            return "ERR_CONNECTION_REFUSED";
        case ERR_CONNECTION_ABORTED:
            return "ERR_CONNECTION_ABORTED";
        case ERR_CONNECTION_FAILED:
            return "ERR_CONNECTION_FAILED";
        case ERR_NAME_NOT_RESOLVED:
            return "ERR_NAME_NOT_RESOLVED";
        case ERR_INTERNET_DISCONNECTED:
            return "ERR_INTERNET_DISCONNECTED";
        case ERR_SSL_PROTOCOL_ERROR:
            return "ERR_SSL_PROTOCOL_ERROR";
        case ERR_ADDRESS_INVALID:
            return "ERR_ADDRESS_INVALID";
        case ERR_ADDRESS_UNREACHABLE:
            return "ERR_ADDRESS_UNREACHABLE";
        case ERR_SSL_CLIENT_AUTH_CERT_NEEDED:
            return "ERR_SSL_CLIENT_AUTH_CERT_NEEDED";
        case ERR_TUNNEL_CONNECTION_FAILED:
            return "ERR_TUNNEL_CONNECTION_FAILED";
        case ERR_NO_SSL_VERSIONS_ENABLED:
            return "ERR_NO_SSL_VERSIONS_ENABLED";
        case ERR_SSL_VERSION_OR_CIPHER_MISMATCH:
            return "ERR_SSL_VERSION_OR_CIPHER_MISMATCH";
        case ERR_SSL_RENEGOTIATION_REQUESTED:
            return "ERR_SSL_RENEGOTIATION_REQUESTED";
        case ERR_CERT_COMMON_NAME_INVALID:
            return "ERR_CERT_COMMON_NAME_INVALID";
        case ERR_CERT_DATE_INVALID:
            return "ERR_CERT_DATE_INVALID";
        case ERR_CERT_AUTHORITY_INVALID:
            return "ERR_CERT_AUTHORITY_INVALID";
        case ERR_CERT_CONTAINS_ERRORS:
            return "ERR_CERT_CONTAINS_ERRORS";
        case ERR_CERT_NO_REVOCATION_MECHANISM:
            return "ERR_CERT_NO_REVOCATION_MECHANISM";
        case ERR_CERT_UNABLE_TO_CHECK_REVOCATION:
            return "ERR_CERT_UNABLE_TO_CHECK_REVOCATION";
        case ERR_CERT_REVOKED:
            return "ERR_CERT_REVOKED";
        case ERR_CERT_INVALID:
            return "ERR_CERT_INVALID";
        case ERR_CERT_END:
            return "ERR_CERT_END";
        case ERR_INVALID_URL:
            return "ERR_INVALID_URL";
        case ERR_DISALLOWED_URL_SCHEME:
            return "ERR_DISALLOWED_URL_SCHEME";
        case ERR_UNKNOWN_URL_SCHEME:
            return "ERR_UNKNOWN_URL_SCHEME";
        case ERR_TOO_MANY_REDIRECTS:
            return "ERR_TOO_MANY_REDIRECTS";
        case ERR_UNSAFE_REDIRECT:
            return "ERR_UNSAFE_REDIRECT";
        case ERR_UNSAFE_PORT:
            return "ERR_UNSAFE_PORT";
        case ERR_INVALID_RESPONSE:
            return "ERR_INVALID_RESPONSE";
        case ERR_INVALID_CHUNKED_ENCODING:
            return "ERR_INVALID_CHUNKED_ENCODING";
        case ERR_METHOD_NOT_SUPPORTED:
            return "ERR_METHOD_NOT_SUPPORTED";
        case ERR_UNEXPECTED_PROXY_AUTH:
            return "ERR_UNEXPECTED_PROXY_AUTH";
        case ERR_EMPTY_RESPONSE:
            return "ERR_EMPTY_RESPONSE";
        case ERR_RESPONSE_HEADERS_TOO_BIG:
            return "ERR_RESPONSE_HEADERS_TOO_BIG";
        case ERR_CACHE_MISS:
            return "ERR_CACHE_MISS";
        case ERR_INSECURE_RESPONSE:
            return "ERR_INSECURE_RESPONSE";
        default:
            return "UNKNOWN";
    }
}

}  // namespace inviwo::cefutil
