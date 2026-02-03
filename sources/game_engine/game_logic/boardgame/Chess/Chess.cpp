// sources/game_engine/game_logic/boardgame/Chess/Chess.cpp

#include "game_engine/game_logic/boardgame/Chess/Chess.hpp"

#include "game_engine/game_logic/boardgame/Chess/ChessPiece.hpp"

#include <optional>
#include <utility>

// -------- helpers (internal only) ----------
namespace
{
    using PiecePtr = std::shared_ptr<ChessPiece>;
    using Board = std::vector<std::vector<BoardPosition<PiecePtr>>>;

    constexpr int N = 8;

    bool inBounds(int r, int c) noexcept
    {
        return r >= 0 && r < N && c >= 0 && c < N;
    }

    ChessColor opposite(ChessColor c) noexcept
    {
        return (c == ChessColor::White) ? ChessColor::Black : ChessColor::White;
    }

    void setSquare(Board& b, int r, int c, PiecePtr p)
    {
        b[r][c] = BoardPosition<PiecePtr>(r, c, std::move(p));
    }

    std::optional<std::pair<int, int>> findKing(const Board& b, ChessColor side)
    {
        for (int r = 0; r < N; ++r)
        {
            for (int c = 0; c < N; ++c)
            {
                const auto p = b[r][c].getBoardPiece();
                if (!p) continue;
                if (p->type() == ChessPieceType::King && p->color() == side)
                {
                    return std::make_pair(r, c);
                }
            }
        }
        return std::nullopt;
    }

    bool attacksSquarePawn(int from_r, int from_c, ChessColor pawn_color, int target_r, int target_c)
    {
        const int dir = (pawn_color == ChessColor::White) ? -1 : +1;
        const int r = from_r + dir;
        if (r != target_r) return false;
        return (target_c == from_c - 1) || (target_c == from_c + 1);
    }

    bool isSquareAttacked(const Board& b, int target_r, int target_c, ChessColor by_side)
    {
        for (int r = 0; r < N; ++r)
        {
            for (int c = 0; c < N; ++c)
            {
                const auto p = b[r][c].getBoardPiece();
                if (!p) continue;
                if (p->color() != by_side) continue;

                // Pawn attacks are NOT the same as pawn pseudo moves (forward squares aren't attacks).
                if (p->type() == ChessPieceType::Pawn)
                {
                    if (attacksSquarePawn(r, c, by_side, target_r, target_c))
                        return true;
                    continue;
                }

                // For all other pieces, pseudo moves represent attack squares well enough (for now).
                const auto moves = p->pseudoLegalMoves(b, r, c);
                for (const auto& m : moves)
                {
                    if (m.to_row == target_r && m.to_col == target_c)
                        return true;
                }
            }
        }
        return false;
    }

    bool wouldLeaveKingInCheck(const Board& b, const ChessMove& move, ChessColor moving_side)
    {
        Board copy = b;

        const auto moving_piece = copy[move.from_row][move.from_col].getBoardPiece();
        if (!moving_piece) return true;

        // Apply move on copy
        setSquare(copy, move.from_row, move.from_col, nullptr);
        setSquare(copy, move.to_row, move.to_col, moving_piece);

        const auto king_pos = findKing(copy, moving_side);
        if (!king_pos.has_value())
            return true; // invalid position (no king)

        const auto [kr, kc] = king_pos.value();
        return isSquareAttacked(copy, kr, kc, opposite(moving_side));
    }

    bool isMoveInPseudoList(const Board& b, int from_r, int from_c, const ChessMove& candidate)
    {
        const auto piece = b[from_r][from_c].getBoardPiece();
        if (!piece) return false;

        const auto pseudo = piece->pseudoLegalMoves(b, from_r, from_c);
        for (const auto& m : pseudo)
        {
            if (m.to_row == candidate.to_row && m.to_col == candidate.to_col &&
                m.from_row == candidate.from_row && m.from_col == candidate.from_col)
            {
                return true;
            }
        }
        return false;
    }
} // namespace

// -------- public API ----------
bool Chess::isInCheck(ChessColor side) const
{
    const auto king_pos = findKing(board, side);
    if (!king_pos.has_value())
        return false; // or true; but "false" keeps things from hard-failing

    const auto [kr, kc] = king_pos.value();
    return isSquareAttacked(board, kr, kc, opposite(side));
}

std::vector<ChessMove> Chess::legalMovesFrom(int from_row, int from_col, ChessColor side) const
{
    std::vector<ChessMove> legal;

    if (!inBounds(from_row, from_col)) return legal;

    const auto piece = board[from_row][from_col].getBoardPiece();
    if (!piece) return legal;
    if (piece->color() != side) return legal;

    const auto pseudo = piece->pseudoLegalMoves(board, from_row, from_col);

    for (const auto& m : pseudo)
    {
        // Safety: ensure this move is indeed from that square (your piece code already does this)
        if (m.from_row != from_row || m.from_col != from_col) continue;

        // Ensure it's a real pseudo move from this piece (guards against bad move structs)
        if (!isMoveInPseudoList(board, from_row, from_col, m)) continue;

        if (!wouldLeaveKingInCheck(board, m, side))
        {
            legal.push_back(m);
        }
    }

    return legal;
}

std::vector<ChessMove> Chess::allLegalMoves(ChessColor side) const
{
    std::vector<ChessMove> all;

    for (int r = 0; r < N; ++r)
    {
        for (int c = 0; c < N; ++c)
        {
            const auto p = board[r][c].getBoardPiece();
            if (!p) continue;
            if (p->color() != side) continue;

            auto moves = legalMovesFrom(r, c, side);
            all.insert(all.end(), moves.begin(), moves.end());
        }
    }

    return all;
}
