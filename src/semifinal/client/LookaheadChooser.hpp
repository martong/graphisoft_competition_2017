#ifndef SEMIFINAL_CLIENT_LOOKAHEADCHOOSER_HPP
#define SEMIFINAL_CLIENT_LOOKAHEADCHOOSER_HPP

#include "GameState.hpp"
#include "PotentialStep.hpp"
#include "Track.hpp"

#include <algorithm>
#include <numeric>
#include <unordered_map>

template<typename Chooser>
class LookaheadChooser {
public:
    LookaheadChooser(Chooser chooser) : chooser(std::move(chooser)) {}

    const Step& chooseGoodStep(
            const std::vector<PotentialStep>& potentialSteps) {
        return chooser.chooseGoodStep(potentialSteps);
    }

    Step chooseBadStep(
            const std::vector<PotentialStep>& potentialSteps) {
        std::vector<StepRecord> stepValues;
        for (const PotentialStep& step : potentialSteps) {
            processStep(stepValues, step);
        }
        if (stepValues.empty()) {
            std::cerr << "No good step is found.\n";
            return chooser.chooseBadStep(potentialSteps);
        }

        std::sort(stepValues.begin(), stepValues.end(),
                [](const StepRecord& lhs, const StepRecord& rhs) {
                    return lhs.value > rhs.value;
                });

        std::vector<PotentialStep> goodSteps;
        for (const StepRecord& record : stepValues) {
            if (record.value != stepValues.front().value) {
                break;
            }
            goodSteps.push_back(record.step);
        }
        std::cerr << "Found " << goodSteps.size()
                << " good steps with value " << stepValues.front().value
                << "\n";
        return chooser.chooseGoodStep(goodSteps);
    }

private:
    struct StepRecord {
        PotentialStep step;
        int value;
    };

    void processStep(std::vector<StepRecord>& stepValues, PotentialStep step) {
        std::unordered_map<Point, int> princessTargets;
        const auto& reachablePoints = step.targetTrack.getReachablePoints(
                step.targetTrack.getPrincess(step.sourceState->playerId));
        for (Point p : reachablePoints) {
            princessTargets.emplace(p, 0);
        }

        GameState newGameState{*step.sourceState, step.targetTrack};
        newGameState.extraField = step.targetExtraField;
        for (const PotentialStep& nextStep :
                calculatePotentialSteps(newGameState)) {
            auto transformedPoints = transformPoints(nextStep.targetTrack,
                    reachablePoints, nextStep.step.pushDirection,
                    nextStep.step.pushPosition);
            TransformedPointCompare comparator{&TransformedPoint::transformed};
            std::sort(transformedPoints.begin(), transformedPoints.end(),
                    comparator);
            for (Point p : nextStep.targetTrack.getReachablePoints(
                    nextStep.targetTrack.getMonitor(
                            step.sourceState->targetMonitor))) {
                auto iterators = std::equal_range(
                        transformedPoints.begin(), transformedPoints.end(), p,
                        comparator);
                if (iterators.first != iterators.second) {
                    ++princessTargets.at(iterators.first->original);
                }
            }
        }

        for (const auto& element : princessTargets) {
            if (element.second == 0) {
                continue;
            }

            step.step.princessTarget = element.first;
            stepValues.push_back(StepRecord{step, element.second});
        }
    }

    Chooser chooser;
};

#endif // SEMIFINAL_CLIENT_LOOKAHEADCHOOSER_HPP