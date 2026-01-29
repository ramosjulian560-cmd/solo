// headers/game_engine/game_logic/boardgame/BoardPosition.hpp
#pragma once


template <typename BoardPiece>
struct BoardPosition
{
    friend class PlotFour;


private:
    BoardPiece board_piece;
    int row, col;


public:
    BoardPosition(int row, int col, BoardPiece board_piece)
        : board_piece(board_piece), row(row), col(col)
    {}


    BoardPiece getBoardPiece() const { return board_piece; }
    int getRow() const { return row; }
    int getCol() const { return col; }


private:
    void setBoardPiece(BoardPiece board_piece) { this->board_piece = board_piece; }
};