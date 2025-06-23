#include <nanobind/nanobind.h>
#include "ompl/geometric/PathSimplifier.h"
#include "init.hh"

namespace nb = nanobind;
namespace ob = ompl::base;
namespace og = ompl::geometric;

void ompl::binding::geometric::init_PathSimplifier(nb::module_& m)
{
    // TODO [og::PathSimplifier][IMPLEMENT]
    nb::class_<og::PathSimplifier>(m, "PathSimplifier")
        ;
}
