// sources/game_engine/game_logic/boardgame/Chess/Pieces/King.cpp

#include "game_engine/game_logic/boardgame/Chess/Pieces/King.hpp"

#include <memory>
#include <vector>

std::vector<ChessMove> King::pseudoLegalMoves(
    const std::vector<std::vector<BoardPosition<std::shared_ptr<ChessPiece>>>>& board,
    int from_row,
    int from_col
) const
{
    std::vector<ChessMove> moves;

    auto inBounds = [](int r, int c) noexcept {
        return r >= 0 && r < 8 && c >= 0 && c < 8;
    };

    for (int dr = -1; dr <= 1; ++dr)
    {
        for (int dc = -1; dc <= 1; ++dc)
        {
            if (dr == 0 && dc == 0) continue;

            const int r = from_row + dr;
            const int c = from_col + dc;
            if (!inBounds(r, c)) continue;

            const auto target = board[r][c].getBoardPiece();
            if (!target || isOpponent(target))
            {
                moves.push_back(ChessMove{from_row, from_col, r, c, nullptr});
            }
        }
    }

    return moves;
}
