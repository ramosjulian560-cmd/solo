#include <iostream>

#include "game_engine/game_logic/boardgame/Chess/Chess.hpp"

namespace
{
const char* toString(ChessColor side)
{
    return (side == ChessColor::White) ? "White" : "Black";
}
} // namespace

int main()
{
    Chess game;
    game.resetBoard();

    std::cout << "Side to move: " << toString(game.sideToMove()) << "\n";
    std::cout << "White legal moves at start: " << game.legalMoveCount(ChessColor::White) << "\n";

    const std::string uci = "e2e4";
    const auto parsed = game.parseMoveUci(uci, game.sideToMove());

    if (!parsed.has_value())
    {
        std::cout << "Failed to parse move: " << uci << "\n";
        return 1;
    }

    if (!game.applyMove(*parsed))
    {
        std::cout << "Failed to apply move: " << uci << "\n";
        return 1;
    }

    std::cout << "Applied move: " << uci << "\n";
    std::cout << "Black legal moves after " << uci << ": "
              << game.legalMoveCount(ChessColor::Black) << "\n";

    return 0;
}
