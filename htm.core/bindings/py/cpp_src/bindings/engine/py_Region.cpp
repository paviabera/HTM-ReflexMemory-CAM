/* ---------------------------------------------------------------------
 * HTM Community Edition of NuPIC
 * Copyright (C) 2018, Numenta, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Affero Public License for more details.
 *
 * You should have received a copy of the GNU Affero Public License
 * along with this program.  If not, see http://www.gnu.org/licenses.
 *
 * Author: @chhenning, 2018
 * --------------------------------------------------------------------- */

/** @file
PyBind11 bindings for Region classes
*/


#include <bindings/suppress_register.hpp>  //include before pybind11.h
#include <pybind11/pybind11.h>
#include <pybind11/iostream.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include <plugin/PyBindRegion.hpp>
#include <htm/regions/TestNode.hpp>

namespace py = pybind11;
using namespace htm;

namespace htm_ext
{
    void init_Regions(py::module& m)
    {
        ///////////////////
        // PyRegion Node
        ///////////////////
        typedef htm::PyBindRegion Region_t;
        py::class_<Region_t> py_Region(m, "PyRegion");

        py_Region.def("initialize", &Region_t::initialize)
            .def("compute", &Region_t::compute);

		// This would be .py code calling getSpec() to get the Spec structure
		// on an already instantiated class.
        py_Region.def("getSpec", &Region_t::getSpec);
        py_Region.def("executeCommand", &Region_t::executeCommand);

        ///////////////////
        // Test Node
        ///////////////////
        py::class_<TestNode> py_TestNode(m, "TestNode");
        py_TestNode.def("initialize", &TestNode::initialize)
            .def("compute", &TestNode::compute)
            .def("getName", [](const TestNode& self) { return "Hello";  });

    }

} // namespace htm_ext
