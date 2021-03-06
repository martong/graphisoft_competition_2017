#ifndef SEMIFINAL_CLIENT_CHOOSINGSTRATEGY_HPP
#define SEMIFINAL_CLIENT_CHOOSINGSTRATEGY_HPP

#include "IChooser.hpp"
#include "GameState.hpp"
#include "OpponentsInfo.hpp"

#include <memory>

class ChoosingStrategy {
public:
    ChoosingStrategy() = default;

    ChoosingStrategy(std::shared_ptr<IChooser> chooser) :
            chooser(std::move(chooser)) {}

    ChoosingStrategy(const ChoosingStrategy&) = default;
    ChoosingStrategy& operator=(const ChoosingStrategy&) = default;

    ChoosingStrategy(ChoosingStrategy&&) noexcept = default;
    ChoosingStrategy& operator=(ChoosingStrategy&&) = default;

    void init(GameInfo gameInfo) {
        this->gameState.gameInfo = std::move(gameInfo);
    }
    Step ourTurn(GameState gameState);
    void opponentsTurn(const Track& track, int playerId);
    const OpponentsInfo& getOpponentsInfo() const { return opponentsInfo; }

    void reset();

    void setGameState(GameState gameState);

private:
    Step calculateStep();

    friend struct OpponentsInfoTestFixture;
    void updateOpponentsInfo(const Track& track, int playerId);

    void setTargetMonitors(const Track& currentTrack);

    std::shared_ptr<IChooser> chooser;
    GameState gameState;

    OpponentsInfo opponentsInfo;

    struct PrevState {
        GameState gameState;
        int playerId = -1;
    } prevSt;

};

#endif // SEMIFINAL_CLIENT_CHOOSINGSTRATEGY_HPP
