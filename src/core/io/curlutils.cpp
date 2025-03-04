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

#include <inviwo/core/io/curlutils.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/formatconversion.h>

#include <curlcpp/curl_easy.h>
#include <curlcpp/curl_ios.h>
#include <curlcpp/curl_exception.h>

#include <fmt/format.h>
#include <fmt/std.h>

namespace inviwo::net {

std::string_view format_as(ResponseCode rs) {
    switch (rs) {
        using enum ResponseCode;
        case Continue:
            return "Continue";
        case Switching_Protocols:
            return "Switching Protocols";
        case Processing:
            return "Processing";
        case Early_Hints:
            return "Early Hints";
        case OK:
            return "OK";
        case Created:
            return "Created";
        case Accepted:
            return "Accepted";
        case Non_Authoritative_Information:
            return "Non-Authoritative Information";
        case No_Content:
            return "No Content";
        case Reset_Content:
            return "Reset Content";
        case Partial_Content:
            return "Partial Content";
        case Multi_Status:
            return "Multi-Status";
        case Already_Reported:
            return "Already Reported";
        case This_is_fine:
            return "This is fine (Apache Web Server)";
        case IM_Used:
            return "IM Used";
        case Multiple_Choices:
            return "Multiple Choices";
        case Moved_Permanently:
            return "Moved Permanently";
        case Found:
            return "Found";
        case See_Other:
            return "See Other";
        case Not_Modified:
            return "Not Modified";
        case Switch_Proxy:
            return "Switch Proxy";
        case Temporary_Redirect:
            return "Temporary Redirect";
        case Resume_Incomplete:
            return "Resume Incomplete";
        case Bad_Request:
            return "Bad Request";
        case Unauthorized:
            return "Unauthorized";
        case Payment_Required:
            return "Payment Required";
        case Forbidden:
            return "Forbidden";
        case Not_Found:
            return "Not Found";
        case Method_Not_Allowed:
            return "Method Not Allowed";
        case Not_Acceptable:
            return "Not Acceptable";
        case Proxy_Authentication_Required:
            return "Proxy Authentication Required";
        case Request_Timeout:
            return "Request Timeout";
        case Conflict:
            return "Conflict";
        case Gone:
            return "Gone";
        case Length_Required:
            return "Length Required";
        case Precondition_Failed:
            return "Precondition Failed";
        case Request_Entity_Too_Large:
            return "Request Entity Too Large";
        case Request_URI_Too_Long:
            return "Request-URI Too Long";
        case Unsupported_Media_Type:
            return "Unsupported Media Type";
        case Requested_Range_Not_Satisfiable:
            return "Requested Range Not Satisfiable";
        case Expectation_Failed:
            return "Expectation Failed";
        case Im_a_teapot:
            return "I'm a teapot";
        case Page_Expired:
            return "Page Expired (Laravel Framework)";
        case Method_Failure:
            return "Method Failure (Spring Framework)";
        case Misdirected_Request:
            return "Misdirected Request";
        case Unprocessable_Entity:
            return "Unprocessable Entity";
        case Locked:
            return "Locked";
        case Failed_Dependency:
            return "Failed Dependency";
        case Upgrade_Required:
            return "Upgrade Required";
        case Precondition_Required:
            return "Precondition Required";
        case Too_Many_Requests:
            return "Too Many Requests";
        case Request_Header_Fields_Too_Large:
            return "Request Header Fields Too Large";
        case Login_Time_out:
            return "Login Time-out";
        case Connection_Closed_Without_Response:
            return "Connection Closed Without Response";
        case Retry_With:
            return "Retry With";
        case Blocked_by_Windows_Parental_Controls:
            return "Blocked by Windows Parental Controls";
        case Unavailable_For_Legal_Reasons:
            return "Unavailable For Legal Reasons";
        case Request_Header_Too_Large:
            return "Request Header Too Large";
        case SSL_Certificate_Error:
            return "SSL Certificate Error";
        case SSL_Certificate_Required:
            return "SSL Certificate Required";
        case HTTP_Request_Sent_to_HTTPS_Port:
            return "HTTP Request Sent to HTTPS Port";
        case Invalid_Token:
            return "Invalid Token (Esri)";
        case Client_Closed_Request:
            return "Client Closed Request";
        case Internal_Server_Error:
            return "Internal Server Error";
        case Not_Implemented:
            return "Not Implemented";
        case Bad_Gateway:
            return "Bad Gateway";
        case Service_Unavailable:
            return "Service Unavailable";
        case Gateway_Timeout:
            return "Gateway Timeout";
        case HTTP_Version_Not_Supported:
            return "HTTP Version Not Supported";
        case Variant_Also_Negotiates:
            return "Variant Also Negotiates";
        case Insufficient_Storage:
            return "Insufficient Storage";
        case Loop_Detected:
            return "Loop Detected";
        case Bandwidth_Limit_Exceeded:
            return "Bandwidth Limit Exceeded";
        case Not_Extended:
            return "Not Extended";
        case Network_Authentication_Required:
            return "Network Authentication Required";
        case Unknown_Error:
            return "Unknown Error";
        case Web_Server_Is_Down:
            return "Web Server Is Down";
        case Connection_Timed_Out:
            return "Connection Timed Out";
        case Origin_Is_Unreachable:
            return "Origin Is Unreachable";
        case A_Timeout_Occurred:
            return "A Timeout Occurred";
        case SSL_Handshake_Failed:
            return "SSL Handshake Failed";
        case Invalid_SSL_Certificate:
            return "Invalid SSL Certificate";
        case Railgun_Listener_to_Origin_Error:
            return "Railgun Listener to Origin Error";
        case Origin_DNS_Error:
            return "Origin DNS Error";
        case Network_Read_Timeout_Error:
            return "Network Read Timeout Error";
    }
    return "Unknown response code";
}

std::string_view description(ResponseCode rs) {
    switch (rs) {
        using enum ResponseCode;
        case Continue:
            return "The server has received the request headers, and the client should proceed to "
                   "send the request body.";
        case Switching_Protocols:
            return "The requester has asked the server to switch protocols.";
        case Processing:
            return "This code indicates that the server has received and is processing the "
                   "request, but no response is available yet. This prevents the client from "
                   "timing out and assuming the request was lost.";
        case Early_Hints:
            return "Used to return some response headers before final HTTP message.";
        case OK:
            return "The request is OK (this is the standard response for successful HTTP "
                   "requests).";
        case Created:
            return "The request has been fulfilled, and a new resource is created.";
        case Accepted:
            return "The request has been accepted for processing, but the processing has not been "
                   "completed.";
        case Non_Authoritative_Information:
            return "The request has been successfully processed, but is returning information that "
                   "may be from another source.";
        case No_Content:
            return "The request has been successfully processed, but is not returning any content.";
        case Reset_Content:
            return "The request has been successfully processed, but is not returning any content, "
                   "and requires that the requester reset the document view.";
        case Partial_Content:
            return "The server is delivering only part of the resource due to a range header sent "
                   "by the client.";
        case Multi_Status:
            return "The message body that follows is by default an XML message and can contain a "
                   "number of separate response codes, depending on how many sub-requests were "
                   "made.";
        case Already_Reported:
            return "The members of a DAV binding have already been enumerated in a preceding part "
                   "of the (multistatus) response, and are not being included again.";
        case This_is_fine:
            return "Used as a catch-all error condition for allowing response bodies to flow "
                   "through Apache when ProxyErrorOverride is enabled.";
        case IM_Used:
            return "The server has fulfilled a request for the resource, and the response is a "
                   "representation of the result of one or more instance-manipulations applied to "
                   "the current instance.";
        case Multiple_Choices:
            return "A link list. The user can select a link and go to that location. Maximum five "
                   "addresses.";
        case Moved_Permanently:
            return "The requested page has moved to a new URL.";
        case Found:
            return "The requested page has moved temporarily to a new URL.";
        case See_Other:
            return "The requested page can be found under a different URL.";
        case Not_Modified:
            return "Indicates the requested page has not been modified since last requested.";
        case Switch_Proxy:
            return "No longer used. Originally meant \"Subsequent requests should use the "
                   "specified proxy.\"";
        case Temporary_Redirect:
            return "The requested page has moved temporarily to a new URL.";
        case Resume_Incomplete:
            return "Used in the resumable requests proposal to resume aborted PUT or POST "
                   "requests.";
        case Bad_Request:
            return "The request cannot be fulfilled due to bad syntax.";
        case Unauthorized:
            return "The request was a legal request, but the server is refusing to respond to it. "
                   "For use when authentication is possible but has failed or not yet been "
                   "provided.";
        case Payment_Required:
            return "Not yet implemented by RFC standards, but reserved for future use.";
        case Forbidden:
            return "The request was a legal request, but the server is refusing to respond to it.";
        case Not_Found:
            return "The requested page could not be found but may be available again in the "
                   "future.";
        case Method_Not_Allowed:
            return "A request was made of a page using a request method not supported by that "
                   "page.";
        case Not_Acceptable:
            return "The server can only generate a response that is not accepted by the client.";
        case Proxy_Authentication_Required:
            return "The client must first authenticate itself with the proxy.";
        case Request_Timeout:
            return "The server timed out waiting for the request.";
        case Conflict:
            return "The request could not be completed because of a conflict in the request.";
        case Gone:
            return "The requested page is no longer available.";
        case Length_Required:
            return "The \"Content-Length\" is not defined. The server will not accept the request "
                   "without it.";
        case Precondition_Failed:
            return "The precondition given in the request evaluated to false by the server.";
        case Request_Entity_Too_Large:
            return "The server will not accept the request, because the request entity is too "
                   "large.";
        case Request_URI_Too_Long:
            return "The server will not accept the request, because the URL is too long. Occurs "
                   "when you convert a POST request to a GET request with a long query "
                   "information.";
        case Unsupported_Media_Type:
            return "The server will not accept the request, because the media type is not "
                   "supported.";
        case Requested_Range_Not_Satisfiable:
            return "The client has asked for a portion of the file, but the server cannot supply "
                   "that portion.";
        case Expectation_Failed:
            return "The server cannot meet the requirements of the Expect request-header field.";
        case Im_a_teapot:
            return "Any attempt to brew coffee with a teapot should result in the error code \"418 "
                   "I'm a teapot\". The resulting entity body MAY be short and stout.";
        case Page_Expired:
            return "Used by the Laravel Framework when a CSRF Token is missing or expired.";
        case Method_Failure:
            return "A deprecated response used by the Spring Framework when a method has failed.";
        case Misdirected_Request:
            return "The request was directed at a server that is not able to produce a response "
                   "(for example because a connection reuse).";
        case Unprocessable_Entity:
            return "The request was well-formed but was unable to be followed due to semantic "
                   "errors.";
        case Locked:
            return "The resource that is being accessed is locked.";
        case Failed_Dependency:
            return "The request failed due to failure of a previous request (e.g., a PROPPATCH).";
        case Upgrade_Required:
            return "The client should switch to a different protocol such as TLS 1.0, given in "
                   "the Upgrade header field.";
        case Precondition_Required:
            return "The origin server requires the request to be conditional.";
        case Too_Many_Requests:
            return "The user has sent too many requests in a given amount of time. Intended for "
                   "use with rate limiting schemes.";
        case Request_Header_Fields_Too_Large:
            return "The server is unwilling to process the request because either an individual "
                   "header field, or all the header fields collectively, are too large.";
        case Login_Time_out:
            return "The client's session has expired and must log in again. (IIS)";
        case Connection_Closed_Without_Response:
            return "A non-standard status code used to instruct nginx to close the connection "
                   "without sending a response to the client, most commonly used to deny malicious "
                   "or malformed requests.";
        case Retry_With:
            return "The server cannot honour the request because the user has not provided the "
                   "required information. (IIS)";
        case Blocked_by_Windows_Parental_Controls:
            return "The Microsoft extension code indicated when Windows Parental Controls are "
                   "turned on and are blocking access to the requested webpage.";
        case Unavailable_For_Legal_Reasons:
            return "A server operator has received a legal demand to deny access to a resource or "
                   "to a set of resources that includes the requested resource.";
        case Request_Header_Too_Large:
            return "Used by nginx to indicate the client sent too large of a request or header "
                   "line that is too long.";
        case SSL_Certificate_Error:
            return "An expansion of the 400 Bad Request response code, used when the client has "
                   "provided an invalid client certificate.";
        case SSL_Certificate_Required:
            return "An expansion of the 400 Bad Request response code, used when a client "
                   "certificate is required but not provided.";
        case HTTP_Request_Sent_to_HTTPS_Port:
            return "An expansion of the 400 Bad Request response code, used when the client has "
                   "made a HTTP request to a port listening for HTTPS requests.";
        case Invalid_Token:
            return "Returned by ArcGIS for Server. Code 498 indicates an expired or otherwise "
                   "invalid token.";
        case Client_Closed_Request:
            return "A non-standard status code introduced by nginx for the case when a client "
                   "closes the connection while nginx is processing the request.";
        case Internal_Server_Error:
            return "An error has occurred in a server side script, a no more specific message is "
                   "suitable.";
        case Not_Implemented:
            return "The server either does not recognize the request method, or it lacks the "
                   "ability to fulfill the request.";
        case Bad_Gateway:
            return "The server was acting as a gateway or proxy and received an invalid response "
                   "from the upstream server.";
        case Service_Unavailable:
            return "The server is currently unavailable (overloaded or down).";
        case Gateway_Timeout:
            return "The server was acting as a gateway or proxy and did not receive a timely "
                   "response from the upstream server.";
        case HTTP_Version_Not_Supported:
            return "The server does not support the HTTP protocol version used in the request.";
        case Variant_Also_Negotiates:
            return "Transparent content negotiation for the request results in a circular "
                   "reference.";
        case Insufficient_Storage:
            return "The server is unable to store the representation needed to complete the "
                   "request.";
        case Loop_Detected:
            return "The server detected an infinite loop while processing the request (sent "
                   "instead of 208 Already Reported).";
        case Bandwidth_Limit_Exceeded:
            return "The server has exceeded the bandwidth specified by the server administrator; "
                   "this is often used by shared hosting providers to limit the bandwidth of "
                   "customers.";
        case Not_Extended:
            return "Further extensions to the request are required for the server to fulfil it.";
        case Network_Authentication_Required:
            return "The client needs to authenticate to gain network access.";
        case Unknown_Error:
            return "The 520 error is used as a \"catch-all response for when the origin server "
                   "returns something unexpected\", listing connection resets, large headers, and "
                   "empty or invalid responses as common triggers.";
        case Web_Server_Is_Down:
            return "The origin server has refused the connection from Cloudflare.";
        case Connection_Timed_Out:
            return "Cloudflare could not negotiate a TCP handshake with the origin server.";
        case Origin_Is_Unreachable:
            return "Cloudflare could not reach the origin server; for example, if the DNS records "
                   "for the origin server are incorrect.";
        case A_Timeout_Occurred:
            return "Cloudflare was able to complete a TCP connection to the origin server, but did "
                   "not receive a timely HTTP response.";
        case SSL_Handshake_Failed:
            return "Cloudflare could not negotiate a SSL/TLS handshake with the origin server.";
        case Invalid_SSL_Certificate:
            return "Used by Cloudflare and Cloud Foundry's gorouter to indicate failure to "
                   "validate the SSL/TLS certificate that the origin server presented.";
        case Railgun_Listener_to_Origin_Error:
            return "Error 527 indicates that the request timed out or failed after the WAN "
                   "connection had been established.";
        case Origin_DNS_Error:
            return "Error 530 indicates that the requested host name could not be resolved on the "
                   "Cloudflare network to an origin server.";
        case Network_Read_Timeout_Error:
            return "Used by some HTTP proxies to signal a network read timeout behind the proxy to "
                   "a client in front of the proxy.";
    }
    return "Unknown response code";
}

std::filesystem::path downloadAndCacheIfUrl(const std::filesystem::path& url) {
    if (!net::isUrl(url)) {
        return url;
    }

    auto cacheDir = filesystem::getPath(PathType::Cache);
    std::filesystem::create_directories(cacheDir);

    const auto file = cacheDir / fmt::format("{:x}-{}", std::hash<std::filesystem::path>{}(url),
                                             url.filename().generic_string());

    if (std::filesystem::is_regular_file(file)) {
        log::info("Url {:?g} found in cache: {:?g}", url, file);
        return file;
    }

    std::ofstream myfile{file, std::ios::out | std::ios::binary};

    curl::curl_ios<std::ostream> writer(myfile);
    curl::curl_easy easy(writer);

    easy.add<CURLOPT_URL>(url.generic_string().c_str());
    easy.add<CURLOPT_FOLLOWLOCATION>(1L);

    try {
        // Execute the request
        log::info("Downloading {:?g} to cache: {:?g}", url, file);
        easy.perform();

        const auto response = net::ResponseCode{easy.get_info<CURLINFO_RESPONSE_CODE>().get()};
        if (response != net::ResponseCode::OK) {
            throw Exception(SourceContext{}, "Error downloading {:?g}: {}: {}", url, response,
                            description(response));
        }

        auto downloaded = easy.get_info<CURLINFO_SIZE_DOWNLOAD>().get();
        auto speed = easy.get_info<CURLINFO_SPEED_DOWNLOAD>().get();
        auto time = easy.get_info<CURLINFO_TOTAL_TIME>().get();

        log::info("Downloaded {} in {}s at {}/s",
                  util::formatBytesToString(static_cast<size_t>(downloaded)), time,
                  util::formatBytesToString(static_cast<size_t>(speed)));

        return file;

    } catch (const curl::curl_easy_exception& error) {
        throw Exception(SourceContext{}, "Error downloading {:?g}: {}", url, error.what());
    }
}

}  // namespace inviwo::net
