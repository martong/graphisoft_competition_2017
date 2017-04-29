#include "MaxReachableChooser.hpp"
#include "ChooserHelper.hpp"

#include <unordered_set>

Step MaxReachableChooser::chooseBadStep(
    const std::vector<PotentialStep>& potentialSteps) {

    auto changedPotentialSteps = potentialSteps;

    savedWeights.clear();
    for (PotentialStep& step : changedPotentialSteps) {
        processStep(step, true);
    }
    return getDelegatedChooser().chooseBadStep(changedPotentialSteps);
}

Step MaxReachableChooser::chooseGoodStep(
    const std::vector<PotentialStep>& potentialSteps) {

    auto changedPotentialSteps = potentialSteps;

    savedWeights.clear();
    for (PotentialStep& step : changedPotentialSteps) {
        processStep(step);
    }

    return getDelegatedChooser().chooseGoodStep(changedPotentialSteps);
}

void MaxReachableChooser::processStep(PotentialStep& step,
        bool isBadStep) {
    auto key = std::make_tuple(step.step.pushDirection, step.step.pushPosition,
            step.step.pushFieldType);
    auto iterator = savedWeights.find(key);
    if (iterator != savedWeights.end()) {
        // std::cerr << "Saved weight: " << iterator->second << "\n";
        step.weight += iterator->second;
        step.debugInfo.push_back(PotentialStep::DebugInfo{"MaxReachableChooser", 
                iterator->second / weightMultiplier, iterator->second});
        return;
    }

    GameState newGameState{*step.sourceState, *step.targetTrack};
    newGameState.extraField = step.targetExtraField;
    const auto& gi = newGameState.gameInfo;

    // std::cerr << "Step " << step.step << "\n";

#define NO_LOOKAHEAD 1
#if NO_LOOKAHEAD
    const auto& reachablePoints = step.targetTrack->getReachablePoints(
            step.targetTrack->getPrincess(gi.playerId));
    double w = static_cast<double>(reachablePoints.size())
            / (gi.width * gi.height);
#else
    const auto& opponentsInfo = *step.opponentsInfo;
    double weight = 0;
    const auto& opponentsInfo = *step.opponentsInfo;
    int opponentExtraField = opponentsInfo[gi.playerId].extraField;
    auto nextSteps = calculatePotentialSteps(
            newGameState, opponentsInfo, gi.playerId, opponentExtraField);
    for (const PotentialStep& nextStep : nextSteps) {
        const auto& reachablePoints =
                nextStep.targetTrack->getReachablePoints(
                        nextStep.targetTrack->getPrincess(gi.playerId));

        double w = static_cast<double>(reachablePoints.size())
                / (gi.width * gi.height);
        // std::cerr << "  Our reachable points = "
        //         << reachablePoints.size() << " w=" << w << "\n";
        weight += w;
    }
    double w = weight / nextSteps.size();
#endif
    // std::cerr << "Total weight = " << w << "\n";
    double ww = w * weightMultiplier;
    if (isBadStep) {
        ww *= 2;
    }
    step.weight += ww;
    step.debugInfo.push_back(PotentialStep::DebugInfo{"MaxReachableChooser", w, ww});
    // std::cerr << "Total weight = " << w << "\n";
    savedWeights.emplace(key, ww);
}
