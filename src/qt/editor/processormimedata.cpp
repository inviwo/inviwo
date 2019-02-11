/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <inviwo/qt/editor/processormimedata.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QByteArray>
#include <QDataStream>
#include <warn/pop>

namespace inviwo {

ProcessorMimeData::ProcessorMimeData(std::unique_ptr<Processor> processor)
    : QMimeData{}, processor_{std::move(processor)} {

    auto cid = utilqt::toQString(processor_->getClassIdentifier());

    QByteArray byteData;
    {
        QDataStream ds(&byteData, QIODevice::WriteOnly);
        ds << cid;
    }
    setData(utilqt::toQString(getMimeTag()), byteData);
    setText(cid);
}

std::unique_ptr<Processor> ProcessorMimeData::get() const { return std::move(processor_); }

Processor* ProcessorMimeData::processor() const { return processor_.get(); }

const std::string& ProcessorMimeData::getMimeTag() {
    static std::string tag{"application/x.vnd.inviwo.processor+txt"};
    return tag;
}

const ProcessorMimeData* ProcessorMimeData::toProcessorMimeData(const QMimeData* data) {
    return qobject_cast<const ProcessorMimeData*>(data);
}

}  // namespace inviwo
