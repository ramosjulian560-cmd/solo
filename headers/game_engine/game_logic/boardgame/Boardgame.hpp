#pragma once

#include <string>
#include <utility>
#include <vector>

#include "game_engine/Game.hpp"
#include "BoardPosition.hpp"

template <typename BoardPiece>
class Boardgame : public Game
{
protected:
    std::vector<std::vector<BoardPosition<BoardPiece>>> board;

public:
    Boardgame(
        std::string name,
        std::string rating,
        std::string type,
        std::string description,
        std::vector<std::string> genres,
        int price
    )
        : Game(
              std::move(name),
              std::move(rating),
              std::move(type),
              std::move(description),
              std::move(genres),
              price
          )
    {}

protected:
    virtual void setBoard() = 0;

public:
    virtual std::vector<std::vector<BoardPosition<BoardPiece>>> getBoard() const final
    {
        return board; // returns a copy (OK for now; keep as-is if engine expects it)
    }
};
