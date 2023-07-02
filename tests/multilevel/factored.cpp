#define BOOST_TEST_MODULE "FactoredMotionPlanning"
#include <boost/test/unit_test.hpp>

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

BOOST_AUTO_TEST_CASE(FactoredSpaceInformation_As_SpaceInformation)
{
    ompl::base::StateSpacePtr total_space = CreateCubeStateSpace(4);
    FactoredSpaceInformationPtr si = std::make_shared<FactoredSpaceInformation>(total_space);

    ProblemDefinitionPtr pdef = std::make_shared<ProblemDefinition>(si);
    ScopedState<> start = CreateState(total_space, 0.0f);
    ScopedState<> goal = CreateState(total_space, +1.0f);
    pdef->setStartAndGoalStates(start, goal);

    auto planner = std::make_shared<ompl::geometric::RRTConnect>(si);
    planner->setProblemDefinition(pdef);
    planner->setup();

    ompl::base::IterationTerminationCondition itc(kDefaultNumberIterations);
    auto ptc = ompl::base::plannerOrTerminationCondition(itc, exactSolnPlannerTerminationCondition(pdef));

    PlannerStatus solved = planner->solve(ptc);

    BOOST_CHECK_EQUAL(solved, ompl::base::PlannerStatus::StatusType::EXACT_SOLUTION);
}

BOOST_AUTO_TEST_CASE(FactoredSpaceInformation_SerialConnection)
{
    ompl::base::StateSpacePtr total_space = CreateCubeStateSpace(4);
    FactoredSpaceInformationPtr si = std::make_shared<FactoredSpaceInformation>(total_space);

    ompl::base::StateSpacePtr base_space = CreateCubeStateSpace(4);
    FactoredSpaceInformationPtr base_si = std::make_shared<FactoredSpaceInformation>(base_space);

    ompl::multilevel::ProjectionPtr projAB = std::make_shared<Projection_RN_RM>(total_space, base_space);
    BOOST_CHECK(si->addChild(base_si, projAB));

    ProblemDefinitionPtr pdef = std::make_shared<ProblemDefinition>(si);
    ScopedState<> start = CreateState(total_space, 0.0f);
    ScopedState<> goal = CreateState(total_space, +1.0f);
    pdef->setStartAndGoalStates(start, goal);

    auto planner = std::make_shared<ompl::multilevel::QRRT>(si);
    planner->setProblemDefinition(pdef);
    planner->setup();

    si->printSettings(std::cout);

    ompl::base::IterationTerminationCondition itc(kDefaultNumberIterations);
    auto ptc = ompl::base::plannerOrTerminationCondition(itc, exactSolnPlannerTerminationCondition(pdef));

    PlannerStatus solved = planner->solve(ptc);

    BOOST_CHECK_EQUAL(solved, ompl::base::PlannerStatus::StatusType::EXACT_SOLUTION);
}

BOOST_AUTO_TEST_CASE(FactoredSpaceInformation_DuplicateFactors) 
{
    ompl::base::StateSpacePtr space_A = CreateCubeStateSpace(6);
    space_A->setName("SpaceA");
    FactoredSpaceInformationPtr A = std::make_shared<FactoredSpaceInformation>(space_A);

    ompl::base::StateSpacePtr space_B = CreateCubeStateSpace(4);
    space_B->setName("SpaceB");
    FactoredSpaceInformationPtr B = std::make_shared<FactoredSpaceInformation>(space_B);

    A->setup();
    auto projAB = std::make_shared<Projection_RN_RM>(space_A, space_B);

    BOOST_CHECK(A->addChild(B, projAB));
    BOOST_CHECK(!A->addChild(B, projAB));
    BOOST_CHECK(!A->addChild(A, projAB));
}

BOOST_AUTO_TEST_CASE(FactoredSpaceInformation_MultiLevelConnection)
{
    //    A(6)
    //  / 
    // B(5) 
    // |
    // C(4)
    // |
    // D(3)
    // |
    // E(2)
    // |
    // F(1)

    ompl::base::StateSpacePtr space_A = CreateCubeStateSpace(6);
    space_A->setName("SpaceA");
    auto A = std::make_shared<FactoredSpaceInformation>(space_A);

    ompl::base::StateSpacePtr space_B = CreateCubeStateSpace(5);
    space_B->setName("SpaceB");
    auto B = std::make_shared<FactoredSpaceInformation>(space_B);

    ompl::base::StateSpacePtr space_C = CreateCubeStateSpace(4);
    space_C->setName("SpaceC");
    auto C = std::make_shared<FactoredSpaceInformation>(space_C);

    ompl::base::StateSpacePtr space_D = CreateCubeStateSpace(3);
    space_D->setName("SpaceD");
    auto D = std::make_shared<FactoredSpaceInformation>(space_D);

    ompl::base::StateSpacePtr space_E = CreateCubeStateSpace(2);
    space_E->setName("SpaceE");
    auto E = std::make_shared<FactoredSpaceInformation>(space_E);

    ompl::base::StateSpacePtr space_F = CreateCubeStateSpace(1);
    space_F->setName("SpaceF");
    auto F = std::make_shared<FactoredSpaceInformation>(space_F);

    ompl::multilevel::ProjectionPtr projAB = std::make_shared<Projection_RN_RM>(space_A, space_B);
    ompl::multilevel::ProjectionPtr projBC = std::make_shared<Projection_RN_RM>(space_B, space_C);
    ompl::multilevel::ProjectionPtr projCD = std::make_shared<Projection_RN_RM>(space_C, space_D);
    ompl::multilevel::ProjectionPtr projDE = std::make_shared<Projection_RN_RM>(space_D, space_E);
    ompl::multilevel::ProjectionPtr projEF = std::make_shared<Projection_RN_RM>(space_E, space_F);

    A->addChild(B, projAB);
    B->addChild(C, projBC);
    C->addChild(D, projCD);
    D->addChild(E, projDE);
    E->addChild(F, projEF);

    ProblemDefinitionPtr pdef = std::make_shared<ProblemDefinition>(A);
    ScopedState<> start = CreateState(space_A, 0.01, 0.01);
    ScopedState<> goal = CreateState(space_A, 0.91f, 0.01);
    pdef->setStartAndGoalStates(start, goal);

    auto planner = std::make_shared<ompl::multilevel::FactorRRT>(A);
    planner->setProblemDefinition(pdef);
    planner->setup();

    A->printSettings(std::cout);

    ompl::base::IterationTerminationCondition itc(kDefaultNumberIterations);
    auto ptc = ompl::base::plannerOrTerminationCondition(itc, exactSolnPlannerTerminationCondition(pdef));

    PlannerStatus solved = planner->solve(ptc);

    BOOST_CHECK_EQUAL(solved, ompl::base::PlannerStatus::StatusType::EXACT_SOLUTION);

    BOOST_CHECK(pdef->hasSolution());

    const auto& path = pdef->getSolutionPath();

    BOOST_CHECK(path->check());
    BOOST_CHECK_GT(path->length(), 0.0f);

    path->print(std::cout);
}

BOOST_AUTO_TEST_CASE(FactoredSpaceInformation_DecompositionConnection)
{
    //      A(8)
    //      /\\
    //     /| \ \
    //    / |  \  \
    //   /  |   \   \
    //  /   |    \    \
    // B(2) C(2) D(2) E(2)

    ompl::base::StateSpacePtr space_A = CreateCubeStateSpace(8);
    space_A->setName("SpaceA");
    auto A = std::make_shared<FactoredSpaceInformation>(space_A);

    ompl::base::StateSpacePtr space_B = CreateCubeStateSpace(2);
    space_B->setName("SpaceB");
    auto B = std::make_shared<FactoredSpaceInformation>(space_B);

    ompl::base::StateSpacePtr space_C = CreateCubeStateSpace(2);
    space_C->setName("SpaceC");
    auto C = std::make_shared<FactoredSpaceInformation>(space_C);

    ompl::base::StateSpacePtr space_D = CreateCubeStateSpace(2);
    space_D->setName("SpaceD");
    auto D = std::make_shared<FactoredSpaceInformation>(space_D);

    ompl::base::StateSpacePtr space_E = CreateCubeStateSpace(2);
    space_E->setName("SpaceE");
    auto E = std::make_shared<FactoredSpaceInformation>(space_E);

    ompl::multilevel::ProjectionPtr projAB = std::make_shared<Projection_RN_RM>(space_A, space_B, std::vector<size_t>({0,1}));
    ompl::multilevel::ProjectionPtr projAC = std::make_shared<Projection_RN_RM>(space_A, space_C, std::vector<size_t>({2,3}));
    ompl::multilevel::ProjectionPtr projAD = std::make_shared<Projection_RN_RM>(space_A, space_D, std::vector<size_t>({4,5}));
    ompl::multilevel::ProjectionPtr projAE = std::make_shared<Projection_RN_RM>(space_A, space_E, std::vector<size_t>({6,7}));

    A->addChild(B, projAB);
    A->addChild(C, projAC);
    A->addChild(D, projAD);
    A->addChild(E, projAE);

    ProblemDefinitionPtr pdef = std::make_shared<ProblemDefinition>(A);
    ScopedState<> start = CreateState(space_A, 0.01, 0.01);
    ScopedState<> goal = CreateState(space_A, 0.91f, 0.01);
    pdef->setStartAndGoalStates(start, goal);

    auto planner = std::make_shared<ompl::multilevel::FactorRRT>(A);
    planner->setSeed(0);
    planner->setProblemDefinition(pdef);
    planner->setup();

    ompl::base::IterationTerminationCondition itc(kDefaultNumberIterations);
    auto ptc = ompl::base::plannerOrTerminationCondition(itc, exactSolnPlannerTerminationCondition(pdef));

    A->printSettings(std::cout);

    PlannerStatus solved = planner->solve(ptc);

    BOOST_CHECK_EQUAL(solved, ompl::base::PlannerStatus::StatusType::EXACT_SOLUTION);

    BOOST_CHECK(pdef->hasSolution());

    const auto& path = pdef->getSolutionPath();

    BOOST_CHECK(path->check());
    BOOST_CHECK_GT(path->length(), 0.0f);

    path->print(std::cout);
}

BOOST_AUTO_TEST_CASE(FactoredSpaceInformation_FactorTree)
{
    //    A(6)
    //  /     \
    // B(4)   C(2)
    // |
    // D(2)
    ompl::base::StateSpacePtr space_A = CreateCubeStateSpace(6);
    space_A->setName("SpaceA");
    auto A = std::make_shared<FactoredSpaceInformation>(space_A);

    ompl::base::StateSpacePtr space_B = CreateCubeStateSpace(4);
    space_B->setName("SpaceB");
    auto B = std::make_shared<FactoredSpaceInformation>(space_B);

    ompl::base::StateSpacePtr space_C = CreateCubeStateSpace(2);
    space_C->setName("SpaceC");
    auto C = std::make_shared<FactoredSpaceInformation>(space_C);

    ompl::base::StateSpacePtr space_D = CreateCubeStateSpace(2);
    space_D->setName("SpaceD");
    auto D = std::make_shared<FactoredSpaceInformation>(space_D);

    ompl::multilevel::ProjectionPtr projAB = std::make_shared<Projection_RN_RM>(space_A, space_B);
    ompl::multilevel::ProjectionPtr projBD = std::make_shared<Projection_RN_RM>(space_B, space_D);
    ompl::multilevel::ProjectionPtr projAC = std::make_shared<Projection_RN_RM>(space_A, space_C, std::vector<size_t>({4,5}));

    A->addChild(B, projAB);
    A->addChild(C, projAC);
    B->addChild(D, projBD);

    ProblemDefinitionPtr pdef = std::make_shared<ProblemDefinition>(A);
    ScopedState<> start = CreateState(space_A, 0.01, 0.01);
    ScopedState<> goal = CreateState(space_A, 0.91f, 0.01);
    pdef->setStartAndGoalStates(start, goal);

    auto planner = std::make_shared<ompl::multilevel::FactorRRT>(A);
    planner->setSeed(0);
    planner->setProblemDefinition(pdef);
    planner->setup();

    A->printSettings(std::cout);
    B->printSettings(std::cout);

    ompl::base::IterationTerminationCondition itc(kDefaultNumberIterations);
    auto ptc = ompl::base::plannerOrTerminationCondition(itc, exactSolnPlannerTerminationCondition(pdef));

    PlannerStatus solved = planner->solve(ptc);

    BOOST_CHECK_EQUAL(solved, ompl::base::PlannerStatus::StatusType::EXACT_SOLUTION);

    BOOST_CHECK(pdef->hasSolution());

    //Check subproblem solutions
    auto all_status = planner->getPlannerStatus();
    BOOST_CHECK_EQUAL(all_status.size(), 4u);

    for(const auto& name_and_status : all_status) {
      const auto& factor_status = name_and_status.second;
      BOOST_CHECK_EQUAL(factor_status, ompl::base::PlannerStatus::StatusType::EXACT_SOLUTION);
    }

    auto all_pdefs = planner->getProblemDefinitions();
    BOOST_CHECK_EQUAL(all_pdefs.size(), 4u);

    for(const auto& name_and_pdef : all_pdefs) {
      const auto& factor_pdef = name_and_pdef.second;
      BOOST_CHECK(factor_pdef->hasSolution());

      const auto& factor_path = factor_pdef->getSolutionPath();

      BOOST_CHECK(factor_path->check());
      BOOST_CHECK_GT(factor_path->length(), 0.0f);

      OMPL_INFORM("Planner for factor %s", name_and_pdef.first.c_str());
      factor_path->print(std::cout);
    }
}

