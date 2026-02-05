// sources/game_engine/game_logic/boardgame/Chess/Chess.cpp

#include "game_engine/game_logic/boardgame/Chess/Chess.hpp"

#include "game_engine/game_logic/boardgame/Chess/ChessPiece.hpp"
#include "game_engine/game_logic/boardgame/Chess/Pieces/Bishop.hpp"
#include "game_engine/game_logic/boardgame/Chess/Pieces/King.hpp"
#include "game_engine/game_logic/boardgame/Chess/Pieces/Knight.hpp"
#include "game_engine/game_logic/boardgame/Chess/Pieces/Pawn.hpp"
#include "game_engine/game_logic/boardgame/Chess/Pieces/Queen.hpp"
#include "game_engine/game_logic/boardgame/Chess/Pieces/Rook.hpp"

#include <cctype>
#include <cmath>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <utility>

namespace
{
    constexpr int kN = 8;

    bool inBounds(int r, int c) noexcept
    {
        return r >= 0 && r < kN && c >= 0 && c < kN;
    }

    int fileFromCol(int c) noexcept { return c; }

    bool isPawnAttackSquare(int pawn_r, int pawn_c, ChessColor pawn_color, int target_r, int target_c) noexcept
    {
        const int dir = (pawn_color == ChessColor::White) ? -1 : +1;
        if (target_r != pawn_r + dir) return false;
        return (target_c == pawn_c - 1) || (target_c == pawn_c + 1);
    }

    std::optional<std::pair<int, int>> findKingSquare(
        const std::vector<std::vector<BoardPosition<std::shared_ptr<ChessPiece>>>>& b,
        ChessColor side)
    {
        for (int r = 0; r < kN; ++r)
        {
            for (int c = 0; c < kN; ++c)
            {
                const auto p = b[r][c].getBoardPiece();
                if (!p) continue;
                if (p->color() == side && p->type() == ChessPieceType::King)
                    return std::make_pair(r, c);
            }
        }
        return std::nullopt;
    }

    void setSquare(
        std::vector<std::vector<BoardPosition<std::shared_ptr<ChessPiece>>>>& b,
        int r,
        int c,
        std::shared_ptr<ChessPiece> p)
    {
        b[r][c] = BoardPosition<std::shared_ptr<ChessPiece>>(r, c, std::move(p));
    }

    // ---- Zobrist hashing (deterministic) ----
    int pieceIndex(const std::shared_ptr<ChessPiece>& p) noexcept
    {
        if (!p) return -1;
        const int color = (p->color() == ChessColor::White) ? 0 : 1;
        return color * 6 + static_cast<int>(p->type());
    }

    std::uint64_t splitmix64(std::uint64_t& x) noexcept
    {
        x += 0x9E3779B97F4A7C15ULL;
        std::uint64_t z = x;
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
        return z ^ (z >> 31);
    }

    struct Zobrist
    {
        std::uint64_t piece[12][64]{};
        std::uint64_t side_to_move{};
        std::uint64_t castling[16]{};
        std::uint64_t ep_file[9]{}; // 0..7 file, 8 = none

        Zobrist()
        {
            std::uint64_t seed = 0xC0FFEE123456789ULL;
            for (int pi = 0; pi < 12; ++pi)
            {
                for (int sq = 0; sq < 64; ++sq)
                {
                    piece[pi][sq] = splitmix64(seed);
                }
            }
            side_to_move = splitmix64(seed);
            for (int i = 0; i < 16; ++i) castling[i] = splitmix64(seed);
            for (int i = 0; i < 9; ++i) ep_file[i] = splitmix64(seed);
        }
    };

    const Zobrist& zobrist()
    {
        static const Zobrist z;
        return z;
    }

    int castleMask(const Chess::CastlingRights& cr) noexcept
    {
        int m = 0;
        if (cr.white_kingside)  m |= 1 << 0;
        if (cr.white_queenside) m |= 1 << 1;
        if (cr.black_kingside)  m |= 1 << 2;
        if (cr.black_queenside) m |= 1 << 3;
        return m;
    }

    std::uint64_t computeHash(
        const std::vector<std::vector<BoardPosition<std::shared_ptr<ChessPiece>>>>& b,
        ChessColor side_to_move,
        const Chess::CastlingRights& cr,
        bool has_ep,
        int ep_row,
        int ep_col)
    {
        std::uint64_t h = 0;

        for (int r = 0; r < kN; ++r)
        {
            for (int c = 0; c < kN; ++c)
            {
                const auto p = b[r][c].getBoardPiece();
                const int idx = pieceIndex(p);
                if (idx < 0) continue;
                const int sq = r * 8 + c;
                h ^= zobrist().piece[idx][sq];
            }
        }

        if (side_to_move == ChessColor::Black)
            h ^= zobrist().side_to_move;

        h ^= zobrist().castling[castleMask(cr)];

        const int epf = has_ep ? fileFromCol(ep_col) : 8;
        h ^= zobrist().ep_file[epf];

        (void)ep_row;
        return h;
    }

    int squareColor(int r, int c) noexcept
    {
        return (r + c) & 1;
    }
} // namespace

// -------------------- Board init --------------------

void Chess::setBoard()
{
    board.clear();
    board.resize(kN);

    for (int r = 0; r < kN; ++r)
    {
        board[r].clear();
        board[r].reserve(kN);
        for (int c = 0; c < kN; ++c)
        {
            board[r].emplace_back(r, c, nullptr);
        }
    }

    auto place = [this](int r, int c, PiecePtr p)
    {
        board[r][c] = BoardPosition<PiecePtr>(r, c, std::move(p));
    };

    side_to_move_ = ChessColor::White;
    castling_rights_ = CastlingRights{};
    clearEnPassantTarget();

    halfmove_clock_ = 0;
    position_counts_.clear();

    place(0, 0, std::make_shared<Rook>(ChessColor::Black));
    place(0, 1, std::make_shared<Knight>(ChessColor::Black));
    place(0, 2, std::make_shared<Bishop>(ChessColor::Black));
    place(0, 3, std::make_shared<Queen>(ChessColor::Black));
    place(0, 4, std::make_shared<King>(ChessColor::Black));
    place(0, 5, std::make_shared<Bishop>(ChessColor::Black));
    place(0, 6, std::make_shared<Knight>(ChessColor::Black));
    place(0, 7, std::make_shared<Rook>(ChessColor::Black));

    for (int c = 0; c < kN; ++c)
        place(1, c, std::make_shared<Pawn>(ChessColor::Black));

    for (int c = 0; c < kN; ++c)
        place(6, c, std::make_shared<Pawn>(ChessColor::White));

    place(7, 0, std::make_shared<Rook>(ChessColor::White));
    place(7, 1, std::make_shared<Knight>(ChessColor::White));
    place(7, 2, std::make_shared<Bishop>(ChessColor::White));
    place(7, 3, std::make_shared<Queen>(ChessColor::White));
    place(7, 4, std::make_shared<King>(ChessColor::White));
    place(7, 5, std::make_shared<Bishop>(ChessColor::White));
    place(7, 6, std::make_shared<Knight>(ChessColor::White));
    place(7, 7, std::make_shared<Rook>(ChessColor::White));

    position_hash_ = computeHash(board, side_to_move_, castling_rights_, has_en_passant_, en_passant_row_, en_passant_col_);
    position_counts_[position_hash_] = 1;
}

void Chess::resetBoard()
{
    setBoard();
}

// -------------------- Attacks / check --------------------

bool Chess::isSquareAttacked(int row, int col, ChessColor by_side) const
{
    for (int r = 0; r < kN; ++r)
    {
        for (int c = 0; c < kN; ++c)
        {
            const auto p = board[r][c].getBoardPiece();
            if (!p || p->color() != by_side) continue;

            if (p->type() == ChessPieceType::Pawn)
            {
                if (isPawnAttackSquare(r, c, by_side, row, col)) return true;
                continue;
            }

            const auto pseudo = p->pseudoLegalMoves(board, r, c);
            for (const auto& m : pseudo)
            {
                if (m.to_row == row && m.to_col == col) return true;
            }
        }
    }
    return false;
}

bool Chess::isInCheck(ChessColor side) const
{
    const auto king = findKingSquare(board, side);
    if (!king.has_value()) return false;
    const auto [kr, kc] = king.value();
    return isSquareAttacked(kr, kc, opposite(side));
}

// -------------------- Promotion expansion --------------------

std::vector<ChessMove> Chess::expandPromotionChoicesIfNeeded(const ChessMove& move, ChessColor side) const
{
    std::vector<ChessMove> out;

    if (!inBounds(move.from_row, move.from_col) || !inBounds(move.to_row, move.to_col))
        return out;

    const auto piece = board[move.from_row][move.from_col].getBoardPiece();
    if (!piece) return out;

    if (piece->type() != ChessPieceType::Pawn)
    {
        out.push_back(move);
        return out;
    }

    const int last_rank = (side == ChessColor::White) ? 0 : 7;
    if (move.to_row != last_rank)
    {
        out.push_back(move);
        return out;
    }

    ChessMove q = move;
    q.special = ChessSpecialMove::Promotion;
    q.promotion = ChessPromotion::Queen;
    out.push_back(q);

    ChessMove r = move;
    r.special = ChessSpecialMove::Promotion;
    r.promotion = ChessPromotion::Rook;
    out.push_back(r);

    ChessMove b = move;
    b.special = ChessSpecialMove::Promotion;
    b.promotion = ChessPromotion::Bishop;
    out.push_back(b);

    ChessMove n = move;
    n.special = ChessSpecialMove::Promotion;
    n.promotion = ChessPromotion::Knight;
    out.push_back(n);

    return out;
}

// -------------------- Special move generation --------------------

std::vector<ChessMove> Chess::addEnPassantMovesIfAny(int pawn_row, int pawn_col, ChessColor side) const
{
    std::vector<ChessMove> out;
    if (!has_en_passant_) return out;

    const auto pawn = board[pawn_row][pawn_col].getBoardPiece();
    if (!pawn || pawn->color() != side || pawn->type() != ChessPieceType::Pawn) return out;

    const int dir = (side == ChessColor::White) ? -1 : +1;

    const int target_r = en_passant_row_;
    const int target_c = en_passant_col_;

    if (target_r != pawn_row + dir) return out;
    if (std::abs(target_c - pawn_col) != 1) return out;

    if (!inBounds(target_r, target_c)) return out;
    if (board[target_r][target_c].getBoardPiece()) return out;

    const int cap_r = (side == ChessColor::White) ? (target_r + 1) : (target_r - 1);
    const int cap_c = target_c;
    if (!inBounds(cap_r, cap_c)) return out;

    const auto captured = board[cap_r][cap_c].getBoardPiece();
    if (!captured || captured->type() != ChessPieceType::Pawn || captured->color() != opposite(side)) return out;

    ChessMove m{};
    m.from_row = pawn_row;
    m.from_col = pawn_col;
    m.to_row = target_r;
    m.to_col = target_c;
    m.special = ChessSpecialMove::EnPassant;
    m.ep_capture_row = cap_r;
    m.ep_capture_col = cap_c;
    out.push_back(m);

    return out;
}

std::vector<ChessMove> Chess::addCastlingMovesIfAny(int king_row, int king_col, ChessColor side) const
{
    std::vector<ChessMove> out;

    const auto king = board[king_row][king_col].getBoardPiece();
    if (!king || king->color() != side || king->type() != ChessPieceType::King) return out;

    const int home_row = (side == ChessColor::White) ? 7 : 0;
    if (king_row != home_row || king_col != 4) return out;

    if (isInCheck(side)) return out;

    const bool can_k = (side == ChessColor::White) ? castling_rights_.white_kingside : castling_rights_.black_kingside;
    const bool can_q = (side == ChessColor::White) ? castling_rights_.white_queenside : castling_rights_.black_queenside;

    if (can_k)
    {
        const auto rook = board[home_row][7].getBoardPiece();
        if (rook && rook->color() == side && rook->type() == ChessPieceType::Rook &&
            !board[home_row][5].getBoardPiece() &&
            !board[home_row][6].getBoardPiece() &&
            !isSquareAttacked(home_row, 5, opposite(side)) &&
            !isSquareAttacked(home_row, 6, opposite(side)))
        {
            ChessMove m{};
            m.from_row = home_row;
            m.from_col = 4;
            m.to_row = home_row;
            m.to_col = 6;
            m.special = ChessSpecialMove::CastleKingside;
            m.rook_from_row = home_row;
            m.rook_from_col = 7;
            m.rook_to_row = home_row;
            m.rook_to_col = 5;
            out.push_back(m);
        }
    }

    if (can_q)
    {
        const auto rook = board[home_row][0].getBoardPiece();
        if (rook && rook->color() == side && rook->type() == ChessPieceType::Rook &&
            !board[home_row][1].getBoardPiece() &&
            !board[home_row][2].getBoardPiece() &&
            !board[home_row][3].getBoardPiece() &&
            !isSquareAttacked(home_row, 3, opposite(side)) &&
            !isSquareAttacked(home_row, 2, opposite(side)))
        {
            ChessMove m{};
            m.from_row = home_row;
            m.from_col = 4;
            m.to_row = home_row;
            m.to_col = 2;
            m.special = ChessSpecialMove::CastleQueenside;
            m.rook_from_row = home_row;
            m.rook_from_col = 0;
            m.rook_to_row = home_row;
            m.rook_to_col = 3;
            out.push_back(m);
        }
    }

    return out;
}

// -------------------- Legal move generation --------------------

std::vector<ChessMove> Chess::legalMovesFrom(int from_row, int from_col, ChessColor side) const
{
    std::vector<ChessMove> legal;

    if (!inBounds(from_row, from_col)) return legal;

    const auto piece = board[from_row][from_col].getBoardPiece();
    if (!piece || piece->color() != side) return legal;

    std::vector<ChessMove> candidates;

    const auto pseudo = piece->pseudoLegalMoves(board, from_row, from_col);
    for (const auto& m : pseudo)
    {
        const auto expanded = expandPromotionChoicesIfNeeded(m, side);
        candidates.insert(candidates.end(), expanded.begin(), expanded.end());
    }

    if (piece->type() == ChessPieceType::Pawn)
    {
        const auto eps = addEnPassantMovesIfAny(from_row, from_col, side);
        candidates.insert(candidates.end(), eps.begin(), eps.end());
    }
    if (piece->type() == ChessPieceType::King)
    {
        const auto castles = addCastlingMovesIfAny(from_row, from_col, side);
        candidates.insert(candidates.end(), castles.begin(), castles.end());
    }

    for (const auto& m : candidates)
    {
        if (m.from_row != from_row || m.from_col != from_col) continue;
        if (m.special == ChessSpecialMove::Promotion && m.promotion == ChessPromotion::None) continue;

        Board copy = board;
        applyMoveUnchecked(copy, m, side);

        const auto king_sq = findKingSquare(copy, side);
        if (!king_sq.has_value()) continue;

        const auto [kr, kc] = king_sq.value();

        auto attacked = [&](int row, int col, ChessColor by_side) -> bool {
            for (int r = 0; r < kN; ++r)
            {
                for (int c = 0; c < kN; ++c)
                {
                    const auto p = copy[r][c].getBoardPiece();
                    if (!p || p->color() != by_side) continue;

                    if (p->type() == ChessPieceType::Pawn)
                    {
                        if (isPawnAttackSquare(r, c, by_side, row, col)) return true;
                        continue;
                    }

                    const auto pm = p->pseudoLegalMoves(copy, r, c);
                    for (const auto& mm : pm)
                        if (mm.to_row == row && mm.to_col == col) return true;
                }
            }
            return false;
        };

        if (!attacked(kr, kc, opposite(side)))
            legal.push_back(m);
    }

    return legal;
}

std::vector<ChessMove> Chess::allLegalMoves(ChessColor side) const
{
    std::vector<ChessMove> out;

    for (int r = 0; r < kN; ++r)
    {
        for (int c = 0; c < kN; ++c)
        {
            const auto p = board[r][c].getBoardPiece();
            if (!p || p->color() != side) continue;

            auto ms = legalMovesFrom(r, c, side);
            out.insert(out.end(), ms.begin(), ms.end());
        }
    }
    return out;
}

int Chess::legalMoveCount(ChessColor side) const
{
    return static_cast<int>(allLegalMoves(side).size());
}

bool Chess::isMoveLegal(const ChessMove& move, ChessColor side) const
{
    if (!inBounds(move.from_row, move.from_col) || !inBounds(move.to_row, move.to_col)) return false;

    const auto legal = legalMovesFrom(move.from_row, move.from_col, side);
    for (const auto& m : legal)
    {
        if (m.from_row != move.from_row || m.from_col != move.from_col) continue;
        if (m.to_row != move.to_row || m.to_col != move.to_col) continue;
        if (m.special != move.special) continue;

        if (m.special == ChessSpecialMove::Promotion && m.promotion != move.promotion) continue;

        if (m.special == ChessSpecialMove::EnPassant &&
            (m.ep_capture_row != move.ep_capture_row || m.ep_capture_col != move.ep_capture_col))
            continue;

        if ((m.special == ChessSpecialMove::CastleKingside || m.special == ChessSpecialMove::CastleQueenside) &&
            (m.rook_from_row != move.rook_from_row || m.rook_from_col != move.rook_from_col ||
             m.rook_to_row != move.rook_to_row || m.rook_to_col != move.rook_to_col))
            continue;

        return true;
    }
    return false;
}

// -------------------- Move application --------------------

Chess::PiecePtr Chess::makePromotionPiece(ChessColor side, ChessPromotion promo) const
{
    switch (promo)
    {
        case ChessPromotion::Queen:  return std::make_shared<Queen>(side);
        case ChessPromotion::Rook:   return std::make_shared<Rook>(side);
        case ChessPromotion::Bishop: return std::make_shared<Bishop>(side);
        case ChessPromotion::Knight: return std::make_shared<Knight>(side);
        case ChessPromotion::None:   break;
    }
    return nullptr;
}

void Chess::applyMoveUnchecked(Board& b, const ChessMove& move, ChessColor side) const
{
    auto moved = b[move.from_row][move.from_col].getBoardPiece();
    setSquare(b, move.from_row, move.from_col, nullptr);

    if (move.special == ChessSpecialMove::EnPassant)
    {
        setSquare(b, move.ep_capture_row, move.ep_capture_col, nullptr);
        setSquare(b, move.to_row, move.to_col, std::move(moved));
        return;
    }

    if (move.special == ChessSpecialMove::CastleKingside || move.special == ChessSpecialMove::CastleQueenside)
    {
        auto rook = b[move.rook_from_row][move.rook_from_col].getBoardPiece();
        setSquare(b, move.to_row, move.to_col, std::move(moved));
        setSquare(b, move.rook_from_row, move.rook_from_col, nullptr);
        setSquare(b, move.rook_to_row, move.rook_to_col, std::move(rook));
        return;
    }

    if (move.special == ChessSpecialMove::Promotion)
    {
        auto promoted = makePromotionPiece(side, move.promotion);
        setSquare(b, move.to_row, move.to_col, std::move(promoted));
        return;
    }

    setSquare(b, move.to_row, move.to_col, std::move(moved));
}

void Chess::updateCastlingRightsAfterMove(const ChessMove& move, ChessColor side)
{
    const int home_row = (side == ChessColor::White) ? 7 : 0;

    const auto moved_piece = board[move.from_row][move.from_col].getBoardPiece();
    const auto captured_piece = board[move.to_row][move.to_col].getBoardPiece();

    if (moved_piece && moved_piece->type() == ChessPieceType::King)
    {
        if (side == ChessColor::White)
        {
            castling_rights_.white_kingside = false;
            castling_rights_.white_queenside = false;
        }
        else
        {
            castling_rights_.black_kingside = false;
            castling_rights_.black_queenside = false;
        }
    }

    if (moved_piece && moved_piece->type() == ChessPieceType::Rook)
    {
        if (move.from_row == home_row && move.from_col == 0)
        {
            if (side == ChessColor::White) castling_rights_.white_queenside = false;
            else castling_rights_.black_queenside = false;
        }
        if (move.from_row == home_row && move.from_col == 7)
        {
            if (side == ChessColor::White) castling_rights_.white_kingside = false;
            else castling_rights_.black_kingside = false;
        }
    }

    if (captured_piece && captured_piece->type() == ChessPieceType::Rook)
    {
        const ChessColor opp = opposite(side);
        const int opp_home = (opp == ChessColor::White) ? 7 : 0;

        if (move.to_row == opp_home && move.to_col == 0)
        {
            if (opp == ChessColor::White) castling_rights_.white_queenside = false;
            else castling_rights_.black_queenside = false;
        }
        if (move.to_row == opp_home && move.to_col == 7)
        {
            if (opp == ChessColor::White) castling_rights_.white_kingside = false;
            else castling_rights_.black_kingside = false;
        }
    }

    if (move.special == ChessSpecialMove::CastleKingside || move.special == ChessSpecialMove::CastleQueenside)
    {
        if (side == ChessColor::White)
        {
            castling_rights_.white_kingside = false;
            castling_rights_.white_queenside = false;
        }
        else
        {
            castling_rights_.black_kingside = false;
            castling_rights_.black_queenside = false;
        }
    }
}

void Chess::updateEnPassantAfterMove(const ChessMove& move, const PiecePtr& moved_piece)
{
    clearEnPassantTarget();

    if (!moved_piece || moved_piece->type() != ChessPieceType::Pawn) return;

    const int dr = move.to_row - move.from_row;
    if (std::abs(dr) != 2) return;

    const int dir = (moved_piece->color() == ChessColor::White) ? -1 : +1;
    const int target_r = move.from_row + dir;
    const int target_c = move.from_col;

    if (inBounds(target_r, target_c))
        setEnPassantTarget(target_r, target_c);
}

bool Chess::applyMove(const ChessMove& move)
{
    const ChessColor side = side_to_move_;

    if (move.special == ChessSpecialMove::Promotion && move.promotion == ChessPromotion::None)
        return false;

    if (!isMoveLegal(move, side)) return false;

    const auto moved_piece = board[move.from_row][move.from_col].getBoardPiece();
    if (!moved_piece) return false;

    bool is_capture = false;
    if (move.special == ChessSpecialMove::EnPassant)
    {
        is_capture = true;
    }
    else
    {
        is_capture = (board[move.to_row][move.to_col].getBoardPiece() != nullptr);
    }

    if (moved_piece->type() == ChessPieceType::Pawn || is_capture)
        halfmove_clock_ = 0;
    else
        ++halfmove_clock_;

    updateCastlingRightsAfterMove(move, side);
    updateEnPassantAfterMove(move, moved_piece);

    applyMoveUnchecked(board, move, side);

    side_to_move_ = opposite(side_to_move_);

    position_hash_ = computeHash(board, side_to_move_, castling_rights_, has_en_passant_, en_passant_row_, en_passant_col_);
    position_counts_[position_hash_] += 1;

    return true;
}

// -------------------- Checkmate / stalemate --------------------

bool Chess::isCheckmate(ChessColor side) const
{
    if (!isInCheck(side)) return false;
    return allLegalMoves(side).empty();
}

bool Chess::isStalemate(ChessColor side) const
{
    if (isInCheck(side)) return false;
    return allLegalMoves(side).empty();
}

// -------------------- Draw rules --------------------

static bool insufficientMaterial(const Chess::Board& b)
{
    int white_minor = 0;
    int black_minor = 0;
    int white_bishops = 0, black_bishops = 0;
    int white_knights = 0, black_knights = 0;

    int white_rooks = 0, black_rooks = 0;
    int white_queens = 0, black_queens = 0;
    int white_pawns = 0, black_pawns = 0;

    std::optional<int> white_bishop_color;
    std::optional<int> black_bishop_color;

    for (int r = 0; r < 8; ++r)
    {
        for (int c = 0; c < 8; ++c)
        {
            const auto p = b[r][c].getBoardPiece();
            if (!p) continue;

            const bool w = (p->color() == ChessColor::White);
            switch (p->type())
            {
                case ChessPieceType::Pawn:   w ? ++white_pawns : ++black_pawns; break;
                case ChessPieceType::Rook:   w ? ++white_rooks : ++black_rooks; break;
                case ChessPieceType::Queen:  w ? ++white_queens : ++black_queens; break;
                case ChessPieceType::Bishop:
                    w ? ++white_bishops : ++black_bishops;
                    if (w) white_bishop_color = squareColor(r, c);
                    else black_bishop_color = squareColor(r, c);
                    break;
                case ChessPieceType::Knight:
                    w ? ++white_knights : ++black_knights;
                    break;
                case ChessPieceType::King:
                    break;
            }
        }
    }

    white_minor = white_bishops + white_knights;
    black_minor = black_bishops + black_knights;

    if (white_pawns || black_pawns) return false;
    if (white_rooks || black_rooks) return false;
    if (white_queens || black_queens) return false;

    if (white_minor == 0 && black_minor == 0) return true;

    if (white_minor == 1 && black_minor == 0) return true;
    if (white_minor == 0 && black_minor == 1) return true;

    if (white_knights == 2 && white_bishops == 0 && black_minor == 0) return true;
    if (black_knights == 2 && black_bishops == 0 && white_minor == 0) return true;

    if (white_bishops == 1 && white_knights == 0 &&
        black_bishops == 1 && black_knights == 0 &&
        white_bishop_color.has_value() && black_bishop_color.has_value() &&
        white_bishop_color.value() == black_bishop_color.value())
    {
        return true;
    }

    return false;
}

Chess::DrawInfo Chess::getDrawInfo() const
{
    if (halfmove_clock_ >= 100)
        return DrawInfo{true, "50-move rule"};

    auto it = position_counts_.find(position_hash_);
    if (it != position_counts_.end() && it->second >= 3)
        return DrawInfo{true, "threefold repetition"};

    if (insufficientMaterial(board))
        return DrawInfo{true, "insufficient material"};

    return DrawInfo{false, ""};
}

Chess::GameResult Chess::getResult() const
{
    const auto draw = getDrawInfo();
    if (draw.is_draw) return GameResult::Draw;

    const ChessColor side = side_to_move_;
    if (isCheckmate(side))
        return (side == ChessColor::White) ? GameResult::BlackWin : GameResult::WhiteWin;

    if (isStalemate(side))
        return GameResult::Draw;

    return GameResult::Ongoing;
}

// -------------------- Move parsing / formatting (UI glue) --------------------

static bool parseSquare(const std::string& s, std::size_t i, int& out_r, int& out_c)
{
    if (i + 1 >= s.size()) return false;
    const char file = s[i];
    const char rank = s[i + 1];

    if (file < 'a' || file > 'h') return false;
    if (rank < '1' || rank > '8') return false;

    out_c = file - 'a';
    const int rank_num = rank - '0';
    out_r = 8 - rank_num;
    return true;
}

static ChessPromotion promoFromChar(char c)
{
    switch (c)
    {
        case 'q': return ChessPromotion::Queen;
        case 'r': return ChessPromotion::Rook;
        case 'b': return ChessPromotion::Bishop;
        case 'n': return ChessPromotion::Knight;
        default:  return ChessPromotion::None;
    }
}

std::optional<ChessMove> Chess::parseMoveUci(const std::string& uci, ChessColor side) const
{
    if (uci.size() < 4) return std::nullopt;

    int fr{}, fc{}, tr{}, tc{};
    if (!parseSquare(uci, 0, fr, fc)) return std::nullopt;
    if (!parseSquare(uci, 2, tr, tc)) return std::nullopt;

    ChessPromotion promo = ChessPromotion::None;
    if (uci.size() >= 5)
    {
        const unsigned char ch = static_cast<unsigned char>(uci[4]);
        promo = promoFromChar(static_cast<char>(std::tolower(ch)));
    }

    const auto legal = legalMovesFrom(fr, fc, side);
    for (auto m : legal)
    {
        if (m.to_row != tr || m.to_col != tc) continue;

        if (m.special == ChessSpecialMove::Promotion)
        {
            if (promo == ChessPromotion::None) continue;
            if (m.promotion != promo) continue;
        }
        return m;
    }

    return std::nullopt;
}

static std::string squareToUci(int r, int c)
{
    const char file = static_cast<char>('a' + c);
    const char rank = static_cast<char>('8' - r);
    std::string s;
    s.push_back(file);
    s.push_back(rank);
    return s;
}

std::string Chess::formatMoveUci(const ChessMove& move) const
{
    std::string s = squareToUci(move.from_row, move.from_col) + squareToUci(move.to_row, move.to_col);

    if (move.special == ChessSpecialMove::Promotion)
    {
        char pc = 'q';
        switch (move.promotion)
        {
            case ChessPromotion::Queen:  pc = 'q'; break;
            case ChessPromotion::Rook:   pc = 'r'; break;
            case ChessPromotion::Bishop: pc = 'b'; break;
            case ChessPromotion::Knight: pc = 'n'; break;
            case ChessPromotion::None:   pc = 'q'; break;
        }
        s.push_back(pc);
    }

    return s;
}