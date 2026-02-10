// sources/game_engine/game_logic/boardgame/Chess/Pieces/Pawn.cpp

#include "game_engine/game_logic/boardgame/Chess/Pieces/Pawn.hpp"

#include <vector>

namespace
{
    constexpr int kN = 8;

    bool inBounds(int r, int c) noexcept
    {
        return r >= 0 && r < kN && c >= 0 && c < kN;
    }
} // namespace


std::vector<ChessMove> Pawn::pseudoLegalMoves(
    const std::vector<std::vector<BoardPosition<std::shared_ptr<ChessPiece>>>>& board,
    int from_row,
    int from_col) const
{
    std::vector<ChessMove> moves;

    if (!inBounds(from_row, from_col)) return moves;

    const auto self = board[from_row][from_col].getBoardPiece();
    if (!self || self->color() != color()) return moves;

    const int dir = (color() == ChessColor::White) ? -1 : +1;
    const int start_row = (color() == ChessColor::White) ? 6 : 1;

    const int one_r = from_row + dir;

    // Forward 1 (must be empty)
    if (inBounds(one_r, from_col) && !board[one_r][from_col].getBoardPiece())
    {
        ChessMove m{};
        m.from_row = from_row;
        m.from_col = from_col;
        m.to_row = one_r;
        m.to_col = from_col;
        moves.push_back(m);

        // Forward 2 from start (both squares must be empty)
        const int two_r = from_row + 2 * dir;
        if (from_row == start_row &&
            inBounds(two_r, from_col) &&
            !board[two_r][from_col].getBoardPiece())
        {
            ChessMove m2{};
            m2.from_row = from_row;
            m2.from_col = from_col;
            m2.to_row = two_r;
            m2.to_col = from_col;
            moves.push_back(m2);
        }
    }

    // Diagonal captures
    for (int dc : {-1, +1})
    {
        const int cap_r = from_row + dir;
        const int cap_c = from_col + dc;
        if (!inBounds(cap_r, cap_c)) continue;

        const auto target = board[cap_r][cap_c].getBoardPiece();
        if (target && isOpponent(target))
        {
            ChessMove m{};
            m.from_row = from_row;
            m.from_col = from_col;
            m.to_row = cap_r;
            m.to_col = cap_c;
            moves.push_back(m);
        }
    }

    return moves;
}
