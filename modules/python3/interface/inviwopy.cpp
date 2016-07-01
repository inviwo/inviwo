#include <pybind11/pybind11.h>

#include <modules/python3/pybindutils.h>
#include <modules/python3/pythoninterface/pyvalueparser.h>

#include <pybind11/stl.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/templateproperty.h>
#include <inviwo/core/processors/canvasprocessor.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>

#include <modules/python3/interface/pyglmtypes.h>
#include <inviwo/core/processors/processorfactory.h>
#include <inviwo/core/metadata/processormetadata.h>

namespace py = pybind11;


template <typename T, typename P>
void pyTemplateProperty(py::class_<P> &prop) {
    using namespace inviwo;
    prop
        .def("set", [](P &p, T t) { p.set(t); })
        .def("get", [](P &p) { p.get(); });
}

template <typename T>
auto pyOrdinalProperty(py::module &m, py::class_<inviwo::Property> &parent) {
    using namespace inviwo;
    using P = OrdinalProperty<T>;
    auto classname = Defaultvalues<T>::getName() + "Property";

    py::class_<P> pyOrdinal(m, classname.c_str(), parent);
    pyOrdinal
        .def("__init__",
             [](P &instance, const std::string &identifier, const std::string &displayName,
                const T &value = Defaultvalues<T>::getVal(),
                const T &minValue = Defaultvalues<T>::getMin(),
                const T &maxValue = Defaultvalues<T>::getMax(),
                const T &increment = Defaultvalues<T>::getInc()) {
                 //(&instance) = new P(identifier, displayName, value, minValue, maxValue,
                 // increment);
                 new (&instance) P(identifier, displayName, value, minValue, maxValue, increment);
             })
        .def("getMinValue", &P::getMinValue)
        .def("getMaxValue", &P::getMaxValue)
        .def("getIncrement", &P::getIncrement)
        .def("setMinValue", &P::setMinValue)
        .def("setMaxValue", &P::setMaxValue)
        .def("setIncrement", &P::setIncrement);

    pyTemplateProperty<T, P>(pyOrdinal);

    return pyOrdinal;
}



PYBIND11_PLUGIN(inviwopy) {

    using namespace inviwo;
    py::module m("inviwopy", "Python interface for Inviwo");

    inviwo::addGLMTypes(m);

    py::class_<InviwoApplication>(m, "InviwoApplication")
        .def("getProcessorNetwork", &InviwoApplication::getProcessorNetwork,
             py::return_value_policy::reference)
        .def_property_readonly("network", &InviwoApplication::getProcessorNetwork,
                               py::return_value_policy::reference)
        //.def("getProcessorNetworkEvaluator", &InviwoApplication::getProcessorNetworkEvaluator,
        // py::return_value_policy::reference)
        .def("getBasePath", &InviwoApplication::getBasePath)
        .def("getDisplayName", &InviwoApplication::getDisplayName)
        .def("getBinaryPath", &InviwoApplication::getBinaryPath)
        .def("getPath", &InviwoApplication::getPath, py::arg("pathType"), py::arg("suffix") = "",
             py::arg("createFolder") = false)
        //.def("getModules", &InviwoApplication::getModules)
        //.def("getModuleFactoryObjects", &InviwoApplication::getModuleFactoryObjects)
        //.def("getModuleByIdentifier", &InviwoApplication::getModuleByIdentifier)
        //.def("getModuleSettings", &InviwoApplication::getModuleSettings)
        .def("waitForPool", &InviwoApplication::waitForPool)
        .def("quit", &InviwoApplication::closeInviwoApplication)
        .def("closeInviwoApplication", &InviwoApplication::closeInviwoApplication)
        .def_property_readonly("processorFactory",&InviwoApplication::getProcessorFactory, py::return_value_policy::reference)
        ;

    py::class_<ProcessorNetwork>(m, "ProcessorNetwork")
        .def_property_readonly("processors", &ProcessorNetwork::getProcessors, py::return_value_policy::reference)
        .def("getProcessorByIdentifier", &ProcessorNetwork::getProcessorByIdentifier,
             py::return_value_policy::reference)
        .def("__getattr__",
             [](ProcessorNetwork &po, std::string key) { return po.getProcessorByIdentifier(key); },
             py::return_value_policy::reference)
        .def("addProcessor", &ProcessorNetwork::addProcessor)
        //.def("removeProcessor", &ProcessorNetwork::removeProcessor)
        //    .def("removeAndDeleteProcessor", &ProcessorNetwork::removeAndDeleteProcessor)
        .def("addConnection", [&](ProcessorNetwork *on, Outport* sourcePort, Inport* destPort) {
        on->addConnection(sourcePort, destPort);
    }, py::arg("sourcePort"), py::arg("destPort"))
        .def("removeConnection", [&](ProcessorNetwork *on, Outport* sourcePort, Inport* destPort) {
        on->removeConnection(sourcePort, destPort);
    }, py::arg("sourcePort"), py::arg("destPort"))
        .def("isConnected", [&](ProcessorNetwork *on, Outport* sourcePort, Inport* destPort) {
        on->isConnected(sourcePort, destPort);
    }, py::arg("sourcePort"), py::arg("destPort"))
        .def("isPortInNetwork", &ProcessorNetwork::isPortInNetwork)
        /*  .def("addLink", &ProcessorNetwork::addLink)
          .def("removeLink", &ProcessorNetwork::removeLink)
          .def("isLinked", &ProcessorNetwork::isLinked)*/
        .def("isLinkedBidirectional", &ProcessorNetwork::isLinkedBidirectional)
        .def("getPropertiesLinkedTo", &ProcessorNetwork::getPropertiesLinkedTo)
        //.def("getLinksBetweenProcessors", &ProcessorNetwork::getLinksBetweenProcessors)
        .def("getProperty", &ProcessorNetwork::getProperty, py::return_value_policy::reference)
        .def("isPropertyInNetwork", &ProcessorNetwork::isPropertyInNetwork)
        .def("getVersion", &ProcessorNetwork::getVersion)
        .def("isEmpty", &ProcessorNetwork::isEmpty)
        .def("isInvalidating", &ProcessorNetwork::isInvalidating)
        .def("isLinking", &ProcessorNetwork::isLinking)
        .def("lock", &ProcessorNetwork::lock)
        .def("unlock", &ProcessorNetwork::unlock)
        .def("islocked", &ProcessorNetwork::islocked)
        .def("isDeserializing", &ProcessorNetwork::isDeserializing)

        .def("clear", [&](ProcessorNetwork *pn) {pn->getApplication()->getWorkspaceManager()->clear(); })
        .def("save",
             [](ProcessorNetwork *network, std::string filename) {
        network->getApplication()->getWorkspaceManager()->save(filename, [&](ExceptionContext ec) {throw; }); // is this the correct way of re throwing (we just want to pass the exception on to python)
             })
        .def("load", [](ProcessorNetwork *network, std::string filename) {
            network->getApplication()->getWorkspaceManager()->load(filename, [&](ExceptionContext ec) {throw; }); // is this the correct way of re throwing (we just want to pass the exception on to python)
        })

                 ;

    //py::class_<ProcessorFactoryObject>(m, "ProcessorFactoryObject");

    py::class_<ProcessorFactory> (m, "ProcessorFactory")
        .def("hasKey", [](ProcessorFactory *pf, std::string key) { return pf->hasKey(key); })
        .def_property_readonly("keys", [](ProcessorFactory *pf) { return pf->getKeys(); })
        .def("create", [](ProcessorFactory *pf, std::string key) { return pf->create(key); }, py::return_value_policy::reference)
        .def("create", [](ProcessorFactory *pf, std::string key,ivec2 pos) { 
        auto p = pf->create(key);
        p->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER)
            ->setPosition(pos);
        return p;
    }, py::return_value_policy::reference)
        ;



    py::class_<PropertyOwner> pyPropertyOwner(m, "PropertyOwner");
    pyPropertyOwner.def("getPath", &PropertyOwner::getPath)
        .def_property_readonly("properties", &PropertyOwner::getProperties, py::return_value_policy::reference)
        .def("__getattr__",
             [](PropertyOwner &po, std::string key) { return po.getPropertyByIdentifier(key); },
             py::return_value_policy::reference)
        .def("getPropertiesRecursive", &PropertyOwner::getPropertiesRecursive)
        .def("addProperty",
             [](PropertyOwner &po, Property *pr) { po.addProperty(pr->clone(), true); })
        .def("getPropertyByIdentifier", &PropertyOwner::getPropertyByIdentifier,
             py::return_value_policy::reference, py::arg("identifier"),
             py::arg("recursiveSearch") = false)
        .def("getPropertyByPath", &PropertyOwner::getPropertyByPath,
             py::return_value_policy::reference)
        .def("size", &PropertyOwner::size)
        //.def("setValid", &PropertyOwner::setValid)
        //.def("getInvalidationLevel", &PropertyOwner::getInvalidationLevel)
        //.def("invalidate", &PropertyOwner::invalidate)
        .def_property_readonly("processor", [](PropertyOwner &p) { return p.getProcessor(); },
             py::return_value_policy::reference)
        .def("setAllPropertiesCurrentStateAsDefault",
             &PropertyOwner::setAllPropertiesCurrentStateAsDefault)
        .def("resetAllPoperties", &PropertyOwner::resetAllPoperties);

    py::class_<Port> pyPort(m, "Port");
    pyPort.def_property_readonly("identifier", &Port::getIdentifier);
    pyPort.def_property_readonly("processor", &Port::getProcessor, py::return_value_policy::reference);
    pyPort.def_property_readonly("classIdentifier", &Port::getClassIdentifier);
    pyPort.def_property_readonly("contentInfo", &Port::getContentInfo);
    pyPort.def("isConnected", &Port::isConnected);
    pyPort.def("isReady", &Port::isReady);


    py::class_<Inport> pyInport(m, "Inport" , pyPort);
    pyInport.def_property("optional", &Inport::isOptional, &Inport::setOptional);
    pyInport.def("canConnectTo", &Inport::canConnectTo);
    pyInport.def("connectTo", &Inport::connectTo);
    pyInport.def("disconnectFrom", &Inport::disconnectFrom);
    pyInport.def("isConnectedTo", &Inport::isConnectedTo);
    pyInport.def("getConnectedOutport", &Inport::getConnectedOutport, py::return_value_policy::reference);
    pyInport.def("getConnectedOutports", &Inport::getConnectedOutports, py::return_value_policy::reference);
    pyInport.def("getMaxNumberOfConnections", &Inport::getMaxNumberOfConnections);
    pyInport.def("getNumberOfConnections", &Inport::getNumberOfConnections);
    pyInport.def("getChangedOutports", &Inport::getChangedOutports);

    py::class_<Outport> pyOutport(m, "Outport" , pyPort);
    pyOutport.def("isConnectedTo", &Outport::isConnectedTo);
    pyOutport.def("getConnectedInports", &Outport::getConnectedInports);


    py::class_<Processor> pyProcessor(m, "Processor", pyPropertyOwner);
    pyProcessor.def("getClassIdentifier", &Processor::getClassIdentifier)
        .def("getDisplayName", &Processor::getDisplayName)
        .def("getCategory", &Processor::getCategory)
        .def("getCodeState", &Processor::getCodeState)
        .def("getTags", &Processor::getTags)
        .def("setIdentifier", &Processor::setIdentifier)
        .def("getIdentifier", &Processor::getIdentifier)
        .def("hasProcessorWidget", &Processor::hasProcessorWidget)
        .def("getNetwork", &Processor::getNetwork, py::return_value_policy::reference)
        .def("hasProcessorWidget", &Processor::hasProcessorWidget, py::return_value_policy::reference)
        .def_property_readonly("inports", &Processor::getInports, py::return_value_policy::reference)
        .def_property_readonly("outports", &Processor::getOutports, py::return_value_policy::reference)
        .def("getPort", &Processor::getPort, py::return_value_policy::reference)
        .def("getInport", &Processor::getInport, py::return_value_policy::reference)
        .def("getOutport", &Processor::getOutport, py::return_value_policy::reference)
        .def_property(
            "location",
            [](Processor *p) {
                return p->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER)
                    ->getPosition();
            },
            [](Processor *p, ivec2 pos) {
                p->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER)
                    ->setPosition(pos);
            }

            )

        ;

    py::class_<CanvasProcessor>(m, "CanvasProcessor", pyProcessor)
        .def("setCanvasSize", &CanvasProcessor::setCanvasSize)
        .def("getCanvasSize", &CanvasProcessor::getCanvasSize)
        .def("getUseCustomDimensions", &CanvasProcessor::getUseCustomDimensions)
        .def("getCustomDimensions", &CanvasProcessor::getCustomDimensions)
        //.def("saveImageLayer", &CanvasProcessor::saveImageLayer)
        //.def("saveImageLayer", &CanvasProcessor::saveImageLayer)
        //.def("getVisibleLayer", &CanvasProcessor::getVisibleLayer)
        //.def("getImage", &CanvasProcessor::getImage)
        .def("isReady", &CanvasProcessor::isReady)
        .def("isFullScreen", &CanvasProcessor::isFullScreen)
        .def("setFullScreen", &CanvasProcessor::setFullScreen)
        .def("snapshot", [](CanvasProcessor *canvas, std::string filepath) {
            auto ext = filesystem::getFileExtension(filepath);

            auto writer = canvas->getNetwork()
                              ->getApplication()
                              ->getDataWriterFactory()
                              ->getWriterForTypeAndExtension<Layer>(ext);
            if (!writer) {
                std::stringstream ss;
                ss << "No write for extension " << ext;
                throw std::exception(ss.str().c_str());
            }

            auto layer = canvas->getVisibleLayer();
            writer->writeData(layer, filepath);
        });

    py::class_<Property> pyproperty(m, "Property", pyPropertyOwner);
    pyproperty.def("setIdentifier", &Property::setIdentifier)
        .def("getIdentifier", &Property::getIdentifier)
        .def("getPath", &Property::getPath)
        .def("setDisplayName", &Property::setDisplayName)
        .def("getDisplayName", &Property::getDisplayName)
        .def("getClassIdentifierForWidget", &Property::getClassIdentifierForWidget)
        .def("setSemantics", &Property::setSemantics)
        .def("getSemantics", &Property::getSemantics)
        .def("setReadOnly", &Property::setReadOnly)
        .def("getReadOnly", &Property::getReadOnly)
        .def("getInvalidationLevel", &Property::getInvalidationLevel)
        .def("updateWidgets", &Property::updateWidgets)
        .def("setCurrentStateAsDefault", &Property::setCurrentStateAsDefault)
        .def("resetToDefaultState", &Property::resetToDefaultState);

    pyOrdinalProperty<float>(m, pyproperty);
    pyOrdinalProperty<int>(m, pyproperty);
    pyOrdinalProperty<size_t>(m, pyproperty);
    pyOrdinalProperty<glm::i64>(m, pyproperty);
    pyOrdinalProperty<double>(m, pyproperty);
    pyOrdinalProperty<vec2>(m, pyproperty);
    pyOrdinalProperty<vec3>(m, pyproperty);
    pyOrdinalProperty<vec4>(m, pyproperty);
    pyOrdinalProperty<dvec2>(m, pyproperty);
    pyOrdinalProperty<dvec3>(m, pyproperty);
    pyOrdinalProperty<dvec4>(m, pyproperty);
    pyOrdinalProperty<ivec2>(m, pyproperty);
    pyOrdinalProperty<ivec3>(m, pyproperty);
    pyOrdinalProperty<ivec4>(m, pyproperty);
    pyOrdinalProperty<size2_t>(m, pyproperty);
    pyOrdinalProperty<size3_t>(m, pyproperty);
    pyOrdinalProperty<size4_t>(m, pyproperty);
    pyOrdinalProperty<mat2>(m, pyproperty);
    pyOrdinalProperty<mat3>(m, pyproperty);
    pyOrdinalProperty<mat4>(m, pyproperty);
    pyOrdinalProperty<dmat2>(m, pyproperty);
    pyOrdinalProperty<dmat3>(m, pyproperty);
    pyOrdinalProperty<dmat4>(m, pyproperty);

    py::class_<ButtonProperty>(m, "ButtonProperty", pyproperty)
        .def("pressButton", &ButtonProperty::pressButton);

    py::class_<CameraProperty>(m, "CameraProperty", pyproperty)
        .def("getLookFrom", &CameraProperty::getLookFrom)
        .def("setLookFrom", &CameraProperty::setLookFrom)
        .def("getLookTo", &CameraProperty::getLookTo)
        .def("setLookTo", &CameraProperty::setLookTo)
        .def("getLookUp", &CameraProperty::getLookUp)
        .def("setLookUp", &CameraProperty::setLookUp)
        .def("getLookRight", &CameraProperty::getLookRight)
        .def("setAspectRatio", &CameraProperty::setAspectRatio)
        .def("getAspectRatio", &CameraProperty::getAspectRatio)
        .def("setLook", &CameraProperty::setLook)
        .def("getNearPlaneDist", &CameraProperty::getNearPlaneDist)
        .def("getFarPlaneDist", &CameraProperty::getFarPlaneDist)
        .def("getLookFromMinValue", &CameraProperty::getLookFromMinValue)
        .def("getLookFromMaxValue", &CameraProperty::getLookFromMaxValue)
        .def("getLookToMinValue", &CameraProperty::getLookToMinValue)
        .def("getLookToMaxValue", &CameraProperty::getLookToMaxValue)
        .def("getWorldPosFromNormalizedDeviceCoords",
             &CameraProperty::getWorldPosFromNormalizedDeviceCoords)
        .def("getClipPosFromNormalizedDeviceCoords",
             &CameraProperty::getClipPosFromNormalizedDeviceCoords)
        .def("getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth",
             &CameraProperty::getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth)
        .def("viewMatrix", &CameraProperty::viewMatrix)
        .def("projectionMatrix", &CameraProperty::projectionMatrix)
        .def("inverseViewMatrix", &CameraProperty::inverseViewMatrix)
        .def("inverseProjectionMatrix", &CameraProperty::inverseProjectionMatrix)
        .def("adjustCameraToData", &CameraProperty::adjustCameraToData)
        .def("resetAdjustCameraToData", &CameraProperty::resetAdjustCameraToData);

    py::class_<TransferFunctionProperty>(m, "TransferFunctionProperty", pyproperty)
        .def("getMask", &TransferFunctionProperty::getMask)
        .def("setMask", &TransferFunctionProperty::setMask)
        .def("getZoomH", &TransferFunctionProperty::getZoomH)
        .def("setZoomH", &TransferFunctionProperty::setZoomH)
        .def("getZoomV", &TransferFunctionProperty::getZoomV)
        .def("setZoomV", &TransferFunctionProperty::setZoomV)
        .def("save",
             [](TransferFunctionProperty *tf, std::string filename) {
                 Serializer serializer(filename);
                 tf->serialize(serializer);
                 serializer.writeFile();
             })
        .def("load",
             [](TransferFunctionProperty *tf, std::string filename) {
                 auto app = pyutil::getApplication();

                 Deserializer deserializer(filename);
                 tf->deserialize(deserializer);
             })
        .def("clear", [](TransferFunctionProperty &tp) { tp.get().clearPoints(); })
        .def("addPoint", [](TransferFunctionProperty &tp, vec2 pos, vec3 color) {
            tp.get().addPoint(pos, vec4(color, pos.y));
        });
    m.attr("app") = py::cast(pyutil::getApplication() , py::return_value_policy::reference);

    py::enum_<inviwo::PathType>(m, "PathType")
        .value("Data", PathType::Data)
        .value("Volumes", PathType::Volumes)
        .value("Workspaces", PathType::Workspaces)
        .value("Scripts", PathType::Scripts)
        .value("PortInspectors", PathType::PortInspectors)
        .value("Images", PathType::Images)
        .value("Databases", PathType::Databases)
        .value("Resources", PathType::Resources)
        .value("TransferFunctions", PathType::TransferFunctions)
        .value("Settings", PathType::Settings)
        .value("Help", PathType::Help)
        .value("Tests", PathType::Tests);

    return m.ptr();
}
