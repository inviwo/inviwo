/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2021 Inviwo Foundation
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
#include <inviwo/dataframe/datastructures/datapoint.h>
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
    auto operator()(py::class_<DataFrame, std::shared_ptr<DataFrame>>& d) {
        auto classname = Defaultvalues<T>::getName();

        d.def(fmt::format("add{}Column", classname).c_str(),
              [](DataFrame& d, const std::string& header, const size_t size = 0) {
                  return d.addColumn<T>(header, size);
              },
              py::arg("header"), py::arg("size") = 0);

        d.def(fmt::format("add{}Column", classname).c_str(),
              [](DataFrame& d, std::string header, std::vector<T> data) {
                  return d.addColumn(std::move(header), std::move(data));
              },
              py::arg("header"), py::arg("data"));
    }
};

struct DataPointReg {
    template <typename T>
    auto operator()(pybind11::module& m) {
        using D = DataPoint<T>;
        auto classname = Defaultvalues<T>::getName() + "DataPoint";

        py::class_<D, DataPointBase, std::shared_ptr<D>> data(m, classname.c_str());
        data.def_property_readonly("data", &D::getData)
            .def_property_readonly("str", &D::toString)
            .def("__repr__",
                 [classname](D& p) { return fmt::format("<{}: '{}'>", classname, p.toString()); });
    }
};

struct TemplateColumnReg {
    template <typename T>
    auto operator()(pybind11::module& m) {
        using C = TemplateColumn<T>;
        auto classname = Defaultvalues<T>::getName() + "Column";

        py::class_<C, Column, std::shared_ptr<C>> col(m, classname.c_str());
        col.def_property_readonly("buffer", [](C& c) { return c.getTypedBuffer(); })
            .def(py::init<const std::string&>())
            .def("add", py::overload_cast<const T&>(&C::add))
            .def("add", py::overload_cast<const std::string&>(&C::add))
            .def("append", [](C& c, C& src) { c.append(src); })
            .def("set", &C::set)
            .def("get",
                 [](const C& c, size_t i) {
                     if (i >= c.getSize()) throw py::index_error();
                     return c.get(i);
                 },
                 py::arg("i"))
            .def("get",
                 [](const C& c, size_t i, bool asString) {
                     if (i >= c.getSize()) throw py::index_error();
                     return c.get(i, asString);
                 },
                 py::arg("i"), py::arg("asString"))
            .def("__repr__", [classname](C& c) {
                return fmt::format("<{}: '{}', {}, {}>", classname, c.getHeader(), c.getSize(),
                                   c.getBuffer()->getDataFormat()->getString());
            });
    }
};

}  // namespace

void exposeDataFrame(pybind11::module& m) {
    py::class_<DataPointBase, std::shared_ptr<DataPointBase>>(m, "DataPointBase")
        .def("__repr__",
             [](DataPointBase& p) { return fmt::format("<DataPoint: '{}'>", p.toString()); });

    py::class_<Column, std::shared_ptr<Column>>(m, "Column")
        .def_property("header", &Column::getHeader, &Column::setHeader)
        .def_property_readonly("buffer", [](Column& self) { return self.getBuffer(); })
        .def_property_readonly("size", &Column::getSize)
        .def("__repr__", [](Column& c) {
            return fmt::format("<Column: '{}', {}, {}>", c.getHeader(), c.getSize(),
                               c.getBuffer()->getDataFormat()->getString());
        });

    using Scalars = std::tuple<float, double, int, glm::i64, size_t, std::uint32_t>;
    util::for_each_type<Scalars>{}(DataPointReg{}, m);
    util::for_each_type<Scalars>{}(TemplateColumnReg{}, m);

    py::class_<CategoricalColumn, TemplateColumn<std::uint32_t>,
               std::shared_ptr<CategoricalColumn>>(m, "CategoricalColumn")
        .def(py::init<const std::string&>())
        .def_property_readonly("categories", &CategoricalColumn::getCategories,
                               py::return_value_policy::copy)
        .def("add", [](CategoricalColumn& c, const std::string& str) { c.add(str); })
        .def("append", [](CategoricalColumn& c, CategoricalColumn& src) { c.append(src); })
        .def("set", [](CategoricalColumn& c, size_t idx, const std::uint32_t& v) { c.set(idx, v); })
        .def("set", py::overload_cast<size_t, const std::string&>(&CategoricalColumn::set))
        .def("get",
             [](const CategoricalColumn& c, size_t i, bool asString) {
                 if (i >= c.getSize()) throw py::index_error();
                 return c.get(i, asString);
             },
             py::arg("i"), py::arg("asString") = true)
        .def("__repr__", [](CategoricalColumn& c) {
            return fmt::format("<CategoricalColumn: '{}', {}, {} categories>", c.getHeader(),
                               c.getSize(), c.getCategories().size());
        });

    py::class_<DataFrame, std::shared_ptr<DataFrame>> dataframe(m, "DataFrame");
    dataframe.def(py::init<std::uint32_t>(), py::arg("size") = 0)
        .def_property_readonly("cols", &DataFrame::getNumberOfColumns)
        .def_property_readonly("rows", &DataFrame::getNumberOfRows)
        .def("indexcol", [](DataFrame& d) { return d.getIndexColumn(); })
        .def("column", [](DataFrame& self, size_t index) { return self.getColumn(index); })
        .def("addColumnFromBuffer", &DataFrame::addColumnFromBuffer)
        .def("addCategoricalColumn",
             py::overload_cast<const std::string&, size_t>(&DataFrame::addCategoricalColumn),
             py::arg("header"), py::arg("size") = 0)
        .def("addCategoricalColumn",
             py::overload_cast<const std::string&, const std::vector<std::string>&>(
                 &DataFrame::addCategoricalColumn),
             py::arg("header"), py::arg("values"))
        .def("getRow", &DataFrame::getDataItem, py::arg("index"), py::arg("asString") = false)

        .def("updateIndex", [](DataFrame& d) { d.updateIndexBuffer(); })

        // interface for operator[]
        .def("__getitem__",
             [](const DataFrame& d, size_t i) {
                 if (i >= d.getNumberOfColumns()) throw py::index_error();
                 return *(d.begin() + i);
             },
             py::return_value_policy::reference_internal)
        // sequence protocol operations
        .def("__iter__", [](const DataFrame& d) { return py::make_iterator(d.begin(), d.end()); },
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

    m.def("createDataFrame", createDataFrame, py::arg("exampleRows"),
          py::arg("colheaders") = std::vector<std::string>{}, py::arg("doubleprecision") = false,
          R"delim(
Create a new DataFrame by guessing the column types from a number of rows.

Parameters
----------
exampleRows     Rows for guessing data type of each column.
colHeaders      Name of each column. If none are given, "Column 1", "Column 2", ... is used
doubleprecision If true, columns with floating point values will use double for storage
)delim")
        .def("appendColumns", dataframe::appendColumns, py::arg("left"), py::arg("right"),
             py::arg("ignoreduplicates") = false, py::arg("fillmissingrows") = false,
             R"delim(
Create a new DataFrame by appending the columns of DataFrame right to DataFrame left

Parameters
----------
ignoreduplicates   duplicate columns, i.e. same column header, are ignored if true
fillmissingrows    if true, missing rows in either DataFrame are filled with 0 or
                   "undefined" (for categorical columns)
)delim")
        .def("appendRows", dataframe::appendRows, py::arg("top"), py::arg("bottom"),
             py::arg("matchbyname") = false,
             R"delim(
Create a new DataFrame by appending the rows of DataFrame bottom to DataFrame top

Parameters
----------
matchByName    if true, column headers are used for matching columns. Otherwise columns
               are matched by order (default)
)delim")
        .def("innerJoin",
             py::overload_cast<const DataFrame&, const DataFrame&, const std::string&>(
                 dataframe::innerJoin),
             py::arg("left"), py::arg("right"), py::arg("keycolumn") = "index",
             R"delim(
Create a new DataFrame by using an inner join of DataFrame left and DataFrame right.
That is only rows with matching keys are kept.

It is assumed that the entries in the key columns are unique. Otherwise results are undefined.

Parameters
----------
keycolumn    header of the column used as key for the join operation (default: index column)
)delim")
        .def("innerJoin",
             py::overload_cast<const DataFrame&, const DataFrame&, const std::vector<std::string>&>(
                 dataframe::innerJoin),
             py::arg("left"), py::arg("right"), py::arg("keycolumns"),
             R"delim(
Create a new DataFrame by using an inner join of DataFrame left and DataFrame right.
That is only rows with matching all keys are kept.

It is assumed that the entries in the key columns are unique. Otherwise results are undefined.

Parameters
----------
keycolumns    list of headers of the columns used as key for the join operation
)delim")
        .def("innerJoin",
             py::overload_cast<const DataFrame&, const DataFrame&, const std::string&>(
                 dataframe::leftJoin),
             py::arg("left"), py::arg("right"), py::arg("keycolumn") = "index",
             R"delim(
Create a new DataFrame by using an outer left join of DataFrame left and DataFrame right.
That is all rows of left are augmented with matching rows from right.

It is assumed that the entries in the key columns of right are unique. Otherwise results
are undefined.

Parameters
----------
keycolumn    header of the column used as key for the join operation (default: index column)
)delim")
        .def("leftJoin",
             py::overload_cast<const DataFrame&, const DataFrame&, const std::vector<std::string>&>(
                 dataframe::leftJoin),
             py::arg("left"), py::arg("right"), py::arg("keycolumns"),
             R"delim(
Create a new DataFrame by using an outer left join of DataFrame left and DataFrame right.
That is all rows of left are augmented with matching rows from right.

It is assumed that the entries in the key columns of right are unique. Otherwise results
are undefined.

Parameters
----------
keycolumns    list of headers of the columns used as key for the join operation
)delim");

    exposeStandardDataPorts<DataFrame>(m, "DataFrame");
}

}  // namespace inviwo
