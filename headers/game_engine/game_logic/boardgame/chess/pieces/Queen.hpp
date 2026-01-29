// headers/game_engine/game_logic/boardgame/chess/pieces/queen.hpp
#pragma once

#include "../chesspiece.hpp"

class Queen final : public ChessPiece
{
public:
    explicit Queen(ChessColor color) noexcept
        : ChessPiece(color, ChessPieceType::Queen)
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
