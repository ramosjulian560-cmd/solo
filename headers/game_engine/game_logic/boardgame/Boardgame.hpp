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


// headers/game_engine/game_logic/boardgame/chess/chess.hpp
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../Boardgame.hpp"

// IMPORTANT: Keep this name/casing consistent with your actual chesspiece.hpp file.
class ChessPiece;

class Chess : public Boardgame<std::shared_ptr<ChessPiece>>
{
public:
    Chess()
        : Boardgame<std::shared_ptr<ChessPiece>>(
              "Chess",
              "E",
              "Boardgame",
              "Classic chess with full rules.",
              std::vector<std::string>{"Board", "Strategy"},
              0
          )
    {}

    ~Chess() override = default;

protected:
    void setBoard() override;
};
