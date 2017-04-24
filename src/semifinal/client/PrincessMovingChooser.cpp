#include "PrincessMovingChooser.hpp"
#include "ChooserHelper.hpp"

Step PrincessMovingChooser::chooseGoodStep(
        const std::vector<PotentialStep>& potentialSteps) {
    if (overrideGoodSteps) {
        auto steps = potentialSteps;
        for (PotentialStep& step : steps) {
            step.weight += calculateWeight(step);
        }
        return DelegatingChooser::chooseGoodStep(steps);
    } else {
        return DelegatingChooser::chooseGoodStep(potentialSteps);
    }
}

Step PrincessMovingChooser::chooseBadStep(
        const std::vector<PotentialStep>& potentialSteps) {
    return processPotentialSteps(potentialSteps,
            [this](auto& stepValues, const auto& step) {
                return processStep(stepValues, step, true);
            }, getDelegatedChooser(), "PrincessMovingChooser");
}

void PrincessMovingChooser::processStep(std::vector<PotentialStep>& stepValues,
        PotentialStep step, bool isBadStep) {
    const Track& track = *step.targetTrack;
    int playerId = step.sourceState->gameInfo.playerId;

    double baseWeight = step.weight;
    for (Point p : track.getReachablePoints(track.getPrincess(playerId))) {
        step.step.princessTarget = p;
        double weight = calculateWeight(step);
        if (isBadStep) {
            weight *= 5;
        }
        step.weight = baseWeight + weight;
        stepValues.push_back(step);
    }
}

double PrincessMovingChooser::calculateWeight(const PotentialStep& step) {
    const Track& track = *step.targetTrack;
    Point target = track.getMonitor(step.sourceState->targetMonitor);

    int size = track.width() + track.height();
    double d = std::abs(step.step.princessTarget.x - target.x)
                + std::abs(step.step.princessTarget.y - target.y);
    // std::cerr << "|" << step.step.princessTarget << " - " << target << "| = "
    //         << d << "\n";
    double result = (1.0 - d / static_cast<double>(size)) * weightMultiplier;
    // std::cerr << step.step << ": (1 - " << d << " / " << size << ") * "
    //         << weightMultiplier << " = " << result << "\n";
    return result;

}
