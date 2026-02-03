// sources/game_engine/game_logic/boardgame/Chess/Pieces/Knight.cpp

#include "game_engine/game_logic/boardgame/Chess/Pieces/Knight.hpp"

#include <memory>
#include <vector>

std::vector<ChessMove> Knight::pseudoLegalMoves(
    const std::vector<std::vector<BoardPosition<std::shared_ptr<ChessPiece>>>>& board,
    int from_row,
    int from_col
) const
{
    std::vector<ChessMove> moves;

    auto inBounds = [](int r, int c) noexcept {
        return r >= 0 && r < 8 && c >= 0 && c < 8;
    };

    static constexpr int kJumps[8][2] = {
        {+2, +1}, {+2, -1},
        {-2, +1}, {-2, -1},
        {+1, +2}, {+1, -2},
        {-1, +2}, {-1, -2},
    };

    for (const auto& j : kJumps)
    {
        const int r = from_row + j[0];
        const int c = from_col + j[1];
        if (!inBounds(r, c)) continue;

        const auto target = board[r][c].getBoardPiece();
        if (!target || isOpponent(target))
        {
            moves.push_back(ChessMove{from_row, from_col, r, c, nullptr});
        }
    }

    return moves;
}
