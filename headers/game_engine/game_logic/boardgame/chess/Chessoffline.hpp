// headers/game_engine/game_logic/boardgame/chess/Chessoffline.hpp
#pragma once

#include "Chess.hpp"

/**
 * Thin offline/local wrapper for Chess.
 *
 * Keeps core rules in Chess; offline flow (CLI/local match loop) can be added later
 * without mixing UI/network code into the rules engine.
 */
class Chessoffline final : public Chess
{
public:
    Chessoffline() = default;
    ~Chessoffline() override = default;
    Chessoffline(const Chessoffline&) = delete;
    Chessoffline& operator=(const Chessoffline&) = delete;

    Chessoffline(Chessoffline&&) noexcept = default;
    Chessoffline& operator=(Chessoffline&&) noexcept = default;
};