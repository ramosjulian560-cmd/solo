// headers/game_engine/game_logic/boardgame/Chess/ChessPiece.hpp
#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "../BoardPosition.hpp"

class ChessPiece;

enum class ChessColor : std::uint8_t
{
    White,
    Black
};

enum class ChessPieceType : std::uint8_t
{
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King
};

enum class ChessSpecialMove : std::uint8_t
{
    Normal,
    CastleKingside,
    CastleQueenside,
    EnPassant,
    Promotion
};

enum class ChessPromotion : std::uint8_t
{
    None,
    Queen,
    Rook,
    Bishop,
    Knight
};

struct ChessMove
{
    int from_row{};
    int from_col{};
    int to_row{};
    int to_col{};

    // Keep this 5th so existing code like ChessMove{..., nullptr} still compiles.
    std::shared_ptr<ChessPiece> promotion_piece{};

    // New metadata (defaults keep old move creation working)
    ChessSpecialMove special{ChessSpecialMove::Normal};
    ChessPromotion promotion{ChessPromotion::None};

    // Castling rook move (only used when special is CastleKingside/CastleQueenside)
    int rook_from_row{-1};
    int rook_from_col{-1};
    int rook_to_row{-1};
    int rook_to_col{-1};

    // En-passant capture square (only used when special is EnPassant)
    int ep_capture_row{-1};
    int ep_capture_col{-1};

    bool isPromotionMove() const noexcept { return special == ChessSpecialMove::Promotion; }

    // Mandatory selection: promotion move is incomplete until Q/R/B/N is chosen.
    bool isComplete() const noexcept
    {
        if (!isPromotionMove()) return true;
        return promotion != ChessPromotion::None;
    }
};

class ChessPiece
{
public:
    virtual ~ChessPiece() = default;

    ChessColor color() const noexcept { return color_; }
    ChessPieceType type() const noexcept { return type_; }

    virtual std::vector<ChessMove> pseudoLegalMoves(
        const std::vector<std::vector<BoardPosition<std::shared_ptr<ChessPiece>>>>& board,
        int from_row,
        int from_col) const = 0;

    bool isOpponent(const std::shared_ptr<ChessPiece>& other) const noexcept
    {
        return other && other->color() != color_;
    }

protected:
    ChessPiece(ChessColor color, ChessPieceType type) noexcept : color_(color), type_(type) {}

private:
    ChessColor color_;
    ChessPieceType type_;
};
