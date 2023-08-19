#ifndef OMPL_MULTILEVEL_PLANNERS_FACTOR_FACTORRRT_
#define OMPL_MULTILEVEL_PLANNERS_FACTOR_FACTORRRT_

#include <ompl/base/Planner.h>
#include <ompl/util/RandomNumbers.h>
#include <unordered_map>
#include <ompl/multilevel/planners/factor/FactoredPlanner.h>

namespace ompl
{
    namespace multilevel
    {
        OMPL_CLASS_FORWARD(FactorRRT);
        OMPL_CLASS_FORWARD(FactoredSpaceInformation);
        OMPL_CLASS_FORWARD(FactoredPlanner);

        class FactorRRT : public base::Planner 
        {
          public:

            using base::Planner::solve;

            FactorRRT(const FactoredSpaceInformationPtr &si);

            ~FactorRRT() override;

            base::PlannerStatus solve(const base::PlannerTerminationCondition &ptc) override;

            void clear() override;
            void setup() override;
            void setSeed(size_t seed);

            void setProblemDefinition(const base::ProblemDefinitionPtr &pdef) override;

            const std::unordered_map<std::string, base::ProblemDefinitionPtr>& getProblemDefinitions() const;
            const std::unordered_map<std::string, base::PlannerStatus>& getPlannerStatus() const;

            const FactoredSpaceInformationPtr& getFactoredSpaceInformation() const;

          protected:
            void grow_(const FactoredSpaceInformationPtr& factor);
            bool hasSolution_(const FactoredSpaceInformationPtr& factor) const;
            const FactoredSpaceInformationPtr& selectFactor_();
            void createPlannerForFactor_(const FactoredSpaceInformationPtr& factor);

            bool isActive_(const FactoredSpaceInformationPtr& factor) const;
            bool isSolved_(const FactoredSpaceInformationPtr& factor) const;
            bool allChildrenHaveSolutions_(const FactoredSpaceInformationPtr& factor) const;

            void createProblemDefinition_(const FactoredSpaceInformationPtr& factor, const base::State* parent_start, const base::State* parent_goal);

            std::vector<FactoredPlannerPtr> getChildrenPlanner_(const FactoredSpaceInformationPtr& factor) const;

          private:
            RNG rng_;

            std::vector<std::pair<FactoredSpaceInformationPtr, base::State*>> start_states_;
            std::vector<std::pair<FactoredSpaceInformationPtr, base::State*>> goal_states_;

            std::optional<size_t> seed_;

            std::vector<FactoredSpaceInformationPtr> active_factors_;
            std::unordered_map<std::string, FactoredPlannerPtr> active_planners_;
            std::unordered_map<std::string, bool> is_active_;
            std::unordered_map<std::string, bool> is_solved_;
            std::unordered_map<std::string, base::ProblemDefinitionPtr> problem_definitions_per_factor_;
            std::unordered_map<std::string, base::PlannerStatus> planner_status_per_factor_;

            base::PlannerStatus planner_status_;
        };

    }
}

#endif
