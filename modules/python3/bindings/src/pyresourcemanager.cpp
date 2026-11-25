/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <inviwopy/pyresourcemanager.h>

#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <pybind11/trampoline_self_life_support.h>

#include <inviwo/core/resourcemanager/resource.h>
#include <inviwo/core/resourcemanager/resourcemanager.h>
#include <inviwo/core/resourcemanager/resourcemanagerobserver.h>
#include <inviwo/core/util/glmfmt.h>
#include <inviwo/core/util/safecstr.h>
#include <inviwo/core/util/stdextensions.h>

#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

#include <fmt/format.h>

#include <iterator>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace inviwo {

namespace {

// Resource::desc is a string_view that is expected to refer to a string with static storage
// duration (usually a type name). Strings coming from python have no such guarantee, so we
// intern them here. The set is node based, hence the stored strings never move.
std::string_view internDescription(std::string_view desc) {
    static std::set<std::string, std::less<>> descriptions;
    static std::mutex mutex;
    const std::scoped_lock lock{mutex};
    const auto it = descriptions.find(desc);
    if (it != descriptions.end()) {
        return *it;
    } else {
        return *descriptions.emplace(desc).first;
    }
}

std::string toString(const Resource& resource) {
    return fmt::format("Resource(dims={}, format={}, desc='{}', meta={}, sizeInBytes={})",
                       resource.dims, resource.format, resource.desc,
                       resource.meta ? fmt::format("'{}'", resource.meta->source) : "None",
                       resource.sizeInBytes());
}

#include <warn/push>
#include <warn/ignore/attributes>
// Allow implementing a ResourceManagerObserver in Python by overriding the virtual functions
class ResourceManagerObserverTrampoline : public ResourceManagerObserver,
                                          public pybind11::trampoline_self_life_support {
public:
    using ResourceManagerObserver::ResourceManagerObserver;

    virtual void onWillAddResource(size_t group, size_t index, const Resource& resource) override {
        PYBIND11_OVERRIDE(void, ResourceManagerObserver, onWillAddResource, group, index, resource);
    }
    virtual void onDidAddResource(size_t group, size_t index, const Resource& resource) override {
        PYBIND11_OVERRIDE(void, ResourceManagerObserver, onDidAddResource, group, index, resource);
    }
    virtual void onWillUpdateResource(size_t group, size_t index,
                                      const Resource& resource) override {
        PYBIND11_OVERRIDE(void, ResourceManagerObserver, onWillUpdateResource, group, index,
                          resource);
    }
    virtual void onDidUpdateResource(size_t group, size_t index,
                                     const Resource& resource) override {
        PYBIND11_OVERRIDE(void, ResourceManagerObserver, onDidUpdateResource, group, index,
                          resource);
    }
    virtual void onWillRemoveResource(size_t group, size_t index,
                                      const Resource& resource) override {
        PYBIND11_OVERRIDE(void, ResourceManagerObserver, onWillRemoveResource, group, index,
                          resource);
    }
    virtual void onDidRemoveResource(size_t group, size_t index,
                                     const Resource& resource) override {
        PYBIND11_OVERRIDE(void, ResourceManagerObserver, onDidRemoveResource, group, index,
                          resource);
    }
};
#include <warn/pop>

template <typename Key>
void exposeResourceKey(pybind11::module& m, pybind11::classh<ResourceManager>& rm) {
    namespace py = pybind11;
    using KeyType = decltype(Key::key);
    constexpr auto group = util::index_of<Key, ResourceManager::Keys>();

    py::classh<Key> cls(m, SafeCStr{Key::name}.c_str());
    cls.def(py::init<>())
        .def(py::init([](KeyType k) { return Key{k}; }), py::arg("key"))
        .def_readwrite("key", &Key::key)
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def("__hash__", [](const Key& k) { return std::hash<Key>{}(k); })
        .def("__repr__",
             [](const Key& k) { return fmt::format("resource.{}(key={})", Key::name, k.key); });
    cls.attr("name") = Key::name;
    cls.attr("group") = group;

    m.def(
        "add", [](const Key& key, Resource resource) { resource::add(key, std::move(resource)); },
        py::arg("key"), py::arg("resource"),
        "Add or update a resource in the application ResourceManager (no-op if resource "
        "tracking is disabled)");
    m.def(
        "move",
        [](const Key& oldKey, const Key& newKey, Resource resource) {
            resource::move(oldKey, newKey, std::move(resource));
        },
        py::arg("oldKey"), py::arg("newKey"), py::arg("resource"),
        "Move a resource from oldKey to newKey in the application ResourceManager");
    m.def(
        "remove", [](const Key& key) { resource::remove(key); }, py::arg("key"),
        "Remove a resource from the application ResourceManager");
    m.def(
        "meta", [](const Key& key, const ResourceMeta& meta) { resource::meta(key, meta); },
        py::arg("key"), py::arg("meta"),
        "Update the meta data of a resource in the application ResourceManager");

    rm.def("add", &ResourceManager::add<Key>, py::arg("key"), py::arg("resource"),
           "Add a resource, or update it if the key already exists")
        .def("move", &ResourceManager::move<Key>, py::arg("oldKey"), py::arg("newKey"),
             py::arg("resource"), "Remove the resource at oldKey and add resource at newKey")
        .def("meta", &ResourceManager::meta<Key>, py::arg("key"), py::arg("meta"),
             "Update the meta data of the resource with the given key")
        .def("remove", &ResourceManager::remove<Key>, py::arg("key"),
             "Remove the resource with the given key, returns the removed Resource or None");
}

}  // namespace

void exposeResourceManager(pybind11::module& m) {
    namespace py = pybind11;

    py::classh<ResourceMeta>(m, "ResourceMeta")
        .def(py::init<>())
        .def(py::init([](std::string source) { return ResourceMeta{std::move(source)}; }),
             py::arg("source"))
        .def_readwrite("source", &ResourceMeta::source)
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def("__repr__", [](const ResourceMeta& meta) {
            return fmt::format("ResourceMeta(source='{}')", meta.source);
        });

    py::classh<Resource>(m, "Resource",
                         "Describes a tracked resource, e.g. a volume or an image in RAM, "
                         "on the GPU, or in python")
        .def(py::init([](glm::size4_t dims, DataFormatId format, std::string_view desc,
                         std::optional<ResourceMeta> meta) {
                 return Resource{.dims = dims,
                                 .format = format,
                                 .desc = internDescription(desc),
                                 .meta = std::move(meta)};
             }),
             py::arg("dims") = glm::size4_t{0, 0, 0, 0},
             py::arg("format") = DataFormatId::NotSpecialized, py::arg("desc") = "",
             py::arg("meta") = py::none())
        .def_readwrite("dims", &Resource::dims, "Dimensions, zeros are treated as one")
        .def_readwrite("format", &Resource::format)
        .def_property(
            "desc", [](const Resource& r) { return r.desc; },
            [](Resource& r, std::string_view desc) { r.desc = internDescription(desc); },
            "Description of the resource, usually the type name")
        .def_readwrite("meta", &Resource::meta, "Optional meta data, e.g. the source file")
        .def("sizeInBytes", &Resource::sizeInBytes,
             "Size of the resource in bytes, i.e. the product of dims and format size")
        .def("__repr__", &toString);

    py::classh<ResourceManagerObserver, ResourceManagerObserverTrampoline>(
        m, "ResourceManagerObserver",
        "Derive from this class and override the on* functions to get notified about changes "
        "in a ResourceManager. The group is the index into ResourceManager.names, i.e. "
        "0 = RAM, 1 = GL, 2 = PY, and index is the position within that group.")
        .def(py::init_alias<>())
        .def("onWillAddResource", &ResourceManagerObserver::onWillAddResource, py::arg("group"),
             py::arg("index"), py::arg("resource"))
        .def("onDidAddResource", &ResourceManagerObserver::onDidAddResource, py::arg("group"),
             py::arg("index"), py::arg("resource"))
        .def("onWillUpdateResource", &ResourceManagerObserver::onWillUpdateResource,
             py::arg("group"), py::arg("index"), py::arg("resource"))
        .def("onDidUpdateResource", &ResourceManagerObserver::onDidUpdateResource, py::arg("group"),
             py::arg("index"), py::arg("resource"))
        .def("onWillRemoveResource", &ResourceManagerObserver::onWillRemoveResource,
             py::arg("group"), py::arg("index"), py::arg("resource"))
        .def("onDidRemoveResource", &ResourceManagerObserver::onDidRemoveResource, py::arg("group"),
             py::arg("index"), py::arg("resource"));

    py::classh<ResourceManager> rm(
        m, "ResourceManager",
        "Keeps track of allocated resources in three groups: RAM, GL, and PY. Access the "
        "application instance via inviwopy.app.resourceManager. Note that resource tracking "
        "is disabled by default, enable it in the system settings "
        "(app.systemSettings.enableResourceTracking).");
    rm.def(py::init<>())
        .def_readonly_static("names", &ResourceManager::names, "The group names: RAM, GL, PY")
        .def("totalSize", &ResourceManager::totalSize, "Total number of resources in all groups")
        .def("size", &ResourceManager::size, py::arg("group"),
             "Number of resources in the given group")
        .def("totalByteSize", &ResourceManager::totalByteSize, py::arg("group"),
             "Sum of the size in bytes of all resources in the given group")
        .def(
            "get",
            [](const ResourceManager& manager, size_t group, size_t index) {
                return manager.get(group, index);
            },
            py::arg("group"), py::arg("index"), py::return_value_policy::copy,
            "Get a copy of the resource at index in group, or None if out of range")
        .def(
            "resources",
            [](const ResourceManager& manager, size_t group) {
                std::vector<Resource> res;
                const auto n = manager.size(group);
                res.reserve(n);
                for (size_t i = 0; i < n; ++i) {
                    if (const auto* r = manager.get(group, i)) res.push_back(*r);
                }
                return res;
            },
            py::arg("group"), "Get a list of copies of all the resources in the given group")
        .def("clear", &ResourceManager::clear, "Remove all resources")
        .def(
            "addObserver",
            [](ResourceManager& manager, ResourceManagerObserver* observer) {
                manager.addObserver(observer);
            },
            py::arg("observer"),
            "Register an observer. The ResourceManager only holds a reference, so the python "
            "object has to be kept alive by the caller.")
        .def(
            "removeObserver",
            [](ResourceManager& manager, ResourceManagerObserver* observer) {
                manager.removeObserver(observer);
            },
            py::arg("observer"))
        .def(
            "isObservedBy",
            [](const ResourceManager& manager, ResourceManagerObserver* observer) {
                return manager.isObservedBy(observer);
            },
            py::arg("observer"))
        .def("__repr__", [](const ResourceManager& manager) {
            std::string res = "ResourceManager(";
            for (size_t group = 0; group < ResourceManager::names.size(); ++group) {
                fmt::format_to(std::back_inserter(res), "{}{}: {} items, {} bytes",
                               group == 0 ? "" : ", ", ResourceManager::names[group],
                               manager.size(group), manager.totalByteSize(group));
            }
            res += ")";
            return res;
        });

    auto resourceModule = m.def_submodule(
        "resource",
        "Resource keys (RAM, GL, PY) and functions to register resources in the application "
        "ResourceManager, mirroring the C++ inviwo::resource namespace");

    exposeResourceKey<resource::RAM>(resourceModule, rm);
    exposeResourceKey<resource::GL>(resourceModule, rm);
    exposeResourceKey<resource::PY>(resourceModule, rm);

    resourceModule.def(
        "toPY",
        [](const py::object& obj) {
            return resource::PY{py::module_::import("builtins").attr("id")(obj).cast<size_t>()};
        },
        py::arg("obj"),
        "Create a PY key for a python object, using id(obj). This is the key used by Inviwo "
        "for numpy arrays backing VolumePY representations.");
}

}  // namespace inviwo
