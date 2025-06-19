#include <nanobind/nanobind.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/string.h>

#include "ompl/geometric/planners/rrt/RRTConnect.h"
#include "../../init.hh"

namespace nb = nanobind;
using namespace ompl::geometric;
using namespace ompl::base;

void ompl::binding::geometric::initPlannersRrt_RRTConnect(nb::module_& m)
{
    nb::class_<RRTConnect, Planner>(m, "RRTConnect")
        .def(nb::init<const SpaceInformationPtr&, bool>(),
             nb::arg("si"),
             nb::arg("addIntermediateStates") = false)
        .def("getIntermediateStates", &RRTConnect::getIntermediateStates)
        .def("setIntermediateStates", &RRTConnect::setIntermediateStates,
             nb::arg("addIntermediateStates"))
        .def("setRange", &RRTConnect::setRange,
             nb::arg("distance"))
        .def("getRange", &RRTConnect::getRange)
        .def("setup", &RRTConnect::setup)
        .def("clear", &RRTConnect::clear)
        .def("solve", &RRTConnect::solve,
             nb::arg("ptc"))
        .def("getPlannerData", &RRTConnect::getPlannerData,
             nb::arg("data"));
}
