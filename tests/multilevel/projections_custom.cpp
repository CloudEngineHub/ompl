/* Author: Andreas Orthey */
#define BOOST_TEST_MODULE "Projections"
#include <boost/test/unit_test.hpp>
#include "boost/program_options.hpp"

#include <fstream>

#include <ompl/geometric/planners/rrt/RRTConnect.h>
#include <ompl/geometric/planners/rrt/RRT.h>
#include <ompl/base/spaces/SE2StateSpace.h>
#include <ompl/base/spaces/RealVectorStateSpace.h>

#include <ompl/multilevel/planners/qrrt/QRRT.h>
#include <ompl/multilevel/planners/qmp/QMP.h>
#include <ompl/multilevel/datastructures/Projection.h>
#include <ompl/multilevel/datastructures/projections/RN_RM.h>

using namespace ompl::geometric;
using namespace ompl::base;
using namespace ompl::multilevel;

namespace ompl
{
    namespace multilevel
    {
        OMPL_CLASS_FORWARD(Projection);
    }
}  // namespace ompl

bool boxConstraint(const double values[])
{
    const double &x = values[0];
    const double &y = values[1];
    double pos_cnstr = sqrt(x * x + y * y);
    return (pos_cnstr > 0.5);
}
bool isStateValid_R4(const State *state)
{
    const auto *R4 = state->as<RealVectorStateSpace::StateType>();
    return boxConstraint(R4->values);
}
bool isStateValid_R2(const State *state)
{
    const auto *R2 = state->as<RealVectorStateSpace::StateType>();
    return boxConstraint(R2->values);
}

const double kTimeout = 2;

BOOST_AUTO_TEST_CASE(Projections_Custom)
{
    // #########################################################################
    // ## Create total space
    // #########################################################################
    ompl::base::StateSpacePtr total_space(new RealVectorStateSpace(4));
    ompl::base::RealVectorBounds bounds_total_space(4);
    bounds_total_space.setLow(-1);
    bounds_total_space.setHigh(+1);
    total_space->as<RealVectorStateSpace>()->setBounds(bounds_total_space);

    SpaceInformationPtr total_space_si = std::make_shared<SpaceInformation>(total_space);

    total_space_si->setStateValidityChecker(isStateValid_R4);
    total_space_si->printSettings();

    // #########################################################################
    // ## Create base space
    // #########################################################################
    ompl::base::StateSpacePtr base_space(new RealVectorStateSpace(2));
    ompl::base::RealVectorBounds bounds_base_space(2);
    bounds_base_space.setLow(-1);
    bounds_base_space.setHigh(+1);
    base_space->as<RealVectorStateSpace>()->setBounds(bounds_base_space);
    SpaceInformationPtr base_space_si = std::make_shared<SpaceInformation>(base_space);

    base_space_si->setStateValidityChecker(isStateValid_R2);
    base_space_si->printSettings();

    // #########################################################################
    // ## Create fibered projection
    // #########################################################################
    ompl::multilevel::ProjectionPtr projAB = std::make_shared<Projection_RN_RM>(total_space, base_space);

    // #########################################################################
    // ## Put it all together
    // #########################################################################
    std::vector<SpaceInformationPtr> siVec;
    std::vector<ompl::multilevel::ProjectionPtr> projVec;

    siVec.push_back(base_space_si);
    projVec.push_back(projAB);
    siVec.push_back(total_space_si);

    auto planner = std::make_shared<ompl::multilevel::QRRT>(siVec, projVec);

    // #########################################################################
    // ## Set start state
    // #########################################################################

    ScopedState<> start(total_space);
    start[0] = -1;
    start[1] = -1;
    start[2] = -1;
    start[3] = -1;
    ScopedState<> goal(total_space);
    goal[0] = +1;
    goal[1] = +1;
    goal[2] = +1;
    goal[3] = +1;

    ProblemDefinitionPtr pdef = std::make_shared<ProblemDefinition>(total_space_si);
    pdef->addStartState(start);
    pdef->setGoalState(goal, 1e-3);

    // #########################################################################
    // ## Invoke planner
    // #########################################################################
    planner->setProblemDefinition(pdef);
    planner->setup();
    PlannerStatus status = planner->Planner::solve(kTimeout);

    if (status == ompl::base::PlannerStatus::EXACT_SOLUTION ||
        status == ompl::base::PlannerStatus::APPROXIMATE_SOLUTION)
    {
        PathPtr path = pdef->getSolutionPath();
        PathGeometric &pgeo = *static_cast<PathGeometric *>(path.get());
        OMPL_INFORM("Solution path has %d states.", pgeo.getStateCount());
        pgeo.print(std::cout);
    }

    BOOST_CHECK_EQUAL(status, ompl::base::PlannerStatus::StatusType::EXACT_SOLUTION);
    BOOST_CHECK_GT(pdef->getSolutionPath()->length(), 0u);
    BOOST_CHECK_EQUAL(pdef->getSolutionPath()->check(), true);
}
