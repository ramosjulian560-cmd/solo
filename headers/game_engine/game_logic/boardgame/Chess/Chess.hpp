// headers/game_engine/game_logic/boardgame/Chess/Chess.hpp
#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../Boardgame.hpp"
#include "ChessPiece.hpp"

/**
 * Chess = core rules + board state.
 *
 * Implemented in Chess.cpp:
 * - setBoard(): starting position + resets state
 * - legal move generation: pseudo + specials + king-safety filter
 * - applyMove(): updates board + castling/en-passant + promotion + draw counters + repetition
 * - status: check/checkmate/stalemate/draw/result
 * - UI glue: parse/format UCI (e2e4, e7e8q)
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

    enum class GameResult : std::uint8_t
    {
        Ongoing,
        WhiteWin,
        BlackWin,
        Draw
    };

    struct DrawInfo
    {
        bool is_draw{false};
        std::string reason{};
    };

    Chess()
        : Boardgame<PiecePtr>(
              "Chess",
              "E",
              "Boardgame",
              "Classic chess with full rules.",
              std::vector<std::string>{"Board", "Strategy"},
              0)
    {
        setBoard();
    }

    ~Chess() = default;

    Chess(const Chess&) = delete;
    Chess& operator=(const Chess&) = delete;
    Chess(Chess&&) noexcept = default;
    Chess& operator=(Chess&&) noexcept = default;

    // -------------------- State --------------------
    ChessColor sideToMove() const noexcept { return side_to_move_; }

    CastlingRights getCastlingRights() const noexcept { return castling_rights_; }

    bool hasEnPassantTarget() const noexcept { return has_en_passant_; }
    std::pair<int, int> getEnPassantTarget() const noexcept { return {en_passant_row_, en_passant_col_}; }

    int halfmoveClock() const noexcept { return halfmove_clock_; }

    void resetBoard();

    // -------------------- Move application --------------------
    bool applyMove(const ChessMove& move);

    // -------------------- Game status / result --------------------
    bool isCheckmate(ChessColor side) const;
    bool isStalemate(ChessColor side) const;

    DrawInfo getDrawInfo() const;
    GameResult getResult() const;
    int legalMoveCount(ChessColor side) const;

    // -------------------- UI glue (UCI) --------------------
    std::optional<ChessMove> parseMoveUci(const std::string& uci, ChessColor side) const;
    std::string formatMoveUci(const ChessMove& move) const;

protected:
    void setBoard() override;

private:
    static ChessColor opposite(ChessColor c) noexcept
    {
        return (c == ChessColor::White) ? ChessColor::Black : ChessColor::White;
    }

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

    // Internal legal move helpers
    std::vector<ChessMove> legalMovesFrom(int from_row, int from_col, ChessColor side) const;
    std::vector<ChessMove> allLegalMoves(ChessColor side) const;

    bool isSquareAttacked(int row, int col, ChessColor by_side) const;
    bool isInCheck(ChessColor side) const;
    bool isMoveLegal(const ChessMove& move, ChessColor side) const;

    // Engine-level special move generation
    std::vector<ChessMove> addCastlingMovesIfAny(int king_row, int king_col, ChessColor side) const;
    std::vector<ChessMove> addEnPassantMovesIfAny(int pawn_row, int pawn_col, ChessColor side) const;

    // Promotion expansion: pawn-to-last-rank expands into 4 moves (Q/R/B/N)
    std::vector<ChessMove> expandPromotionChoicesIfNeeded(const ChessMove& move, ChessColor side) const;

    // Internal move application helpers
    void applyMoveUnchecked(Board& b, const ChessMove& move, ChessColor side) const;
    void updateCastlingRightsAfterMove(const ChessMove& move, ChessColor side);
    void updateEnPassantAfterMove(const ChessMove& move, const PiecePtr& moved_piece);

    // Promotion piece factory (Q/R/B/N only)
    PiecePtr makePromotionPiece(ChessColor side, ChessPromotion promo) const;

private:
    // Core state
    ChessColor side_to_move_{ChessColor::White};
    CastlingRights castling_rights_{};

    bool has_en_passant_{false};
    int en_passant_row_{-1};
    int en_passant_col_{-1};

    // Draw/repetition state
    int halfmove_clock_{0}; // 100 plies => 50-move draw
    std::unordered_map<std::uint64_t, int> position_counts_{};
    std::uint64_t position_hash_{0};
};
