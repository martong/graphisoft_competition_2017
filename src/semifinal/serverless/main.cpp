#include "ChoosingStrategy.hpp"
#include "GameState.hpp"
#include "StrategyParser.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>

#include <iostream>
#include <random>
#include <set>
#include <time.h>

using Rng = std::mt19937;

struct Options {
    int numRuns = 1;
    unsigned seed = 0;
    std::vector<std::string> strategyStrings;
};

std::vector<Point> generatePoints(Rng& rng, int width, int height,
        std::size_t number) {
    std::set<Point> result;
    std::uniform_int_distribution<int> xDistribution{0, width - 1};
    std::uniform_int_distribution<int> yDistribution{0, height - 1};

    while (result.size() != number) {
        Point p{xDistribution(rng), yDistribution(rng)};
        result.insert(p);
    }
    return std::vector<Point>(result.begin(), result.end());
}

GameState generateGame(Rng& rng) {
    GameState result;
    result.width = 15;
    result.height = 6;
    result.numDisplays = 20;
    result.maxTick = 40;
    result.currentTick = 0;
    result.extraField = 15;

    std::uniform_int_distribution<int> fieldDistribution{1, 15};
    int numFields = result.width * result.height;
    std::vector<int> fields;
    fields.reserve(numFields);
    for (int i = 0; i < numFields; ++i) {
        fields.push_back(fieldDistribution(rng));
    }

    auto monitors =
            generatePoints(rng, result.width, result.height,
                    result.numDisplays);
    std::cerr << "Monitors: ";
    for (Point p : monitors) {
        std::cerr << p << " ";
    }
    std::cerr << "\n";

    result.track = Track(result.width, result.height, fields, monitors,
            generatePoints(rng, result.width, result.height, numPlayers));

    return result;
}

int getRandomMonitor(Rng& rng, const GameState& gameState) {
    std::uniform_int_distribution<int> distribution{0,
            gameState.numDisplays - 1};
    int result;
    do {
        result = distribution(rng);
    } while (gameState.track.getMonitor(result).x < 0);
    return result;
}

struct PlayerState {
    PlayerState(ChoosingStrategy strategy) : strategy{std::move(strategy)} {}

    PlayerState(const PlayerState&) = delete;
    PlayerState& operator=(const PlayerState&) = delete;

    PlayerState(PlayerState&&) noexcept = default;
    PlayerState& operator=(PlayerState&&) noexcept = default;


    ChoosingStrategy strategy;
    GameState gameState;
    int score = 0;
    clock_t time = 0;
};

void setPlayerMonitors(Rng& rng, std::vector<PlayerState>& playerStates,
        GameState& globalState) {
    for (PlayerState& playerState : playerStates) {
        if (globalState.track.getMonitor(playerState.gameState.targetMonitor)
                .x < 0) {
            playerState.gameState.targetMonitor = getRandomMonitor(
                    rng, globalState);
        }
    }
}

std::vector<ChoosingStrategy> createStrategies(Rng& rng,
        const Options& options) {
    std::vector<ChoosingStrategy> result;
    for (const std::string& strategyString : options.strategyStrings) {
        result.push_back(parseStrategy(strategyString, rng));
    }
    return result;
}

void runGame(Rng& rng, std::vector<PlayerState>& playerStates, bool print) {
    GameState gameState = generateGame(rng);
    for (std::size_t i = 0; i < playerStates.size(); ++i) {
        GameState& state = playerStates[i].gameState;
        state = gameState;
        state.playerId = i;
        state.targetMonitor = getRandomMonitor(rng, state);
    }

    for (gameState.currentTick = 0; gameState.currentTick < gameState.maxTick
            && gameState.track.getRemainingMonitors() != 0;
            ++gameState.currentTick) {
        for (PlayerState& playerState : playerStates) {
            int playerId = playerState.gameState.playerId;
            int targetMonitor = playerState.gameState.targetMonitor;
            Track& track = gameState.track;

            if (print) {
                std::cout
                        << "Tick " << gameState.currentTick
                        << " monitors " << track.getRemainingMonitors()
                        << "\n" << setColor(defaultColor,
                                playerColors[playerId])
                        << "Player " << playerId
                        << " score " << playerState.score
                        << " target " << track.getMonitor(targetMonitor)
                        << clearColor()
                        << "\n" << toBox(gameState.track, playerId,
                                targetMonitor);
            }

            playerState.gameState.track = track;
            playerState.gameState.currentTick = gameState.currentTick;

            clock_t start = ::clock();
            Step step = playerState.strategy(playerState.gameState);
            clock_t end = ::clock();
            playerState.time += end - start;
            if (print) {
                std::cout << "Step: push " << step.pushDirection
                        << " " << step.pushPosition
                        << " " << fieldTypes[step.pushFieldType]
                        << " princess=" << step.princessTarget << "\n";
            }
            playerState.gameState.extraField = executeStep(track, playerId,
                    step);

            if (track.getPrincess(playerId) ==
                    track.getMonitor(targetMonitor)) {
                ++playerState.score;
                if (print) {
                    std::cout << "Monitor removed: " << targetMonitor
                            << " " << track.getMonitor(targetMonitor)
                            << " Remaining: " << track.getRemainingMonitors()
                            << "\n"
                            << setColor(defaultColor, playerColors[playerId])
                            << "Score awarded to player " << playerId
                            << clearColor() << "\n";
                }
                track.removeMonitor(targetMonitor);
                if (track.getRemainingMonitors() == 0) {
                    break;
                }
                setPlayerMonitors(rng, playerStates, gameState);
            }
            if (print) {
                std::cout << "\n" << std::endl;
            }
        }
    }
}

namespace po = boost::program_options;

template <typename T>
po::typed_value<T>* defaultValue(T& v) {
    return po::value(&v)->default_value(v);
}

Options parseOptions(int argc, const char* argv[]) {
    po::options_description optionsDescription;
    Options options;
    bool help = false;
    optionsDescription.add_options()
        ("help,h", po::bool_switch(&help))
        ("num-runs,n", defaultValue(options.numRuns))
        ("seed", defaultValue(options.seed))
        ("strategy,s", po::value(&options.strategyStrings)->multitoken())
        ;

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
            options(optionsDescription).
            run(), vm);
    po::notify(vm);

    if (help) {
        std::cerr << optionsDescription << "\n";
        exit(0);
    }
    return options;
}

int main(int argc, const char* argv[]) {
    Options options = parseOptions(argc, argv);
    if (options.seed == 0) {
        options.seed = std::random_device{}();
    }
    Rng rng{options.seed};

    std::vector<PlayerState> playerStates;

    {
        std::vector<ChoosingStrategy> strategies = createStrategies(rng,
                options);
        for (int i = 0; i < static_cast<int>(numPlayers); ++i) {
            playerStates.emplace_back(strategies[i % strategies.size()]);
        }
    }

    for (int i = 0; i < options.numRuns; ++i) {
        std::cout << "Run #" << i << "\n";
        runGame(rng, playerStates, options.numRuns == 1);
    }

    std::cout << "Game over.\n";
    for (const PlayerState& playerState : playerStates) {
        int playerId = playerState.gameState.playerId;
        std::cout << setColor(defaultColor, playerColors[playerId])
                << "Player " << playerId << " final score "
                << playerState.score << " Total time spent: "
                << static_cast<double>(playerState.time) / CLOCKS_PER_SEC
                << " s" << clearColor() << std::endl;
    }
}
