#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/pytypes.h>

#include <modules/python3/python3module.h>
#include <modules/python3/pybindutils.h>


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
#include <inviwo/core/processors/processorwidget.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/util/settings/settings.h>
#include <inviwo/core/util/exception.h>
#include <modules/base/processors/volumesource.h>
#include <inviwo/core/util/commandlineparser.h>



//
//template<typename T>
//class ProcessorHolder {
//
//public:
//    ProcessorHolder() : processor_(nullptr) {}
//    ProcessorHolder(T *t) : processor_(t) {}
//    ProcessorHolder(const ProcessorHolder &ph) : processor_(ph.processor_) {}
//
//    ProcessorHolder& operator=(ProcessorHolder &&r) {
//        if (*this != r) {
//            processor_ = r.processor_;
//            r.processor_ = nullptr;
//        }
//        return *this;
//    }
//
//
//
//    ProcessorHolder& operator=(T *r) {
//        if (processor_ != r) {
//            if (processor_ && processor_->getNetwork() == nullptr) {
//                delete processor_;
//            }
//            processor_ = r;
//        }
//        return *this;
//    }
//
//    ProcessorHolder(ProcessorHolder &&ph) : processor_(ph.processor_) {
//        ph.processor_ = nullptr;
//    }
//
//    virtual ~ProcessorHolder() {
//
//        if (processor_ && processor_->getNetwork() == nullptr) {
//            delete processor_;
//        }
//    }
//
//
//
//private:
//
//    T* processor_;
//
//};
//
//PYBIND11_DECLARE_HOLDER_TYPE(inviwo::Processor*, ProcessorHolder<inviwo::Processor*>);


template<typename T>
using ListCasterBase = pybind11::detail::list_caster<std::vector<T *>, T *>;

namespace pybind11 {
    namespace detail {
        using namespace inviwo;
        template<> struct type_caster<std::vector<Processor *>> : ListCasterBase<Processor> {
            static handle cast(const std::vector<Processor *> &src, return_value_policy, handle parent) {
                return ListCasterBase<Processor>::cast(src, return_value_policy::reference, parent);
            }
            static handle cast(const std::vector<Processor *> *src, return_value_policy pol, handle parent) {
                return cast(*src, pol, parent);
            }
        };



        template<> struct type_caster<std::vector<CanvasProcessor *>> : ListCasterBase<CanvasProcessor> {
            static handle cast(const std::vector<CanvasProcessor *> &src, return_value_policy, handle parent) {
                return ListCasterBase<CanvasProcessor>::cast(src, return_value_policy::reference, parent);
            }
            static handle cast(const std::vector<CanvasProcessor *> *src, return_value_policy pol, handle parent) {
                return cast(*src, pol, parent);
            }
        };


    }
}




namespace py = pybind11;


template<typename T> void addProcessorDefs(T class_) {
    class_.def(py::init<>());
}


//PYBIND11_MAKE_OPAQUE(std::vector<inviwo::Processor*>);

template<typename T>
struct HasOwnerDeleter { void operator()(T* p) { if (p && p->getOwner() == nullptr) delete p; } };

template<typename T>
using NoDelete = std::unique_ptr<T,HasOwnerDeleter<T>>;


template <typename T, typename P , typename C>
void pyTemplateProperty(C &prop) {
    using namespace inviwo;
    prop.def_property("value", [](P &p) { p.get(); }, [](P &p, T t) { p.set(t); });
}

template <typename T>
auto pyOrdinalProperty(py::module &m) {
    using namespace inviwo;
    using P = OrdinalProperty<T>;
    auto classname = Defaultvalues<T>::getName() + "Property";

    py::class_<P, Property, std::unique_ptr<P, HasOwnerDeleter<P>>> pyOrdinal(m, classname.c_str());
    pyOrdinal
        .def("__init__",
             [](P &instance, const std::string &identifier, const std::string &displayName,
                const T &value = Defaultvalues<T>::getVal(),
                const T &minValue = Defaultvalues<T>::getMin(),
                const T &maxValue = Defaultvalues<T>::getMax(),
                const T &increment = Defaultvalues<T>::getInc()) {
        new (&instance) P(identifier, displayName, value, minValue, maxValue, increment);
    })
        .def_property("minValue", &P::getMinValue, &P::setMinValue)
        .def_property("maxValue", &P::getMaxValue, &P::setMaxValue)
        .def_property("increment", &P::getIncrement, &P::setIncrement)
        ;
    pyTemplateProperty<T, P>(pyOrdinal);

    return pyOrdinal;
}

PYBIND11_PLUGIN(inviwopy) {


#ifdef IVW_ENABLE_MSVC_MEM_LEAK_TEST
    VLDDisable();
#endif

    using namespace inviwo;
    //pybind11::module m("inviwopy", "Python interface for Inviwo");
    PyBindModule m("inviwopy", "Python interface for Inviwo");

    addGLMTypes(m.mainModule_);

    m.addClass<InviwoApplication>("InviwoApplication")
        .def_property_readonly("network", &InviwoApplication::getProcessorNetwork,
                               py::return_value_policy::reference)
        //.def("getProcessorNetworkEvaluator", &InviwoApplication::getProcessorNetworkEvaluator,
        // py::return_value_policy::reference)
        .def_property_readonly("basePath", &InviwoApplication::getBasePath)
        .def_property_readonly("displayName", &InviwoApplication::getDisplayName)
        .def_property_readonly("binaryPath", &InviwoApplication::getBinaryPath)
        .def("getPath", &InviwoApplication::getPath, py::arg("pathType"), py::arg("suffix") = "",
             py::arg("createFolder") = false)
        .def_property_readonly("modules",
                               [](InviwoApplication *app) {
                                   std::vector<InviwoModule *> modules;
                                   for (auto &m : app->getModules()) {
                                       modules.push_back(m.get());
                                   }
                                   return modules;
                               },
                               py::return_value_policy::reference)
        .def("getModuleByIdentifier", &InviwoApplication::getModuleByIdentifier,
             py::return_value_policy::reference)
        .def("getModuleSettings", &InviwoApplication::getModuleSettings,
             py::return_value_policy::reference)
        .def("waitForPool", &InviwoApplication::waitForPool)
        .def("closeInviwoApplication", &InviwoApplication::closeInviwoApplication)
        .def_property_readonly("processorFactory", &InviwoApplication::getProcessorFactory,
            py::return_value_policy::reference)
        .def_property_readonly("propertyFactory", &InviwoApplication::getPropertyFactory,
                                       py::return_value_policy::reference)
                                   

        .def("getOutputPath",
             [](InviwoApplication *app) { return app->getCommandLineParser().getOutputPath(); })

        ;

    m.addClass<InviwoModule>("InviwoModule")
        .def_property_readonly("identifier", &InviwoModule::getIdentifier)
        .def_property_readonly("description", &InviwoModule::getDescription)
        .def_property_readonly("path", [](InviwoModule *m) { return m->getPath(); })
        //     .def("getPath", [](InviwoModule *m , ModulePath type) { return m->getPath(type); }) //TODO expost modulePath
        .def_property_readonly("version", &InviwoModule::getVersion)
        ;


    py::class_<PortConnection>(m.mainModule_, "PortConnection")
        .def(py::init<Outport *, Inport *>())
        .def_property_readonly("inport", &PortConnection::getInport,
            py::return_value_policy::reference)
        .def_property_readonly("outport", &PortConnection::getOutport,
            py::return_value_policy::reference);

    py::class_<PropertyLink>(m.mainModule_, "PropertyLink")
        .def(py::init<Property *, Property *>(), py::arg("src"), py::arg("dst"))
        .def_property_readonly("source", &PropertyLink::getSource, py::return_value_policy::reference)
        .def_property_readonly("destination", &PropertyLink::getDestination, py::return_value_policy::reference)
        ;

    py::class_<ProcessorNetwork>(m.mainModule_, "ProcessorNetwork")
        .def_property_readonly("processors", &ProcessorNetwork::getProcessors,
                               py::return_value_policy::reference)
        .def("getProcessorByIdentifier", &ProcessorNetwork::getProcessorByIdentifier,
             py::return_value_policy::reference)
        .def("__getattr__",
             [](ProcessorNetwork &po, std::string key) {
                 auto p = po.getProcessorByIdentifier(key);
                 if (auto cp = dynamic_cast<CanvasProcessor *>(p)) {
                     return py::cast(cp);
                 }
                 return py::cast(p);

             },
             py::return_value_policy::reference)
        .def("addProcessor",
             [](ProcessorNetwork *pn, Processor *processor) { pn->addProcessor(processor); })
        .def("removeProcessor",
             [](ProcessorNetwork *pn, Processor *processor) { pn->removeProcessor(processor); })

        .def_property_readonly("connections", &ProcessorNetwork::getConnections,
                               py::return_value_policy::reference)
        .def("addConnection", [&](ProcessorNetwork *on, Outport *sourcePort,
                                  Inport *destPort) { on->addConnection(sourcePort, destPort); },
             py::arg("sourcePort"), py::arg("destPort"))
        .def("addConnection", [&](ProcessorNetwork *on,
                                  PortConnection &connection) { on->addConnection(connection); })
        .def("removeConnection",
             [&](ProcessorNetwork *on, Outport *sourcePort, Inport *destPort) {
                 on->removeConnection(sourcePort, destPort);
             },
             py::arg("sourcePort"), py::arg("destPort"))
        .def("removeConnection",
             [&](ProcessorNetwork *on, PortConnection &connection) {
                 on->removeConnection(connection);
             })

        .def("isConnected", [&](ProcessorNetwork *on, Outport *sourcePort,
                                Inport *destPort) { on->isConnected(sourcePort, destPort); },
             py::arg("sourcePort"), py::arg("destPort"))
        .def("isPortInNetwork", &ProcessorNetwork::isPortInNetwork)
        .def_property_readonly("links", &ProcessorNetwork::getLinks,
                               py::return_value_policy::reference)
        .def("addLink",
             [](ProcessorNetwork *pn, Property *src, Property *dst) { pn->addLink(src, dst); })
        .def("addLink", [](ProcessorNetwork *pn, PropertyLink &link) { pn->addLink(link); })
        .def("removeLink",
             [](ProcessorNetwork *pn, Property *src, Property *dst) { pn->removeLink(src, dst); })

        .def("removeLink",
             [](ProcessorNetwork *pn, Property *src, Property *dst) { pn->removeLink(src, dst); })
        .def("isLinked", [](ProcessorNetwork *pn, PropertyLink &link) { pn->isLinked(link); })
        .def("isLinkedBidirectional", &ProcessorNetwork::isLinkedBidirectional)
        .def("getLinksBetweenProcessors", &ProcessorNetwork::getLinksBetweenProcessors,
             py::return_value_policy::reference)
        .def_property_readonly("canvases", &ProcessorNetwork::getProcessorsByType<CanvasProcessor>,
                               py::return_value_policy::reference)
        .def("getProperty", &ProcessorNetwork::getProperty, py::return_value_policy::reference)
        .def("getPropertiesLinkedTo", &ProcessorNetwork::getPropertiesLinkedTo,
             py::return_value_policy::reference)
        .def("isPropertyInNetwork", &ProcessorNetwork::isPropertyInNetwork)
        .def_property_readonly("version", &ProcessorNetwork::getVersion)
        .def_property_readonly("empty", &ProcessorNetwork::isEmpty)
        .def_property_readonly("invalidating", &ProcessorNetwork::isInvalidating)
        .def_property_readonly("linking", &ProcessorNetwork::isLinking)
        .def("lock", &ProcessorNetwork::lock)
        .def("unlock", &ProcessorNetwork::unlock)
        .def_property_readonly("locked", &ProcessorNetwork::islocked)
        .def_property_readonly("deserializing", &ProcessorNetwork::isDeserializing)

        .def("clear",
             [&](ProcessorNetwork *pn) { pn->getApplication()->getWorkspaceManager()->clear(); })
        .def("save",
             [](ProcessorNetwork *network, std::string filename) {
                 network->getApplication()->getWorkspaceManager()->save(
                     filename, [&](ExceptionContext ec) { throw; });  // is this the correct way of
                                                                      // re throwing (we just want
                                                                      // to pass the exception on to
                                                                      // python)
             })
        .def("load", [](ProcessorNetwork *network, std::string filename) {
            network->getApplication()->getWorkspaceManager()->load(
                filename, [&](ExceptionContext ec) { throw; });  // is this the correct way of re
                                                                 // throwing (we just want to pass
                                                                 // the exception on to python)
        });

    py::class_<ProcessorFactory>(m.mainModule_, "ProcessorFactory")
        .def("hasKey", [](ProcessorFactory *pf, std::string key) { return pf->hasKey(key); })
        .def_property_readonly("keys", [](ProcessorFactory *pf) { return pf->getKeys(); })
        .def("create",
            [](ProcessorFactory *pf, std::string key) { return pf->create(key).release(); })
        .def("create",
            [](ProcessorFactory *pf, std::string key, ivec2 pos) {
        auto p = pf->create(key);
        p->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER)
            ->setPosition(pos);
        return p.release();
    }  );
    



    py::class_<PropertyFactory>(m.mainModule_, "PropertyFactory")
        .def("hasKey", [](PropertyFactory *pf, std::string key) { return pf->hasKey(key); })
        .def_property_readonly("keys", [](PropertyFactory *pf) { return pf->getKeys(); })
        .def("create",
            [](PropertyFactory *pf, std::string key) { return pf->create(key).release(); })
    ;




    //struct propertyOwnerDelete {
    //    void operator()(PropertyOwner* p) {
    //        if (auto pr = dynamic_cast<Processor*>(p)) {
    //            if (pr->getNetwork() == nullptr) {
    //                delete pr;
    //            }
    //        }
    //    }
    //};
    py::class_<PropertyOwner , std::unique_ptr<PropertyOwner, py::nodelete>>(m.mainModule_, "PropertyOwner")
        .def("getPath", &PropertyOwner::getPath)
        .def_property_readonly("properties", &PropertyOwner::getProperties,py::return_value_policy::reference)
        .def("__getattr__",
            [](PropertyOwner &po, std::string key) { 
                auto prop = po.getPropertyByIdentifier(key);
                if(auto cp = dynamic_cast<CompositeProperty*>(prop)){
                    return py::cast(cp);
                }else{
                    return py::cast(prop);
                }
    },
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

    py::class_<Port>(m.mainModule_, "Port")
    .def_property_readonly("identifier", &Port::getIdentifier)
    .def_property_readonly("processor", &Port::getProcessor,
        py::return_value_policy::reference)
    .def_property_readonly("classIdentifier", &Port::getClassIdentifier)
    .def_property_readonly("contentInfo", &Port::getContentInfo)
    .def("isConnected", &Port::isConnected)
    .def("isReady", &Port::isReady);

    py::class_<Inport,Port>(m.mainModule_, "Inport")
        .def_property("optional", &Inport::isOptional, &Inport::setOptional)
        .def("canConnectTo", &Inport::canConnectTo)
        .def("connectTo", &Inport::connectTo)
        .def("disconnectFrom", &Inport::disconnectFrom)
        .def("isConnectedTo", &Inport::isConnectedTo)
        .def("getConnectedOutport", &Inport::getConnectedOutport,
            py::return_value_policy::reference)
        .def("getConnectedOutports", &Inport::getConnectedOutports,
            py::return_value_policy::reference)
        .def("getMaxNumberOfConnections", &Inport::getMaxNumberOfConnections)
        .def("getNumberOfConnections", &Inport::getNumberOfConnections)
        .def("getChangedOutports", &Inport::getChangedOutports);

    py::class_<Outport,Port> pyOutport(m.mainModule_, "Outport");
    pyOutport.def("isConnectedTo", &Outport::isConnectedTo);
    pyOutport.def("getConnectedInports", &Outport::getConnectedInports);

    py::class_<ProcessorWidget>(m.mainModule_, "ProcessorWidget")
        .def_property("visibility", &ProcessorWidget::isVisible, &ProcessorWidget::setVisible)
        .def_property("dimensions", &ProcessorWidget::getDimensions, &ProcessorWidget::setDimensions)
        .def_property("position", &ProcessorWidget::getPosition, &ProcessorWidget::setPosition)
        .def("show", &ProcessorWidget::show)
        .def("hide", &ProcessorWidget::hide)
        ;


    py::class_<Settings, PropertyOwner , std::unique_ptr<Settings , py::nodelete>>(m.mainModule_, "Settings");

    struct processorDelete { void operator()(Processor* p) { if (p && p->getNetwork() == nullptr) delete p; } };
    //py::class_<Processor, PropertyOwner >(m.mainModule_, "Processor")
    py::class_<Processor, PropertyOwner, std::unique_ptr<Processor, processorDelete > >(m.mainModule_, "Processor")
        .def_property_readonly("classIdentifier", &Processor::getClassIdentifier)
        .def_property_readonly("displayName", &Processor::getDisplayName)
        .def_property_readonly("category", &Processor::getCategory)
        .def_property_readonly("codeState", &Processor::getCodeState)  // TODO expose states
        .def_property_readonly("tags", &Processor::getTags)            // TODO expose tags
        .def_property("identifier", &Processor::getIdentifier, &Processor::setIdentifier)
        .def("hasProcessorWidget", &Processor::hasProcessorWidget)
        .def_property_readonly("widget", &Processor::getProcessorWidget)
        .def_property_readonly("network", &Processor::getNetwork,
                               py::return_value_policy::reference)
        .def_property_readonly("inports", &Processor::getInports,
                               py::return_value_policy::reference)
        .def_property_readonly("outports", &Processor::getOutports,
                               py::return_value_policy::reference)
        .def("getPort", &Processor::getPort, py::return_value_policy::reference)
        .def("getInport", &Processor::getInport, py::return_value_policy::reference)
        .def("getOutport", &Processor::getOutport, py::return_value_policy::reference)
        .def_property("position",
                      [](Processor *p) {
                          return p
                              ->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER)
                              ->getPosition();
                      },
                      [](Processor *p, ivec2 pos) {
                          p->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER)
                              ->setPosition(pos);
                      })
        .def_property("selected",
                      [](Processor *p) {
                          return p
                              ->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER)
                              ->isSelected();
                      },
                      [](Processor *p, bool selected) {
                          p->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER)
                              ->setSelected(selected);
                      })
        .def_property("visible",
                      [](Processor *p) {
                          return p
                              ->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER)
                              ->isVisible();
                      },
                      [](Processor *p, bool selected) {
                          p->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER)
                              ->setVisible(selected);
                      });

    py::class_<CanvasProcessor, Processor> canvasPorcessor(m.mainModule_, "CanvasProcessor");
    canvasPorcessor
        .def_property("size", &CanvasProcessor::getCanvasSize, &CanvasProcessor::setCanvasSize)
        .def("getUseCustomDimensions", &CanvasProcessor::getUseCustomDimensions)
        .def_property_readonly("customDimensions", &CanvasProcessor::getCustomDimensions)
        .def_property_readonly("ready", &CanvasProcessor::isReady)
        .def_property("fullScreen", &CanvasProcessor::isFullScreen, &CanvasProcessor::setFullScreen)
        .def("snapshot", [](CanvasProcessor *canvas, std::string filepath) {
        auto ext = filesystem::getFileExtension(filepath);

        auto writer = canvas->getNetwork()
            ->getApplication()
            ->getDataWriterFactory()
            ->getWriterForTypeAndExtension<Layer>(ext);
        if (!writer) {
            std::stringstream ss;
            ss << "No write for extension " << ext;
            throw Exception(ss.str().c_str());
        }

        auto layer = canvas->getVisibleLayer();
        writer->writeData(layer, filepath);
    });


    py::class_<PropertyWidget>(m.mainModule_, "PropertyWidget")
        .def_property_readonly("editorWidget", &PropertyWidget::getEditorWidget,
            py::return_value_policy::reference)
        .def_property_readonly("property", &PropertyWidget::getProperty,
            py::return_value_policy::reference);

    py::class_<PropertyEditorWidget>(m.mainModule_, "PropertyEditorWidget")
        .def_property("visibility", &PropertyEditorWidget::isVisible,
            &PropertyEditorWidget::setVisibility)
        .def_property("dimensions", &PropertyEditorWidget::getDimensions,
            &PropertyEditorWidget::setDimensions)
        .def_property("position", &PropertyEditorWidget::getPosition,
            &PropertyEditorWidget::setPosition)
        //.def_property("dockStatus", &PropertyEditorWidget::getDockStatus,
        //&PropertyEditorWidget::setDockStatus) //TODO expose dock status
        .def_property("sticky", &PropertyEditorWidget::isSticky, &PropertyEditorWidget::setSticky)
        ;


    struct propertyDelete { void operator()(Property* p) { if (p && p->getOwner() == nullptr) delete p; } };

    py::class_<Property , std::unique_ptr<Property, HasOwnerDeleter<Property>>> (m.mainModule_, "Property")
        .def_property("identifier", &Property::getIdentifier, &Property::setIdentifier)
        .def_property("displayName", &Property::getDisplayName, &Property::setDisplayName)
        .def_property("readOnly", &Property::getReadOnly, &Property::setReadOnly)
        .def_property("semantics", &Property::getSemantics,
            &Property::setSemantics)  // TODO expose semantics
        .def_property_readonly("classIdentifierForWidget", &Property::getClassIdentifierForWidget)
        .def_property_readonly("path", &Property::getPath)
        .def_property_readonly("invalidationLevel", &Property::getInvalidationLevel)
        .def_property_readonly("widgets", &Property::getWidgets)
        .def("updateWidgets", &Property::updateWidgets)
        .def("hasWidgets", &Property::hasWidgets)
        .def("setCurrentStateAsDefault", &Property::setCurrentStateAsDefault)
        .def("resetToDefaultState", &Property::resetToDefaultState);


    py::class_<CompositeProperty, Property, PropertyOwner , std::unique_ptr<CompositeProperty, HasOwnerDeleter<CompositeProperty>> >(m.mainModule_, "CompositeProperty")
        .def("__getattr__",
            [](CompositeProperty &po, std::string key) { return po.getPropertyByIdentifier(key); },
            py::return_value_policy::reference)
        ;


    pyOrdinalProperty<float>(m.mainModule_);
    pyOrdinalProperty<int>(m.mainModule_);
    pyOrdinalProperty<size_t>(m.mainModule_);
    pyOrdinalProperty<glm::i64>(m.mainModule_);
    pyOrdinalProperty<double>(m.mainModule_);
    pyOrdinalProperty<vec2>(m.mainModule_);
    pyOrdinalProperty<vec3>(m.mainModule_);
    pyOrdinalProperty<vec4>(m.mainModule_);
    pyOrdinalProperty<dvec2>(m.mainModule_);
    pyOrdinalProperty<dvec3>(m.mainModule_);
    pyOrdinalProperty<dvec4>(m.mainModule_);
    pyOrdinalProperty<ivec2>(m.mainModule_);
    pyOrdinalProperty<ivec3>(m.mainModule_);
    pyOrdinalProperty<ivec4>(m.mainModule_);
    pyOrdinalProperty<size2_t>(m.mainModule_);
    pyOrdinalProperty<size3_t>(m.mainModule_);
    pyOrdinalProperty<size4_t>(m.mainModule_);
    pyOrdinalProperty<mat2>(m.mainModule_);
    pyOrdinalProperty<mat3>(m.mainModule_);
    pyOrdinalProperty<mat4>(m.mainModule_);
    pyOrdinalProperty<dmat2>(m.mainModule_);
    pyOrdinalProperty<dmat3>(m.mainModule_);
    pyOrdinalProperty<dmat4>(m.mainModule_);

    py::class_<ButtonProperty, Property , NoDelete<ButtonProperty> >(m.mainModule_, "ButtonProperty")
        .def("press", &ButtonProperty::pressButton);

    py::class_<CameraProperty, CompositeProperty , NoDelete<CameraProperty>>(m.mainModule_, "CameraProperty")
        .def_property("lookFrom", &CameraProperty::getLookFrom, &CameraProperty::setLookFrom)
        .def_property("lookTo", &CameraProperty::getLookTo, &CameraProperty::setLookTo)
        .def_property("lookUp", &CameraProperty::getLookUp, &CameraProperty::setLookUp)
        .def_property_readonly("lookRight", &CameraProperty::getLookRight)
        .def_property("aspectRatio", &CameraProperty::getAspectRatio,
            &CameraProperty::setAspectRatio)
        .def_property("nearPlane", &CameraProperty::getNearPlaneDist,
            &CameraProperty::setNearPlaneDist)
        .def_property("farPlane", &CameraProperty::getFarPlaneDist,
            &CameraProperty::setFarPlaneDist)
        .def("setLook", &CameraProperty::setLook)
        .def_property_readonly("lookFromMinValue", &CameraProperty::getLookFromMinValue)
        .def_property_readonly("lookFromMaxValue", &CameraProperty::getLookFromMaxValue)
        .def_property_readonly("lookToMinValue", &CameraProperty::getLookToMinValue)
        .def_property_readonly("lookToMaxValue", &CameraProperty::getLookToMaxValue)
        .def("getWorldPosFromNormalizedDeviceCoords",
            &CameraProperty::getWorldPosFromNormalizedDeviceCoords)
        .def("getClipPosFromNormalizedDeviceCoords",
            &CameraProperty::getClipPosFromNormalizedDeviceCoords)
        .def("getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth",
            &CameraProperty::getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth)
        .def_property_readonly("viewMatrix", &CameraProperty::viewMatrix)
        .def_property_readonly("projectionMatrix", &CameraProperty::projectionMatrix)
        .def_property_readonly("inverseViewMatrix", &CameraProperty::inverseViewMatrix)
        .def_property_readonly("inverseProjectionMatrix", &CameraProperty::inverseProjectionMatrix)
        .def("adjustCameraToData", &CameraProperty::adjustCameraToData)
        .def("resetAdjustCameraToData", &CameraProperty::resetAdjustCameraToData);

    py::class_<TransferFunctionProperty, Property , NoDelete<TransferFunctionProperty>>(m.mainModule_, "TransferFunctionProperty")
        .def_property("mask", &TransferFunctionProperty::getMask,
            &TransferFunctionProperty::setMask)
        .def_property("zoomH", &TransferFunctionProperty::getZoomH,
            &TransferFunctionProperty::setZoomH)
        .def_property("zoomV", &TransferFunctionProperty::getZoomV,
            &TransferFunctionProperty::setZoomV)
        .def("save",
            [](TransferFunctionProperty *tf, std::string filename) {
                 tf->get().save(filename);
    })
        .def("load",
            [](TransferFunctionProperty *tf, std::string filename) {
                 tf->get().load(filename);
    })
        .def("clear", [](TransferFunctionProperty &tp) { tp.get().clearPoints(); })
        .def("addPoint", [](TransferFunctionProperty &tp, vec2 pos, vec3 color) {
        tp.get().addPoint(pos, vec4(color, pos.y));
    });

    py::class_<StringProperty, Property, NoDelete<StringProperty>> strProperty(m.mainModule_, "StringProperty");
    pyTemplateProperty<std::string, StringProperty>(strProperty);


    py::class_<FileProperty, Property, NoDelete<FileProperty>> fileProperty(m.mainModule_, "FileProperty");
    pyTemplateProperty<std::string, FileProperty>(fileProperty);

    m.mainModule_.attr("app") = py::cast(InviwoApplication::getPtr(), py::return_value_policy::reference);

    py::enum_<inviwo::PathType>(m.mainModule_, "PathType")
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

    








    auto module = InviwoApplication::getPtr()->getModuleByType<Python3Module>();
    if (module) {
        module->invokePythonInitCallbacks(&m);
    }


#ifdef IVW_ENABLE_MSVC_MEM_LEAK_TEST
    VLDEnable();
#endif

    return m.mainModule_.ptr();
}
