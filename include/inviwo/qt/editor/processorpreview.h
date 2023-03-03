/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2023 Inviwo Foundation
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

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <warn/push>
#include <warn/ignore/all>
#include <QImage>
#include <QString>
#include <warn/pop>

namespace inviwo {

class InviwoApplication;
class Processor;
class ProcessorFactory;

namespace utilqt {

/**
 * Generate an image of a processor including port connections and port names
 */
IVW_QTEDITOR_API QImage generatePreview(Processor& processor);

/**
 * Generate an image of a processor including port connections and port names
 */
IVW_QTEDITOR_API QImage generatePreview(std::string_view classIdentifier,
                                        ProcessorFactory* factory);

/**
 * Generate an image of a processor
 *
 * @param classIdentifier class Identifier of the processor to generate the preview for.
 * @param opacity   if opacity is less than one, the output image will be semitransparent, range
 * [0,1]
 */
IVW_QTEDITOR_API QImage generateProcessorPreview(const QString& classIdentifier,
                                                 double opacity = 1.0);

IVW_QTEDITOR_API QImage generateProcessorPreview(Processor& processor, double opacity = 1.0);

IVW_QTEDITOR_API void saveProcessorPreviews(InviwoApplication* app, std::string& path);

}  // namespace utilqt

}  // namespace inviwo
