#include "ompl/multilevel/planners/factor/FactoredPlanner.h"
#include "ompl/multilevel/planners/factor/RestrictionSampler.h"
#include "ompl/multilevel/datastructures/FactoredSpaceInformation.h"

using namespace ompl::multilevel;

FactoredPlanner::FactoredPlanner(const FactoredSpaceInformationPtr& si, const std::vector<FactoredPlannerPtr>& children_planner) 
  : RRTConnect(si)
{
  setName("PlannerOn" + si->getName());
  if(!children_planner.empty()) 
  {
    sampler_ = std::make_shared<RestrictionSampler>(si, children_planner);
  }
}

ompl::base::PlannerStatus FactoredPlanner::solve(const ompl::base::PlannerTerminationCondition &ptc) 
{
  return RRTConnect::solve(ptc);
}

void FactoredPlanner::setSeed(size_t seed) 
{
  ompl::RNG::setSeed(seed);
  rng_ = ompl::RNG(seed);
}

void FactoredPlanner::sampleFromPath(const std::vector<base::State *>& path_states, ompl::base::State* state) 
{
  std::vector<double> distances;
  for (unsigned int i = 1; i < path_states.size(); i++)
  {
      const double d = si_->distance(path_states.at(i - 1), path_states.at(i));
      distances.push_back(d);
  }
  const double path_length = std::accumulate(distances.begin(), distances.end(), 0.0f);
  const double random_position_on_path = rng_.uniformReal(0, path_length);

  double current_distance = 0.0;
  for (unsigned int i = 0; i < distances.size(); i++)
  {
      const double& d = distances.at(i);
      current_distance += d;
      if(current_distance > random_position_on_path) 
      {
        //r lies between path_states i+1 and i
        const auto s1 = path_states.at(i);
        const auto s2 = path_states.at(i + 1);

        //   |--------------| d
        //             |----| current_distance-random_position_on_path
        //   |---------|      d - (current_distance-random_position_on_path)
        //---|---------x----|------
        //   s1        r    s2
        if(d > 1e-3) 
        {
          const double s = (d - (current_distance -random_position_on_path))/d; //in [0,1]
          si_->getStateSpace()->interpolate(s1, s2, s, state);
        }else {
          si_->copyState(state, s1);
        }
        return;
      }
      distances.push_back(d);
  }
  OMPL_ERROR("Path sampler reached end of method with lenght %f and random value %f", path_length, random_position_on_path);
}

void FactoredPlanner::sampleFromDatastructure(ompl::base::State* state) 
{
    const auto& pdef = getProblemDefinition();
    if(!pdef->hasSolution()) 
    {
      OMPL_ERROR("Cannot sample from space without a solution.");
      return;
    }

    //Path restriction sampling
    // const auto path = pdef->getSolutionPath()->as<geometric::PathGeometric>();
    // const std::vector<base::State *>& path_states = path->getStates();
    // sampleFromPath(path_states, state);

    //Tree restriction sampling (vertex version)
    const size_t N = tStart_->size() + tGoal_->size();
    const size_t R = rng_.uniformInt(0, N-1);

    std::vector<Motion*> data;
    if(R < tStart_->size()) 
    {
      //sample from start tree
      tStart_->list(data);
      si_->copyState(state, data.at(R)->state);
    } else {
      //sample from goal tree
      tGoal_->list(data);
      si_->copyState(state, data.at(R - tStart_->size())->state);
    }
}
