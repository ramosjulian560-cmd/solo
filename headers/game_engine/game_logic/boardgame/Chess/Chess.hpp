// headers/game_engine/game_logic/boardgame/Chess/Chess.hpp
#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../Boardgame.hpp"
#include "ChessPiece.hpp"

/**
 * Chess = core rules + board state.
 *
 * - setBoard(): initialize starting position
 * - pseudoLegalMoves(): handled by piece classes
 * - legal move filtering: king-safety + special rules (castling / en-passant / promotion)
 * - applyMove(): updates board + turn + castling rights + en-passant target + promotion/castle/en-passant effects
 */
class Chess final : public Boardgame<std::shared_ptr<ChessPiece>>
{
public:
    using PiecePtr = std::shared_ptr<ChessPiece>;
    using Board = std::vector<std::vector<BoardPosition<PiecePtr>>>;

    struct CastlingRights
    {
        bool white_kingside{true};
        bool white_queenside{true};
        bool black_kingside{true};
        bool black_queenside{true};
    };

    Chess()
        : Boardgame<PiecePtr>(
              "Chess",
              "E",
              "Boardgame",
              "Classic chess with full rules.",
              std::vector<std::string>{"Board", "Strategy"},
              0)
    {}

    ~Chess() override = default;

    Chess(const Chess&) = delete;
    Chess& operator=(const Chess&) = delete;
    Chess(Chess&&) noexcept = default;
    Chess& operator=(Chess&&) noexcept = default;

    // -------------------- Turn / State --------------------
    ChessColor sideToMove() const noexcept { return side_to_move_; }
    void setSideToMove(ChessColor side) noexcept { side_to_move_ = side; }

    CastlingRights getCastlingRights() const noexcept { return castling_rights_; }
    void setCastlingRights(const CastlingRights& rights) noexcept { castling_rights_ = rights; }

    bool hasEnPassantTarget() const noexcept { return has_en_passant_; }
    std::pair<int, int> getEnPassantTarget() const noexcept { return {en_passant_row_, en_passant_col_}; }

    void clearEnPassantTarget() noexcept
    {
        has_en_passant_ = false;
        en_passant_row_ = -1;
        en_passant_col_ = -1;
    }

    void setEnPassantTarget(int row, int col) noexcept
    {
        has_en_passant_ = true;
        en_passant_row_ = row;
        en_passant_col_ = col;
    }

    // -------------------- Legal move API --------------------
    std::vector<ChessMove> legalMovesFrom(int from_row, int from_col, ChessColor side) const;
    std::vector<ChessMove> allLegalMoves(ChessColor side) const;
    bool isInCheck(ChessColor side) const;

    // -------------------- Move application --------------------
    bool applyMove(const ChessMove& move);
    bool isMoveLegal(const ChessMove& move, ChessColor side) const;

protected:
    void setBoard() override;

private:
    static ChessColor opposite(ChessColor c) noexcept
    {
        return (c == ChessColor::White) ? ChessColor::Black : ChessColor::White;
    }

    // Attack / check helpers
    bool isSquareAttacked(int row, int col, ChessColor by_side) const;

    // Special move generation (called by legalMovesFrom/allLegalMoves)
    std::vector<ChessMove> addCastlingMovesIfAny(int king_row, int king_col, ChessColor side) const;
    std::vector<ChessMove> addEnPassantMovesIfAny(int pawn_row, int pawn_col, ChessColor side) const;

    // Promotion expansion: when a pawn reaches last rank, generate 4 moves (Q/R/B/N)
    std::vector<ChessMove> expandPromotionChoicesIfNeeded(const ChessMove& move, ChessColor side) const;

    // Internal move application helpers
    void applyMoveUnchecked(Board& b, const ChessMove& move, ChessColor side) const;
    void updateCastlingRightsAfterMove(const ChessMove& move, ChessColor side);
    void updateEnPassantAfterMove(const ChessMove& move, const PiecePtr& moved_piece);

    // Promotion piece factory (Q/R/B/N only)
    PiecePtr makePromotionPiece(ChessColor side, ChessPromotion promo) const;

private:
    ChessColor side_to_move_{ChessColor::White};
    CastlingRights castling_rights_{};

    bool has_en_passant_{false};
    int en_passant_row_{-1};
    int en_passant_col_{-1};
};
