#ifndef XBOARD_INCLUDE
#define XBOARD_INCLUDE
#include "alphabeta.hpp"
#include "bitboard.hpp"

class XBoard
{
public:
    void loop();
    void undo_move();
    Alphabeta::eval_move xboard_move_to_move(std::string s, int color);
    std::string square_to_xboard_move(Alphabeta::eval_move result);
    XBoard(Alphabeta* a, Bitboard* board);
    std::vector<Alphabeta::eval_move> game_tree;

private:
    Alphabeta* alg;
    Bitboard* board;
    int color = WHITE;
};
#endif