// sources/game_engine/game_logic/boardgame/Chess/Pieces/Queen.cpp

#include "game_engine/game_logic/boardgame/Chess/Pieces/Queen.hpp"

#include <memory>
#include <vector>

std::vector<ChessMove> Queen::pseudoLegalMoves(
    const std::vector<std::vector<BoardPosition<std::shared_ptr<ChessPiece>>>>& board,
    int from_row,
    int from_col
) const
{
    std::vector<ChessMove> moves;

    auto inBounds = [](int r, int c) noexcept {
        return r >= 0 && r < 8 && c >= 0 && c < 8;
    };

    static constexpr int kDirs[8][2] = {
        {+1,  0}, {-1,  0}, { 0, +1}, { 0, -1}, // rook-like
        {+1, +1}, {+1, -1}, {-1, +1}, {-1, -1}  // bishop-like
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
