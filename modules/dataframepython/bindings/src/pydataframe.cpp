/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2023 Inviwo Foundation
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

#include <ivwdataframe/pydataframe.h>

#include <warn/push>
#include <warn/ignore/shadow>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <warn/pop>

#include <inviwo/dataframe/datastructures/column.h>
#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/dataframe/util/dataframeutil.h>

#include <inviwo/core/util/defaultvalues.h>
#include <inviwo/core/util/safecstr.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <modules/python3/pyportutils.h>

#include <fmt/format.h>

namespace py = pybind11;

namespace inviwo {

namespace {

struct DataFrameAddColumnReg {
    template <typename T>
    auto operator()(py::class_<DataFrame>& d) {
        auto classname = Defaultvalues<T>::getName();

        d.def(
            fmt::format("add{}Column", classname).c_str(),
            [](DataFrame& d, const std::string& header, const size_t size = 0, Unit unit = Unit(),
               std::optional<dvec2> range = std::nullopt) {
                return d.addColumn<T>(header, size, unit, range);
            },
            py::arg("header"), py::arg("size") = 0, py::arg("unit") = Unit(),
            py::arg("range") = std::nullopt);

        d.def(
            fmt::format("add{}Column", classname).c_str(),
            [](DataFrame& d, std::string header, std::vector<T> data, Unit unit = Unit(),
               std::optional<dvec2> range = std::nullopt) {
                return d.addColumn(std::move(header), std::move(data), unit, range);
            },
            py::arg("header"), py::arg("data"), py::arg("unit") = Unit(),
            py::arg("range") = std::nullopt);
    }
};

struct TemplateColumnReg {
    template <typename T>
    auto operator()(pybind11::module& m) {
        using C = TemplateColumn<T>;
        auto classname = Defaultvalues<T>::getName() + "Column";

        py::class_<C, Column> col(m, classname.c_str());
        col.def_property_readonly("buffer", [](C& c) { return c.getTypedBuffer(); })
            .def(py::init<std::string_view>())
            .def("add", py::overload_cast<const T&>(&C::add))
            .def("add", py::overload_cast<std::string_view>(&C::add))
            .def("append", [](C& c, C& src) { c.append(src); })
            .def("set", &C::set)
            .def(
                "get",
                [](const C& c, size_t i) {
                    if (i >= c.getSize()) throw py::index_error();
                    return c.get(i);
                },
                py::arg("i"))
            .def(
                "getAsString",
                [](const C& c, size_t i) {
                    if (i >= c.getSize()) throw py::index_error();
                    return c.getAsString(i);
                },
                py::arg("i"))
            .def("__repr__",
                 [classname](C& c) {
                     return fmt::format("<{}: '{}', {}, {}>", classname, c.getHeader(), c.getSize(),
                                        c.getBuffer()->getDataFormat()->getString());
                 })
            .def(
                "__iter__", [](const C& c) { return py::make_iterator(c.begin(), c.end()); },
                py::keep_alive<0, 1>());
    }
};

}  // namespace

void exposeDataFrame(pybind11::module& m) {
    py::enum_<ColumnType>(m, "ColumnType")
        .value("Index", ColumnType::Index)
        .value("Ordinal", ColumnType::Ordinal)
        .value("Categorical", ColumnType::Categorical);

    py::class_<Column>(m, "Column")
        .def_property("header", &Column::getHeader, &Column::setHeader)
        .def_property_readonly("buffer", [](Column& self) { return self.getBuffer(); })
        .def_property_readonly("size", &Column::getSize)
        .def_property_readonly("type", &Column::getColumnType)
        .def_property("unit", &Column::getUnit, &Column::setUnit)
        .def_property("customRange", &Column::getCustomRange, &Column::setCustomRange)
        .def_property_readonly("range", &Column::getRange)
        .def_property_readonly("dataRange", &Column::getDataRange)
        .def("__repr__", [](Column& c) {
            return fmt::format("<Column: {}{: [}, {}, {}>", c.getHeader(), c.getUnit(), c.getSize(),
                               c.getBuffer()->getDataFormat()->getString());
        });

    using Scalars = std::tuple<float, double, int, glm::i64, size_t, std::uint32_t>;
    util::for_each_type<Scalars>{}(TemplateColumnReg{}, m);

    py::class_<CategoricalColumn, Column>(m, "CategoricalColumn")
        .def(py::init<std::string_view>())
        .def_property_readonly("categories", &CategoricalColumn::getCategories,
                               py::return_value_policy::copy)
        .def("add", &CategoricalColumn::add)
        .def("append", [](CategoricalColumn& c, CategoricalColumn& src) { c.append(src); })
        .def("append",
             py::overload_cast<const std::vector<std::string>&>(&CategoricalColumn::append))
        .def("set", py::overload_cast<size_t, std::uint32_t>(&CategoricalColumn::set))
        .def("set", py::overload_cast<size_t, std::string_view>(&CategoricalColumn::set))
        .def("get", &CategoricalColumn::get)
        .def("getId", &CategoricalColumn::getId)
        .def("__repr__",
             [](CategoricalColumn& c) {
                 return fmt::format("<CategoricalColumn: '{}', {}, {} categories>", c.getHeader(),
                                    c.getSize(), c.getCategories().size());
             })
        .def(
            "__iter__",
            [](const CategoricalColumn& c) { return py::make_iterator(c.begin(), c.end()); },
            py::keep_alive<0, 1>());

    py::class_<IndexColumn, TemplateColumn<std::uint32_t>>(m, "IndexColumn")
        .def(py::init<std::string_view>());

    py::class_<DataFrame> dataframe(m, "DataFrame");
    dataframe.def(py::init<std::uint32_t>(), py::arg("size") = 0)
        .def_property_readonly("cols", &DataFrame::getNumberOfColumns)
        .def_property_readonly("rows", &DataFrame::getNumberOfRows)
        .def("indexcol", [](DataFrame& d) { return d.getIndexColumn(); })
        .def("column", [](DataFrame& self, size_t index) { return self.getColumn(index); })
        .def("addColumn",
             static_cast<std::shared_ptr<Column> (DataFrame::*)(std::shared_ptr<Column>)>(
                 &DataFrame::addColumn),
             py::arg("column"))
        .def("addColumnFromBuffer", &DataFrame::addColumnFromBuffer, py::arg("identifier"),
             py::arg("buffer"), py::arg("unit") = Unit(), py::arg("range") = std::nullopt)
        .def("addCategoricalColumn",
             py::overload_cast<std::string_view, size_t>(&DataFrame::addCategoricalColumn),
             py::arg("header"), py::arg("size") = 0)
        .def("addCategoricalColumn",
             py::overload_cast<std::string_view, const std::vector<std::string>&>(
                 &DataFrame::addCategoricalColumn),
             py::arg("header"), py::arg("values"))

        .def("updateIndex", [](DataFrame& d) { d.updateIndexBuffer(); })

        // interface for operator[]
        .def(
            "__getitem__",
            [](const DataFrame& d, size_t i) {
                if (i >= d.getNumberOfColumns()) throw py::index_error();
                return *(d.begin() + i);
            },
            py::return_value_policy::reference_internal)
        // sequence protocol operations
        .def(
            "__iter__", [](const DataFrame& d) { return py::make_iterator(d.begin(), d.end()); },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */)

        .def("__repr__", [](const DataFrame& d) {
            std::string str = fmt::format("<DataFrame: {} column(s), {} rows",
                                          d.getNumberOfColumns(), d.getNumberOfRows());
            size_t i = 0;
            for (auto c : d) {
                ++i;
                str += fmt::format("\n   {:>3}: '{}', {}, {}", i, c->getHeader(), c->getSize(),
                                   c->getBuffer()->getDataFormat()->getString());
            }
            return str + ">";
        });

    util::for_each_type<Scalars>{}(DataFrameAddColumnReg{}, dataframe);

    m.def("appendColumns", dataframe::appendColumns, py::arg("left"), py::arg("right"),
          py::arg("ignoreDuplicates") = false, py::arg("fillMissingRows") = false,
          R"delim(
          Create a new DataFrame by appending the columns of DataFrame right to
          DataFrame left
    
          Args:
              ignoreDuplicates: duplicate columns, i.e. same column header, are ignored
                  if true
              fillMissingRows: if true, missing rows in either DataFrame are filled with
                  0 or "undefined" (for categorical columns)
          )delim")
        .def("appendRows", dataframe::appendRows, py::arg("top"), py::arg("bottom"),
             py::arg("matchByName") = false,
             R"delim(
             Create a new DataFrame by appending the rows of DataFrame bottom to DataFrame top
      
             Args:
                 matchByName: if true, column headers are used for matching columns.
                     Otherwise columns are matched by order (default)
             )delim")
        .def("innerJoin",
             py::overload_cast<const DataFrame&, const DataFrame&,
                               const std::pair<std::string, std::string>&>(dataframe::innerJoin),
             py::arg("left"), py::arg("right"), py::arg("keyColumn") = "index",
             R"delim(
             Create a new DataFrame by using an inner join of DataFrame left and DataFrame
             right. That is only rows with matching keys are kept.
                 
             It is assumed that the entries in the key columns are unique. Otherwise results
             are undefined.
             
             Args:
                 keyColumn: header of the column used as key for the join operation
                     (default: index column)
             )delim")
        .def("innerJoin",
             py::overload_cast<const DataFrame&, const DataFrame&,
                               const std::vector<std::pair<std::string, std::string>>&>(
                 dataframe::innerJoin),
             py::arg("left"), py::arg("right"), py::arg("keyColumn"),
             R"delim(
             Create a new DataFrame by using an inner join of DataFrame left and DataFrame
             right. That is only rows with matching all keys are kept.
                 
             It is assumed that the entries in the key columns are unique. Otherwise results
             are undefined.
             
             Args:
                 keyColumn: list of headers of the columns used as key for the join operation
             )delim")
        .def("innerJoin",
             py::overload_cast<const DataFrame&, const DataFrame&,
                               const std::pair<std::string, std::string>&>(dataframe::leftJoin),
             py::arg("left"), py::arg("right"), py::arg("keyColumn") = "index",
             R"delim(
             Create a new DataFrame by using an outer left join of DataFrame left and DataFrame
             right. That is all rows of left are augmented with matching rows from right.
                 
             It is assumed that the entries in the key columns of right are unique. Otherwise
             results are undefined.
             
             Args:
                 keyColumn: header of the column used as key for the join operation
                     (default: index column)
             )delim")
        .def("leftJoin",
             py::overload_cast<const DataFrame&, const DataFrame&,
                               const std::vector<std::pair<std::string, std::string>>&>(
                 dataframe::leftJoin),
             py::arg("left"), py::arg("right"), py::arg("keyColumn"),
             R"delim(
             Create a new DataFrame by using an outer left join of DataFrame left and DataFrame
             right. That is all rows of left are augmented with matching rows from right.
                 
             It is assumed that the entries in the key columns of right are unique. Otherwise
             results are undefined.
             
             Args:
                 keyColumn: list of headers of the columns used as key for the join operation
             )delim");

    exposeStandardDataPorts<DataFrame>(m, "DataFrame");
}

}  // namespace inviwo
