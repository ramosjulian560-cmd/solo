// headers/game_engine/game_logic/boardgame/Chess/Pieces/Bishop.hpp
#pragma once


#include "../ChessPiece.hpp"


class Bishop final : public ChessPiece
{
public:
    explicit Bishop(ChessColor color) noexcept
        : ChessPiece(color, ChessPieceType::Bishop)
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