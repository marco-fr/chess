#ifndef ALPHABETA_INCLUDE
#define ALPHABETA_INCLUDE
#include "evaltables.hpp"
#include "moves.hpp"

class Alphabeta
{
public:
    struct eval_move;
    int eval_piece(int piece);
    int eval_board();
    Alphabeta::eval_move start_alphabeta(int color);
    Alphabeta::eval_move alphabeta(int depth, int color, int alpha, int beta);
    Alphabeta::eval_move pieces_in_bitboard(U64* piece, int color,
                                            U64 (Move::*function)(U64, int),
                                            int depth, int alpha, int beta);
    void move_alphabeta_eval(Alphabeta::eval_move result, int color);
    void comp_moves(int color, Alphabeta::eval_move* b, Alphabeta::eval_move e);
    Alphabeta(Move* move, int depth);

private:
    const int HASH_SIZE_MB = 32;
    Move* mo;
    Bitboard* curBoard;
    ZHash* hash;
    int depth;
    U64 cur_hash;
    U64(Move::*function_calls[6])
    (U64,
     int) = {&Move::find_pawn_legal_moves,   &Move::find_knight_legal_moves,
             &Move::find_bishop_legal_moves, &Move::find_rook_legal_moves,
             &Move::find_queen_legal_moves,  &Move::find_king_legal_moves};
};

struct Alphabeta::eval_move
{
    int score = -1e9;
    U64 from = 0ULL, to = 0ULL;
    int promote = -1;
};

#endif