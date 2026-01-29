// headers/game_engine/game_logic/boardgame/chess/pieces/rook.hpp
#pragma once

#include "../chesspiece.hpp"

class Rook final : public ChessPiece
{
public:
    explicit Rook(ChessColor color) noexcept
        : ChessPiece(color, ChessPieceType::Rook)
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