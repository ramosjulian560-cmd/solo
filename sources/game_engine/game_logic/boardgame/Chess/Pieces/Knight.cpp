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

    constexpr int kN = 8;
    auto inBounds = [](int r, int c) noexcept {
        return r >= 0 && r < kN && c >= 0 && c < kN;
    };

    if (!inBounds(from_row, from_col)) return moves;

    const auto self = board[from_row][from_col].getBoardPiece();
    if (!self || self->color() != color()) return moves;

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
