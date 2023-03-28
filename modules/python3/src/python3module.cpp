/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2023 Inviwo Foundation
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

#include <modules/python3/python3module.h>  // for Python3Module

#include <pybind11/pybind11.h>  // for module

#include <inviwo/core/common/inviwoapplication.h>                    // for InviwoApplication
#include <inviwo/core/common/inviwomodule.h>                         // for InviwoModule
#include <inviwo/core/datastructures/datarepresentation.h>           // for DataRepresentation<>...
#include <inviwo/core/datastructures/representationconverter.h>      // for RepresentationConver...
#include <inviwo/core/datastructures/representationfactoryobject.h>  // for RepresentationFactor...
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumerepresentation.h>  // for VolumeRepresentation
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerrepresentation.h>
#include <inviwo/core/util/commandlineparser.h>  // for CommandLineParser
#include <inviwo/core/util/exception.h>          // for ModuleInitException
#include <inviwo/core/util/filesystem.h>         // for fileExists
#include <inviwo/core/util/logcentral.h>         // for LogCentral, LogWarn
#include <inviwo/core/util/pathtype.h>           // for PathType, PathType::...
#include <inviwo/core/util/sourcecontext.h>      // for IVW_CONTEXT
#include <modules/python3/pythonscript.h>
#include <modules/python3/pythoninterpreter.h>  // for PythonInterpreter
#include <modules/python3/pythonlogger.h>       // for PythonLogger
#include <modules/python3/volumepy.h>           // for VolumePy, VolumePy2R...
#include <modules/python3/layerpy.h>

#include <tclap/ArgException.h>  // for ArgParseException
#include <tclap/ValueArg.h>      // for ValueArg

#include <exception>    // for exception
#include <functional>   // for __base, function
#include <memory>       // for make_unique, unique_ptr
#include <ostream>      // for operator<<
#include <string>       // for string, operator+
#include <string_view>  // for string_view
#include <vector>       // for vector

namespace inviwo {

namespace {
class VolumePyFactoryObject
    : public RepresentationFactoryObjectTemplate<VolumeRepresentation, VolumePy> {
public:
    virtual std::unique_ptr<VolumeRepresentation> create(
        const typename VolumeRepresentation::ReprOwner* volume) {
        auto volumePy = std::make_unique<VolumePy>(
            volume->getDimensions(), volume->getDataFormat(), volume->getSwizzleMask(),
            volume->getInterpolation(), volume->getWrapping());

        return volumePy;
    }
};

class LayerPyFactoryObject
    : public RepresentationFactoryObjectTemplate<LayerRepresentation, LayerPy> {
public:
    virtual std::unique_ptr<LayerRepresentation> create(
        const typename LayerRepresentation::ReprOwner* layer) {
        auto layerPy = std::make_unique<LayerPy>(layer->getDimensions(), layer->getLayerType(),
                                                 layer->getDataFormat(), layer->getSwizzleMask(),
                                                 layer->getInterpolation(), layer->getWrapping());

        return layerPy;
    }
};
}  // namespace

Python3Module::Python3Module(InviwoApplication* app)
    : InviwoModule(app, "Python3")
    , pythonInterpreter_(std::make_unique<PythonInterpreter>())
    , pythonScriptArg_("p", "pythonScript", "Specify a python script to run at startup", false, "",
                       "python script")
    , argHolder_{app, pythonScriptArg_,
                 [this]() {
                     auto filename = pythonScriptArg_.getValue();
                     if (!filesystem::fileExists(filename)) {
                         LogWarn("Could not run script, file does not exist: " << filename);
                         return;
                     }
                     PythonScriptDisk s(filename);
                     s.run();
                 },
                 100}
    , pythonLogger_{}
    , scripts_{getPath() + "/scripts"}
    , pythonFolderObserver_{app, getPath() + "/processors", *this}
    , settingsFolderObserver_{app, app->getPath(PathType::Settings, "/python_processors", true),
                              *this} {

    pythonInterpreter_->addObserver(&pythonLogger_);

    registerRepresentationFactoryObject<VolumeRepresentation>(
        std::make_unique<VolumePyFactoryObject>());
    registerRepresentationConverter<VolumeRepresentation>(
        std::make_unique<VolumeRAM2PyConverter>());
    registerRepresentationConverter<VolumeRepresentation>(
        std::make_unique<VolumePy2RAMConverter>());

    registerRepresentationFactoryObject<LayerRepresentation>(
        std::make_unique<LayerPyFactoryObject>());
    registerRepresentationConverter<LayerRepresentation>(std::make_unique<LayerRAM2PyConverter>());
    registerRepresentationConverter<LayerRepresentation>(std::make_unique<LayerPy2RAMConverter>());

    // We need to import inviwopy to trigger the initialization code in inviwopy.cpp, this is needed
    // to be able to cast cpp/inviwo objects to python objects.
    try {
        pybind11::module::import("inviwopy");
    } catch (const std::exception& e) {
        throw ModuleInitException(e.what(), IVW_CONTEXT);
    }
}

Python3Module::~Python3Module() {
    pythonInterpreter_->removeObserver(&pythonLogger_);
    app_->getCommandLineParser().remove(&pythonScriptArg_);
}

PythonInterpreter* Python3Module::getPythonInterpreter() { return pythonInterpreter_.get(); }

}  // namespace inviwo
