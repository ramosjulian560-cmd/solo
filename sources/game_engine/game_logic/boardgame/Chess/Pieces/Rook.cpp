// sources/game_engine/game_logic/boardgame/Chess/Pieces/Rook.cpp

#include "game_engine/game_logic/boardgame/Chess/Pieces/Rook.hpp"

#include <memory>
#include <vector>

std::vector<ChessMove> Rook::pseudoLegalMoves(
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

    static constexpr int kDirs[4][2] = {
        {+1,  0},
        {-1,  0},
        { 0, +1},
        { 0, -1},
    };

    for (const auto& d : kDirs)
    {
        int r = from_row + d[0];
        int c = from_col + d[1];

        while (inBounds(r, c))
        {
            const auto target = board[r][c].getBoardPiece();

            if (!target)
            {
                moves.push_back(ChessMove{from_row, from_col, r, c, nullptr});
                r += d[0];
                c += d[1];
                continue;
            }

            if (isOpponent(target))
            {
                moves.push_back(ChessMove{from_row, from_col, r, c, nullptr});
            }
            break; // blocked by any piece
        }
    }

    return moves;
}
