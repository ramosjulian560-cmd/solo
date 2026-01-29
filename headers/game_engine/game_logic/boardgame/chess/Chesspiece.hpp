#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "../BoardPosition.hpp"

class ChessPiece;

enum class ChessColor : uint8_t
{
    White,
    Black
};

enum class ChessPieceType : uint8_t
{
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King
};

struct ChessMove
{
    int from_row{};
    int from_col{};
    int to_row{};
    int to_col{};

    // Promotion is only used for pawn moves reaching the last rank.
    // If not promoting, leave as nullptr.
    std::shared_ptr<ChessPiece> promotion_piece{};
};

class ChessPiece
{
public:
    virtual ~ChessPiece() = default;

    ChessColor color() const noexcept { return color_; }
    ChessPieceType type() const noexcept { return type_; }

    // Pseudo-legal moves: piece movement rules only (does NOT check for leaving king in check).
    // "board" uses the engine's BoardPosition<BoardPiece> with BoardPiece = std::shared_ptr<ChessPiece>.
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
