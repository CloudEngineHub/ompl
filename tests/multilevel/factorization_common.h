#ifndef TESTS_MULTILEVEL_FACTORIZATION_COMMON_
#define TESTS_MULTILEVEL_FACTORIZATION_COMMON_
#include <ompl/base/StateSpace.h>
#include <ompl/base/spaces/RealVectorStateSpace.h>
#include <ompl/base/terminationconditions/IterationTerminationCondition.h>

#include <ompl/geometric/planners/rrt/RRTConnect.h>
#include <ompl/geometric/planners/rrt/RRT.h>
#include <ompl/multilevel/planners/qrrt/QRRT.h>
#include <ompl/multilevel/planners/qmp/QMP.h>
#include <ompl/multilevel/planners/factor/FactorRRT.h>
#include <ompl/multilevel/datastructures/FactoredSpaceInformation.h>
#include <ompl/multilevel/datastructures/projections/RN_RM.h>
#include <ompl/util/Console.h>

using namespace ompl::base;
using namespace ompl::multilevel;

const unsigned int kDefaultNumberIterations = 500;

ompl::base::StateSpacePtr CreateCubeStateSpace(size_t dim) {
  ompl::base::StateSpacePtr space(new RealVectorStateSpace(dim));
  ompl::base::RealVectorBounds bounds_space(dim);
  bounds_space.setLow(0);
  bounds_space.setHigh(+1);
  space->as<RealVectorStateSpace>()->setBounds(bounds_space);
  return space;
}

ScopedState<> CreateState(const ompl::base::StateSpacePtr& space, const float value, const float step_size = 0.0f) {
  ScopedState<> state(space);
  for(size_t k = 0; k < space->getDimension(); k++) {
    state[k] = value + k * step_size;
  }
  return state;
}

#endif // TESTS_MULTILEVEL_FACTORIZATION_COMMON_
