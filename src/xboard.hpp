#ifndef XBOARD_INCLUDE
#define XBOARD_INCLUDE
#include "alphabeta.hpp"
#include "bitboard.hpp"

class XBoard
{
public:
    void loop();
    void xboard_move_to_move(std::string s, int color);
    std::string square_to_xboard_move(Alphabeta::eval_move result);
    XBoard(Alphabeta* a, Bitboard* board);

private:
    Alphabeta* alg;
    Bitboard* board;
};
#endif