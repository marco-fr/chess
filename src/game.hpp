#ifndef GAME_INCLUDE
#define GAME_INCLUDE

#include "alphabeta.hpp"
#include "bitboard.hpp"
#include "moves.hpp"

class Game
{
public:
    void start_xboard();
    void start_commandline();
    Game(int depth);

private:
    Bitboard* board;
    Move* move;
    Alphabeta* alg;
    int color;
};

#endif
