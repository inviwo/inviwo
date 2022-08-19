/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2021 Inviwo Foundation
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

#include <modules/basegl/processors/entryexitpointsprocessor.h>
#include <inviwo/core/io/serialization/versionconverter.h>
#include <inviwo/core/algorithm/boundingbox.h>
#include <modules/opengl/image/imagegl.h>

namespace inviwo {

const ProcessorInfo EntryExitPoints::processorInfo_{
    "org.inviwo.EntryExitPoints",  // Class identifier
    "Entry Exit Points",           // Display name
    "Mesh Rendering",              // Category
    CodeState::Stable,             // Code state
    Tags::GL,                      // Tags
    R"(Computes the entry and exit points of a triangle mesh from the camera position
    in texture space. The output color will be zero if no intersection is found,
    otherwise.)"_unindentHelp};

const ProcessorInfo EntryExitPoints::getProcessorInfo() const { return processorInfo_; }

EntryExitPoints::EntryExitPoints()
    : Processor()
    , inport_("geometry", "The input mesh used for determining entry and exit points"_help)
    , entryPort_("entry", "The first hit point in texture coordinates [0,1]"_help,
                 DataVec4UInt16::get())
    , exitPort_("exit", "The last hit point in texture coordinates [0,1]"_help,
                DataVec4UInt16::get())
    , camera_("camera", "Camera", util::boundingBox(inport_))
    , capNearClipping_(
          "capNearClipping", "Cap near plane clipping",
          "Insert a plane at the near plane clip point to avoid generating entry points "
          "from the inside of the geometry"_help,
          true)
    , trackball_(&camera_) {

    addPort(inport_);
    addPort(entryPort_, "ImagePortGroup1");
    addPort(exitPort_, "ImagePortGroup1");
    addProperties(capNearClipping_, camera_, trackball_);

    onReloadCallback_ =
        entryExitHelper_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

EntryExitPoints::~EntryExitPoints() = default;

void EntryExitPoints::process() {
    auto entry = entryPort_.getEditableData();

    if (inport_.getData()->hasBuffer(BufferType::NormalAttrib) &&
        entry->getNumberOfColorLayers() != 2) {

        entry = std::make_shared<Image>(entry->getDimensions(), DataVec4UInt16::get());
        // Add a layer for the normals
        entry->addColorLayer(
            std::make_shared<Layer>(entry->getDimensions(), DataVec4Int8::get(), LayerType::Color));
        entryPort_.setData(entry);
        entryImg_ = entry->getEditableRepresentation<ImageGL>();
    } else if (!inport_.getData()->hasBuffer(BufferType::NormalAttrib) &&
               entry->getNumberOfColorLayers() != 1) {

        entry = std::make_shared<Image>(entry->getDimensions(), DataVec4UInt16::get());
        entryPort_.setData(entry);
        entryImg_ = entry->getEditableRepresentation<ImageGL>();
    } else if (!entryImg_ || !entryImg_->isValid()) {
        entryImg_ = entry->getEditableRepresentation<ImageGL>();
    }
    if (!exitImg_ || !exitImg_->isValid()) {
        exitImg_ = exitPort_.getEditableData()->getEditableRepresentation<ImageGL>();
    }

    const bool addNormals = inport_.getData()->hasBuffer(BufferType::NormalAttrib);
    entryExitHelper_(*entryImg_, *exitImg_, camera_.get(), *inport_.getData(),
                     capNearClipping_ ? algorithm::CapNearClip::Yes : algorithm::CapNearClip::No,
                     addNormals ? algorithm::IncludeNormals::Yes : algorithm::IncludeNormals::No);
}

void EntryExitPoints::deserialize(Deserializer& d) {
    util::renamePort(d, {{&entryPort_, "entry-points"}, {&exitPort_, "exit-points"}});
    Processor::deserialize(d);
}

}  // namespace inviwo
