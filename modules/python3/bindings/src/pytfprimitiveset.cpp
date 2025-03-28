/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2025 Inviwo Foundation
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

#include <inviwopy/pytfprimitiveset.h>

#include <pybind11/operators.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/numpy.h>
#include <pybind11/typing.h>

#include <inviwo/core/datastructures/tfprimitive.h>
#include <inviwo/core/datastructures/tfprimitiveset.h>
#include <inviwo/core/datastructures/transferfunction.h>
#include <inviwo/core/datastructures/isovaluecollection.h>
#include <inviwo/core/util/colorconversion.h>

#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

#include <sstream>
#include <utility>

namespace inviwo {

namespace {

template <typename T>
T try_cast(pybind11::handle obj, const std::string& errorMsg) {
    try {
        return obj.cast<T>();
    } catch (const pybind11::cast_error&) {
        throw(pybind11::value_error(errorMsg));
    }
}

void addPoints(TFPrimitiveSet* ps, const pybind11::list& values) {
    for (auto&& item : values) {
        const auto data = try_cast<pybind11::list>(
            item, "expected a list of [pos, hex color] and/or [pos, alpha, hex color]");
        if (data.size() == 2) {
            const auto pos = try_cast<double>(data[0], "expected a double (position)");
            const auto color =
                color::hex2rgba(try_cast<std::string>(data[1], "expected a hex color code"));
            ps->add(pos, color);
        } else if (data.size() == 3) {
            const auto pos =
                try_cast<double>(data[0], "expected a double (position)" +
                                              data[0].attr("__repr__")().cast<std::string>());
            const auto alpha = try_cast<double>(data[1], "expected a double (alpha)");
            const auto color =
                color::hex2rgba(try_cast<std::string>(data[2], "expected a hex color code"));
            ps->add(pos, vec4(vec3(color), static_cast<float>(alpha)));
        } else {
            throw pybind11::value_error(
                "expected a list of [pos, hex color] and/or [pos, alpha, hex color]");
        }
    }
}

}  // namespace

void exposeTFPrimitiveSet(pybind11::module& m) {
    namespace py = pybind11;

    py::enum_<TFPrimitiveSetType>(m, "TFPrimitiveSetType")
        .value("Relative", TFPrimitiveSetType::Relative)
        .value("Absolute", TFPrimitiveSetType::Absolute);

    py::classh<TFPrimitiveData>(m, "TFPrimitiveData")
        .def(py::init())
        .def(py::init<double, vec4>(), py::arg("pos"), py::arg("colorAndOpacity"))
        .def(py::init([](double pos, double opacity, vec3 color) {
                 return TFPrimitiveData{pos, vec4{color, opacity}};
             }),
             py::arg("pos"), py::arg("opacity") = 0.0, py::arg("color") = vec3{0, 0, 0})
        .def(py::init([](double pos, std::string_view color) {
                 return TFPrimitiveData{pos, color::hex2rgba(color)};
             }),
             py::arg("pos"), py::arg("colorAndAlpha"))
        .def(py::init([](const py::list& list) {
            TFPrimitiveData data{};

            if (list.size() == 2) {
                auto first = list[0];
                auto second = list[1];

                if (py::isinstance<py::float_>(first) && py::isinstance<py::list>(second) &&
                    second.cast<py::list>().size() == 4) {
                    auto color = second.cast<py::list>();

                    data.pos = list[0].cast<double>();
                    data.color.x = color[0].cast<float>();
                    data.color.y = color[1].cast<float>();
                    data.color.z = color[2].cast<float>();
                    data.color.a = color[2].cast<float>();
                    return data;
                } else if (py::isinstance<py::list>(first) && first.cast<py::list>().size() == 2 &&
                           py::isinstance<py::list>(second) &&
                           second.cast<py::list>().size() == 3) {

                    auto pos = first.cast<py::list>();
                    auto color = second.cast<py::list>();

                    data.pos = pos[0].cast<double>();
                    data.color.x = color[0].cast<float>();
                    data.color.y = color[1].cast<float>();
                    data.color.z = color[2].cast<float>();
                    data.color.a = pos[1].cast<float>();
                    return data;
                }
            } else if (list.size() == 3) {
                auto first = list[0];
                auto second = list[1];
                auto third = list[2];
                if (py::isinstance<py::float_>(first) && py::isinstance<py::float_>(second) &&
                    py::isinstance<py::list>(third) && third.cast<py::list>().size() == 3) {
                    auto color = third.cast<py::list>();
                    data.pos = first.cast<double>();
                    data.color.x = color[0].cast<float>();
                    data.color.y = color[1].cast<float>();
                    data.color.z = color[2].cast<float>();
                    data.color.a = second.cast<float>();
                    return data;
                }
            }
            throw std::invalid_argument{
                "Expected (pos, alpha, (r,b,g)) or ((pos, alpha), (r,b,g)) or "
                "(pos, (r,b,g,alpha))"};
        }))
        .def_readwrite("pos", &TFPrimitiveData::pos)
        .def_readwrite("color", &TFPrimitiveData::color)
        .def("__repr__",
             [](const TFPrimitiveData& p) {
                 std::ostringstream oss;
                 oss << "TFPrimitiveData: [" << p.pos << ", " << color::rgba2hex(p.color) << "]";
                 return oss.str();
             })
        /// Comparisons
        .def(py::self == py::self)
        .def(py::self != py::self);

    py::implicitly_convertible<py::list, TFPrimitiveData>();

    py::classh<TFPrimitive>(m, "TFPrimitive")
        .def(py::init([](double pos, const vec4& color) { return new TFPrimitive(pos, color); }),
             py::arg("pos") = 0.0, py::arg("color") = vec4(0.0f))
        .def(py::init([](double iso, const std::string& color) {
            return new TFPrimitive{iso, color::hex2rgba(color)};
        }))
        .def_property("data", &TFPrimitive::getData, &TFPrimitive::setData)
        .def_property("pos", &TFPrimitive::getPosition, &TFPrimitive::setPosition)
        .def_property("alpha", &TFPrimitive::getAlpha, &TFPrimitive::setAlpha)
        .def_property("color", &TFPrimitive::getColor,
                      py::overload_cast<const vec3&>(&TFPrimitive::setColor))
        .def_property("color", &TFPrimitive::getColor,
                      py::overload_cast<const vec4&>(&TFPrimitive::setColor))
        .def("setPositionAlpha",
             [](TFPrimitive& ps, double pos, float alpha) { ps.setPositionAlpha(pos, alpha); })
        .def("setPositionAlpha",
             [](TFPrimitive& ps, const dvec2& pos) { ps.setPositionAlpha(pos); })
        .def("__repr__",
             [](const TFPrimitive& ps) {
                 std::ostringstream oss;
                 oss << "<TFPrimitive: " << ps.getPosition() << ", "
                     << color::rgba2hex(ps.getColor()) << ">";
                 return oss.str();
             })
        /// Comparisons
        .def(py::self == py::self)
        .def(py::self != py::self);

    py::classh<TFPrimitiveSet>(m, "TFPrimitiveSet")
        .def(py::init([](const std::vector<TFPrimitiveData>& values, TFPrimitiveSetType type) {
                 return new TFPrimitiveSet(values, type);
             }),
             py::arg("values") = std::vector<TFPrimitiveData>{},
             py::arg("type") = TFPrimitiveSetType::Relative)
        .def_property_readonly("type", &TFPrimitiveSet::getType)
        .def_property_readonly("range", &TFPrimitiveSet::getRange)
        .def_property_readonly("size", &TFPrimitiveSet::size)
        .def_property_readonly("empty", &TFPrimitiveSet::empty)
        // interface for operator[]
        .def(
            "__getitem__",
            [](const TFPrimitiveSet& ps, size_t i) {
                if (i >= ps.size()) throw py::index_error();
                return &ps[i];
            },
            py::return_value_policy::reference_internal)
        .def("__setitem__",
             [](TFPrimitiveSet& ps, size_t i, const TFPrimitiveData& primitive) {
                 if (i >= ps.size()) throw py::index_error();
                 ps[i].setData(primitive);
             })
        // sequence protocol operations
        .def(
            "__iter__",
            [](const TFPrimitiveSet& ps) { return py::make_iterator(ps.begin(), ps.end()); },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */)
        //
        .def("clear", [](TFPrimitiveSet& ps) { ps.clear(); })
        .def("add", py::overload_cast<const TFPrimitive&>(&TFPrimitiveSet::add))

        .def("add", py::overload_cast<double, const vec4&>(&TFPrimitiveSet::add))
        .def("add", py::overload_cast<const dvec2&>(&TFPrimitiveSet::add))
        .def("add", py::overload_cast<const TFPrimitiveData&>(&TFPrimitiveSet::add))
        .def("add", py::overload_cast<const std::vector<TFPrimitiveData>&>(&TFPrimitiveSet::add))

        .def("set",
             [](TFPrimitiveSet& ps, const py::typing::Iterable<TFPrimitiveData>& items) {
                 std::vector<TFPrimitiveData> points;
                 for (auto item : items) {
                     points.emplace_back(item.cast<TFPrimitiveData>());
                 }
                 ps.set(points);
             })

        .def("remove", [](TFPrimitiveSet& ps, TFPrimitive& primitive) { ps.remove(primitive); })
        .def("__repr__", [](const TFPrimitiveSet& ps) {
            std::ostringstream oss;
            oss << "<TFPrimitiveSet:  " << ps.size() << " primitives";
            for (const auto& p : ps) {
                oss << "\n    " << p.getPosition() << ", " << color::rgba2hex(p.getColor());
            }
            oss << ">";
            return oss.str();
        });

    py::classh<TransferFunction, TFPrimitiveSet>(m, "TransferFunction")
        .def(py::init([]() { return TransferFunction(); }))
        .def(py::init([](const std::vector<TFPrimitiveData>& values) {
                 return new TransferFunction(values);
             }),
             py::arg("values"))
        .def(py::init([](const std::vector<TFPrimitiveData>& values, TFPrimitiveSetType type) {
                 return new TransferFunction(values, type);
             }),
             py::arg("values"), py::arg("type"))
        .def(py::init([](py::list values, TFPrimitiveSetType type) {
                 auto* tf = new TransferFunction{{}, type};
                 addPoints(tf, values);
                 return tf;
             }),
             py::arg("values"), py::arg("type") = TFPrimitiveSetType::Relative)
        .def_property(
            "mask", [](TransferFunction& tf) { return dvec2(tf.getMaskMin(), tf.getMaskMax()); },
            [](TransferFunction& tf, const dvec2& mask) {
                tf.setMaskMin(mask.x);
                tf.setMaskMax(mask.y);
            })
        .def("clearMask", &TransferFunction::clearMask)
        .def("sample", [](TransferFunction& tf, double v) -> vec4 { return tf.sample(v); })
        .def_static("save", &TransferFunction::save)
        .def_static("load", &TransferFunction::load)
        .def("__repr__", [](const TransferFunction& tf) {
            std::ostringstream oss;
            oss << "<TransferFunction:  " << tf.size() << " points";
            for (const auto& p : tf) {
                oss << "\n    " << p.getPosition() << ", " << color::rgba2hex(p.getColor());
            }
            oss << ">";
            return oss.str();
        });

    py::classh<IsoValueCollection, TFPrimitiveSet>(m, "IsoValueCollection")
        .def(py::init([](const std::vector<TFPrimitiveData>& values, TFPrimitiveSetType type) {
                 return new IsoValueCollection(values, type);
             }),
             py::arg("values") = std::vector<TFPrimitiveData>{},
             py::arg("type") = TFPrimitiveSetType::Relative)
        .def(py::init([](py::list values, TFPrimitiveSetType type) {
                 auto* tf = new IsoValueCollection{{}, type};
                 addPoints(tf, values);
                 return tf;
             }),
             py::arg("values"), py::arg("type") = TFPrimitiveSetType::Relative)
        .def_static("save", &IsoValueCollection::save)
        .def_static("load", &IsoValueCollection::load)
        .def("__repr__", [](const IsoValueCollection& ivc) {
            std::ostringstream oss;
            oss << "<IsoValueCollection:  " << ivc.size() << " isovalues";
            for (const auto& p : ivc) {
                oss << "\n    " << p.getPosition() << ", " << color::rgba2hex(p.getColor());
            }
            oss << ">";
            return oss.str();
        });
}

}  // namespace inviwo
