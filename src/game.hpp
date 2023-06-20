#ifndef GAME_INCLUDE
#define GAME_INCLUDE

#include "alphabeta.hpp"
#include "bitboard.hpp"
#include "moves.hpp"
#include <vector>

class Game
{
public:
    void start_xboard();
    void start_commandline();
    void test();
    Game(int depth, int quise);

private:
    Bitboard* board;
    Move* move;
    Alphabeta* alg;
    int color;
};

#endif
