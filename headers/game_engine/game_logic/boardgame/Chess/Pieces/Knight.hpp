// headers/game_engine/game_logic/boardgame/Chess/Pieces/Knight.hpp
#pragma once

#include "../chesspiece.hpp"

class Knight final : public ChessPiece
{
public:
    explicit Knight(ChessColor color) noexcept
        : ChessPiece(color, ChessPieceType::Knight)
    {}

    std::vector<ChessMove> pseudoLegalMoves(
        const std::vector<std::vector<BoardPosition<std::shared_ptr<ChessPiece>>>>& board,
        int from_row,
        int from_col
    ) const override;

private:
    static bool inBounds(int r, int c) noexcept
    {
        return r >= 0 && r < 8 && c >= 0 && c < 8;
    }
};