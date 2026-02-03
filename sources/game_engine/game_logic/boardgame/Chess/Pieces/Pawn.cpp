// sources/game_engine/game_logic/boardgame/Chess/Pieces/Pawn.cpp

#include "game_engine/game_logic/boardgame/Chess/Pieces/Pawn.hpp"

#include <memory>
#include <vector>

std::vector<ChessMove> Pawn::pseudoLegalMoves(
    const std::vector<std::vector<BoardPosition<std::shared_ptr<ChessPiece>>>>& board,
    int from_row,
    int from_col
) const
{
    std::vector<ChessMove> moves;

    auto inBounds = [](int r, int c) noexcept {
        return r >= 0 && r < 8 && c >= 0 && c < 8;
    };

    const bool is_white = (color() == ChessColor::White);
    const int dir = is_white ? -1 : +1;
    const int start_row = is_white ? 6 : 1;

    const int one_r = from_row + dir;

    // Forward 1
    if (inBounds(one_r, from_col) && !board[one_r][from_col].getBoardPiece())
    {
        moves.push_back(ChessMove{from_row, from_col, one_r, from_col, nullptr});

        // Forward 2 from starting rank (only if forward 1 is empty too)
        const int two_r = from_row + 2 * dir;
        if (from_row == start_row && inBounds(two_r, from_col) &&
            !board[two_r][from_col].getBoardPiece())
        {
            moves.push_back(ChessMove{from_row, from_col, two_r, from_col, nullptr});
        }
    }

    // Diagonal captures
    for (int dc : {-1, +1})
    {
        const int r = from_row + dir;
        const int c = from_col + dc;
        if (!inBounds(r, c)) continue;

        const auto target = board[r][c].getBoardPiece();
        if (isOpponent(target))
        {
            moves.push_back(ChessMove{from_row, from_col, r, c, nullptr});
        }
    }

    return moves;
}
