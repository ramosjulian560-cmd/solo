// headers/game_engine/game_logic/boardgame/chess/pieces/pawn.hpp
#pragma once

#include "../chesspiece.hpp"

class Pawn final : public ChessPiece
{
public:
    explicit Pawn(ChessColor color) noexcept
        : ChessPiece(color, ChessPieceType::Pawn)
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
