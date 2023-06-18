#ifndef ALPHABETA_INCLUDE
#define ALPHABETA_INCLUDE
#include "evaltables.hpp"
#include "moves.hpp"
#include <queue>

class Compare;

class Alphabeta
{
public:
    struct eval_move;
    struct queue_item;
    int eval_piece(int piece);
    int eval_board(int color);
    Alphabeta::eval_move start_alphabeta(int color);
    int alphabeta(int depth, int color, int alpha, int beta);
    int quiescence(int depth, int color, int alpha, int beta);
    Alphabeta::eval_move root_alphabeta(int depth, int color, int alpha,
                                        int beta);
    Alphabeta::eval_move pieces_in_bitboard(
        U64* piece, int color, U64 (Move::*function)(U64, int), int depth,
        std::priority_queue<queue_item, std::vector<queue_item>, Compare>& list,
        int only_attacking);
    void move_alphabeta_eval(Alphabeta::eval_move result, int color);
    void engine_make_move(Alphabeta::queue_item next, int color);
    void engine_remove_move(Alphabeta::queue_item next, int color, U64 key,
                            Bitboard::Flags& fl_copy);
    void get_all_moves(
        int color, int depth,
        std::priority_queue<queue_item, std::vector<queue_item>, Compare>& list,
        int attacking);
    Alphabeta(Move* move, int depth);

private:
    Move* mo;
    Bitboard* curBoard;
    int depth;
    const int MAX = 1e9;
    int max_quise = -4;
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

struct Alphabeta::queue_item
{
    int score = 0;
    U64 from = 0ULL, to = 0ULL;
    int promote = -1;
    int type, prev_piece;
    U64 prev_board;
    int is_promoting;
};

#endif