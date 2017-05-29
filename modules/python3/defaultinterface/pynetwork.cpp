/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

#include <modules/python3/pythonincluder.h>

#include <modules/python3/defaultinterface/pynetwork.h>
#include <modules/python3/pythoninterface/pythonparameterparser.h>
#include <modules/python3/pythoninterface/pyvalueparser.h>
#include <modules/python3/defaultinterface/utilities.h>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/networkutils.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorfactory.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/network/portconnection.h>
#include <inviwo/core/links/propertylink.h>
#include <inviwo/core/network/networklock.h>

namespace inviwo {

PyObject* py_getFactoryProcessors(PyObject* self, PyObject* args) {
    static PythonParameterParser tester;

    if (tester.parse(args) == 0) {
        auto keys = InviwoApplication::getPtr()->getProcessorFactory()->getKeys();
        return utilpy::makePyList(keys);
    }
    return nullptr;

}

PyObject* py_getProcessors(PyObject* self, PyObject* args) {
    static PythonParameterParser tester;

    if (tester.parse(args) == 0) {
        auto network = InviwoApplication::getPtr()->getProcessorNetwork();
        return utilpy::makePyList(util::transform(network->getProcessors(),
                                                  [](Processor* p) { return p->getIdentifier(); }));
    }
    return nullptr;
}

PyObject* py_addProcessor(PyObject* self, PyObject* args) {
    static PythonParameterParser tester;

    std::string classIdentifier;
    std::string identifier;
    if (tester.parse<std::string, std::string>(args, classIdentifier, identifier) == 2) {
        auto app = InviwoApplication::getPtr();
        auto network = app->getProcessorNetwork();

        try {
            auto p = app->getProcessorFactory()->create(classIdentifier);
            identifier = p->setIdentifier(identifier);

            network->addProcessor(p.get());
            util::autoLinkProcessor(network, p.get());
            p.release();

            return PyValueParser::toPyObject(identifier);
        } catch (Exception& exception) {
            std::string msg = "Unable to create processor " + classIdentifier + " due to " +
                              exception.getMessage();
            PyErr_SetString(PyExc_ValueError, msg.c_str());
            return nullptr;
        }
    }
    return nullptr;
}

PyObject* py_removeProcessor(PyObject* self, PyObject* args) {
    static PythonParameterParser tester;

    std::string identifier;
    if (tester.parse<std::string>(args, identifier) == 1) {
        auto app = InviwoApplication::getPtr();
        auto network = app->getProcessorNetwork();

        if (auto p = network->getProcessorByIdentifier(identifier)) {
            network->removeAndDeleteProcessor(p);
            Py_RETURN_NONE;
        }
    }
    std::string msg = std::string("Could not find a processor with identifier: ") + identifier;
    PyErr_SetString(PyExc_ValueError, msg.c_str());
    return nullptr;
}

PyObject* py_getConnections(PyObject* self, PyObject* args) {
    static PythonParameterParser tester;

    if (tester.parse(args) == 0) {
        auto network = InviwoApplication::getPtr()->getProcessorNetwork();
        return utilpy::makePyList(util::transform(network->getConnections(), [](const PortConnection& p) {
            return std::make_pair(std::make_pair(p.getOutport()->getProcessor()->getIdentifier(),
                                                 p.getOutport()->getIdentifier()),
                                  std::make_pair(p.getInport()->getProcessor()->getIdentifier(),
                                                 p.getInport()->getIdentifier()));

        }));
    }
    return nullptr;
}

std::pair<Processor*, Processor*> getProcessorPair(const std::string& outProcessor,
                                                   const std::string& inProcessor) {
    auto app = InviwoApplication::getPtr();
    auto network = app->getProcessorNetwork();

    auto out = network->getProcessorByIdentifier(outProcessor);
    auto in = network->getProcessorByIdentifier(inProcessor);

    if (out && in) {
        return std::make_pair(out, in);
    } else if (!out) {
        std::string msg = "Could not find a processor with identifier: " + outProcessor;
        PyErr_SetString(PyExc_ValueError, msg.c_str());
    } else if (!in) {
        std::string msg = "Could not find a processor with identifier: " + inProcessor;
        PyErr_SetString(PyExc_ValueError, msg.c_str());
    }
    return std::make_pair(nullptr, nullptr);
}

std::pair<Outport*, Inport*> getPortPair(const std::string& outProcessor,
                                         const std::string& outPort, const std::string& inProcessor,
                                         const std::string& inPort) {
    auto p = getProcessorPair(outProcessor, inProcessor);
    if (p.first && p.second) {
        auto outport = p.first->getOutport(outPort);
        auto inport = p.second->getInport(inPort);

        if (outport && inport) {
            return std::make_pair(outport, inport);
        } else if (!outport) {
            std::string msg = "Count not find outport with identifier: " + outPort;
            PyErr_SetString(PyExc_ValueError, msg.c_str());
        } else if (!inport) {
            std::string msg = "Count not find inport with identifier: " + inPort;
            PyErr_SetString(PyExc_ValueError, msg.c_str());
        }
    }
    return std::make_pair(nullptr, nullptr);
}

PyObject* py_addConnection(PyObject* self, PyObject* args) {
    static PythonParameterParser tester;

    std::string outProcessor;
    std::string outPort;

    std::string inProcessor;
    std::string inPort;

    if (tester.parse<std::string, std::string, std::string, std::string>(
            args, outProcessor, outPort, inProcessor, inPort) == 4) {
        auto p = getPortPair(outProcessor, outPort, inProcessor, inPort);
        if (p.first && p.second) {
            auto app = InviwoApplication::getPtr();
            auto network = app->getProcessorNetwork();
            network->addConnection(p.first, p.second);
            Py_RETURN_NONE;
        }
    }
    return nullptr;
}

PyObject* py_removeConnection(PyObject* self, PyObject* args) {
    static PythonParameterParser tester;

    std::string outProcessor;
    std::string outPort;

    std::string inProcessor;
    std::string inPort;

    if (tester.parse<std::string, std::string, std::string, std::string>(
            args, outProcessor, outPort, inProcessor, inPort) == 4) {
        auto p = getPortPair(outProcessor, outPort, inProcessor, inPort);
        if (p.first && p.second) {
            auto app = InviwoApplication::getPtr();
            auto network = app->getProcessorNetwork();
            network->removeConnection(p.first, p.second);
            Py_RETURN_NONE;
        }
    }
    return nullptr;
}

PyObject* py_getLinks(PyObject* self, PyObject* args) {
    static PythonParameterParser tester;

    if (tester.parse(args) == 0) {
        auto network = InviwoApplication::getPtr()->getProcessorNetwork();
        return utilpy::makePyList(util::transform(network->getLinks(), [](const PropertyLink& l) {
            return std::make_pair(joinString(l.getSource()->getPath(), "."),
                                  joinString(l.getDestination()->getPath(), "."));

        }));
    }
    return nullptr;
}

PyObject* py_addLink(PyObject* self, PyObject* args) {
    static PythonParameterParser tester;

    std::string source;
    std::string target;

    if (tester.parse<std::string, std::string>(args, source, target) == 2) {
        auto app = InviwoApplication::getPtr();
        auto network = app->getProcessorNetwork();

        auto s = network->getProperty(splitString(source, '.'));
        auto t = network->getProperty(splitString(target, '.'));

        if (s && t) {
            network->addLink(s, t);
            Py_RETURN_NONE;
        } else if (!s) {
            std::string msg = "Count not find property with path: " + source;
            PyErr_SetString(PyExc_ValueError, msg.c_str());
        } else if (!t) {
            std::string msg = "Count not find property with path: " + target;
            PyErr_SetString(PyExc_ValueError, msg.c_str());
        }
    }
    return nullptr;
}

PyObject* py_removeLink(PyObject* self, PyObject* args) {
    static PythonParameterParser tester;

    std::string source;
    std::string target;

    if (tester.parse<std::string, std::string>(args, source, target) == 2) {
        auto app = InviwoApplication::getPtr();
        auto network = app->getProcessorNetwork();

        auto s = network->getProperty(splitString(source, '.'));
        auto t = network->getProperty(splitString(target, '.'));

        if (s && t) {
            network->removeLink(s, t);
            Py_RETURN_NONE;
        } else if (!s) {
            std::string msg = "Count not find property with path: " + source;
            PyErr_SetString(PyExc_ValueError, msg.c_str());
        } else if (!t) {
            std::string msg = "Count not find property with path: " + target;
            PyErr_SetString(PyExc_ValueError, msg.c_str());
        }
    }
    return nullptr;
}

PyObject* py_clearNetwork(PyObject* self, PyObject* args) {
    static PythonParameterParser tester;
    if (tester.parse(args) == -1) {
        return nullptr;
    }
    auto app = InviwoApplication::getPtr();
    app->getWorkspaceManager()->clear();
    Py_RETURN_NONE;
}

PyObject* py_loadNetwork(PyObject* self, PyObject* args) {
    static PythonParameterParser tester;
    std::string filename;
    if (tester.parse(args, filename) == -1) {
        return nullptr;
    }
    if (!filesystem::fileExists(filename)) {
        filename = filesystem::getPath(PathType::Workspaces) + "/" + filename;

        if (!filesystem::fileExists(filename)) {
            std::string msg = std::string("loadWorkspace() could not find file") + filename;
            PyErr_SetString(PyExc_TypeError, msg.c_str());
            return nullptr;
        }
    }

    {
        auto app = InviwoApplication::getPtr();
        NetworkLock lock(app->getProcessorNetwork());
        app->getWorkspaceManager()->clear();
        try {
            app->getWorkspaceManager()->load(filename, [&](ExceptionContext ec) {
                try {
                    throw;
                } catch (const IgnoreException& e) {
                    util::log(e.getContext(), "Incomplete network loading " + filename +
                                                  " due to " + e.getMessage(),
                              LogLevel::Error);
                }
            });
        } catch (const Exception& e) {
            PyErr_SetString(PyExc_TypeError, std::string("Unable to load network " + filename +
                                                         " due to " + e.getMessage())
                                                 .c_str());
            app->getWorkspaceManager()->clear();
            return nullptr;
        }
    }
    Py_RETURN_NONE;
}

PyObject* py_saveNetwork(PyObject* self, PyObject* args) {
    static PythonParameterParser tester(1);
    std::string filename;
    bool setAsFilename;
    if (tester.parse(args, filename, setAsFilename) == -1) {
        return nullptr;
    }

    auto app = InviwoApplication::getPtr();
    try {
        app->getWorkspaceManager()->save(filename, [&](ExceptionContext ec) {
            try {
                throw;
            } catch (const IgnoreException& e) {
                util::log(e.getContext(),
                          "Incomplete network save " + filename + " due to " + e.getMessage(),
                          LogLevel::Error);
            }
        });
    } catch (const Exception& e) {
        PyErr_SetString(PyExc_TypeError, std::string("Unable to save network " + filename +
                                                     " due to " + e.getMessage())
                                             .c_str());
        return nullptr;
    }
    Py_RETURN_NONE;
}

}  // namespace
