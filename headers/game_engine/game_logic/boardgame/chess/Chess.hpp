#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../Boardgame.hpp"

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
              0)
    {}

    ~Chess() override = default;

protected:
    void setBoard() override;
};
