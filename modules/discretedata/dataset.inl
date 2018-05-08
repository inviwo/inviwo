/*********************************************************************************
*
* Inviwo - Interactive Visualization Workshop
*
* Copyright (c) 2012-2018 Inviwo Foundation
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

namespace inviwo {
namespace dd {

template <typename T>
std::shared_ptr<const DataChannel<T>> DataChannelMap::getFirstChannel() const {
    SharedConstChannel channel = getFirstChannel();

    return std::dynamic_pointer_cast<const DataChannel<T>, const Channel>(channel);
}

template <typename T>
std::shared_ptr<const DataChannel<T>> DataChannelMap::getChannel(const std::string& name,
                                                                 GridPrimitive definedOn) const {
    return getChannel(name.c_str(), definedOn);
}

template <typename T>
std::shared_ptr<const DataChannel<T>> DataChannelMap::getChannel(const char* name,
                                                                 GridPrimitive definedOn) const {
    SharedConstChannel channel = getChannel(name, definedOn);

    return std::dynamic_pointer_cast<const DataChannel<T>, const Channel>(channel);
}

template <typename T>
std::shared_ptr<const BufferChannel<T>> DataChannelMap::getAsBuffer(const std::string& name,
                                                                    GridPrimitive definedOn) const {
    SharedConstChannel channel = getChannel(name, definedOn);

    // Data is not present in this data set.
    if (!channel.get()) return std::shared_ptr<const BufferChannel<T>>();

    // Try to cast the channel to DataChannel<T>.
    // Return empty shared_ptr if unsuccessfull.
    std::shared_ptr<const DataChannel<T>> dataChannel =
        std::dynamic_pointer_cast<const DataChannel<T>, const Channel>(channel);

    // Check for nullptr inside
    if (!dataChannel.get()) return std::shared_ptr<const BufferChannel<T>>();

    // Try to cast the channel to buffer directly.
    // If successfull, return a shared pointer to the buffer directly.
    // (Shared pointer remains valid and shares the reference counter).
    std::shared_ptr<const BufferChannel<T>> bufferChannel =
        std::dynamic_pointer_cast<const BufferChannel<T>, const Channel>(channel);

    // Check for nullptr inside.
    if (bufferChannel.get()) return bufferChannel;

    // Copy data over.
    BufferChannel<T>* buffer = new BufferChannel<T>(
        dataChannel->getNumElements(), dataChannel->getNumComponents(), name, definedOn);
    for (ind element = 0; element < dataChannel->getNumElements(); ++element)
        dataChannel->fill(buffer->get(element), element);

    buffer->copyMetaDataFrom(*dataChannel.get());

    return std::shared_ptr<const BufferChannel<T>>(buffer);
}

}  // namespace
}
