// headers/game_engine/game_logic/boardgame/Chess/ChessOffline.hpp
#pragma once

#include "Chess.hpp"
#include <cstdint>
#include <string>
#include <utility>

class ChessOffline final : public Chess
{
public:
    enum class Difficulty
    {
        Easy,
        Medium,
        Hard
    };

    using RatingRange = std::pair<int, int>;

    ChessOffline() = default;

    explicit ChessOffline(Difficulty difficulty) noexcept
        : difficulty_(difficulty)
    {}

    ~ChessOffline() = default;

    ChessOffline(const ChessOffline&) = delete;
    ChessOffline& operator=(const ChessOffline&) = delete;
    ChessOffline(ChessOffline&&) noexcept = default;
    ChessOffline& operator=(ChessOffline&&) noexcept = default;

    void setDifficulty(Difficulty difficulty) noexcept { difficulty_ = difficulty; }
    Difficulty getDifficulty() const noexcept { return difficulty_; }

    RatingRange ratingRange() const noexcept
    {
        switch (difficulty_)
        {
            case Difficulty::Easy:   return {600, 900};
            case Difficulty::Medium: return {1100, 1500};
            case Difficulty::Hard:   return {1700, 3200};
        }
        return {1100, 1500};
    }

    static std::string toString(Difficulty d)
    {
        switch (d)
        {
            case Difficulty::Easy:   return "easy";
            case Difficulty::Medium: return "medium";
            case Difficulty::Hard:   return "hard";
        }
        return "medium";
    }

    /**
     * Optional: deterministically pick a rating within ratingRange().
     * Later your AI can use this to decide depth/time/randomness.
     */
    int pickRating(std::uint32_t seed = 0) const noexcept
    {
        const auto [lo, hi] = ratingRange();
        if (hi <= lo) return lo;

        const std::uint32_t span = static_cast<std::uint32_t>(hi - lo + 1);
        return lo + static_cast<int>(seed % span);
    }

private:
    Difficulty difficulty_ = Difficulty::Medium;
};
