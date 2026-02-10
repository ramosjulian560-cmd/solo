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
        std::cerr << "Failed to start Stockfish. Make sure 'stockfish' is installed and on PATH.\n";
        return 1;
    }

    std::vector<std::string> moves_uci;
    constexpr int kDepth = 12;

    std::cout << "Minimal Chess + Stockfish CLI\n";
    std::cout << "Enter UCI moves like e2e4. Type 'quit' to exit.\n\n";

    while (true)
    {
        const auto result = game.getResult();
        if (result != Chess::GameResult::Ongoing)
        {
            std::cout << "Game ended.\n";
            break;
        }

        if (game.sideToMove() == ChessColor::White)
        {
            std::string uci;
            std::cout << "Your move (" << toString(game.sideToMove()) << "): ";
            if (!(std::cin >> uci)) break;
            if (uci == "quit") break;

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
            std::cout << "You played: " << uci << "\n";
            continue;
        }

        const auto ai_move = engine.bestMoveUci(moves_uci, kDepth);
        if (!ai_move.has_value())
        {
            std::cout << "Stockfish did not return a move.\n";
            break;
        }

        const auto parsed_ai = game.parseMoveUci(*ai_move, game.sideToMove());
        if (!parsed_ai.has_value() || !game.applyMove(*parsed_ai))
        {
            std::cout << "AI move invalid in current engine state: " << *ai_move << "\n";
            break;
        }

        moves_uci.push_back(*ai_move);
        std::cout << "AI played: " << *ai_move << "\n";
    }

    engine.stop();

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
