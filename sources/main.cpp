#include <iostream>
#include <string>
#include <vector>

#include "game_engine/ai/StockfishEngine.hpp"
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

    StockfishEngine engine;
if (!engine.start("stockfish"))
{
    return 1;
}

std::vector<std::string> moves_uci;
constexpr int kDepth = 12;

std::cout << "Minimal Chess + Stockfish CLI\n";
std::cout << "Enter UCI moves like e2e4. Type 'quit' to exit.\n\n";

while (game.getResult() == Chess::GameResult::Ongoing)
{
    if (game.sideToMove() == ChessColor::White)
    {
        std::string uci;
        std::cout << "Your move (" << toString(game.sideToMove()) << "): ";

        if (!(std::cin >> uci))
        {
            engine.stop();
            return 0;
        }

        if (uci == "quit")
        {
            engine.stop();
            return 0;
        }

        const auto parsed = game.parseMoveUci(uci, game.sideToMove());
        if (!parsed.has_value())
        {
            std::cout << "Invalid move: " << uci << "\n";
            continue;
        }

        if (!game.applyMove(*parsed))
        {
            std::cout << "Move rejected by engine: " << uci << "\n";
            continue;
        }

        moves_uci.push_back(uci);
    }
    else
    {
        const auto ai_move = engine.bestMoveUci(moves_uci, kDepth);
        if (!ai_move.has_value())
        {
            engine.stop();
            return 0;
        }

        const auto parsed_ai = game.parseMoveUci(*ai_move, game.sideToMove());
        if (!parsed_ai.has_value() || !game.applyMove(*parsed_ai))
        {
            engine.stop();
            return 0;
        }

        moves_uci.push_back(*ai_move);
        std::cout << "AI played: " << *ai_move << "\n";
    }
}

engine.stop();
std::cout << "Game ended.\n";
return 0;
}
