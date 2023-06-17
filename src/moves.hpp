#ifndef MOVES_INCLUDE
#define MOVES_INCLUDE

#include "bitboard.hpp"
#include "hash.hpp"
#include "magic.hpp"
#include <functional>

class Move
{
public:
    Bitboard* curBoard;
    U64 FILE_A, FILE_AB, FILE_H, FILE_GH, RANK_5, RANK_4, RANK_2, RANK_7,
        RANK_1, RANK_8;
    U64 KING_CASTLING[2];
    U64 QUEEN_CASTLING[2];
    void generate_file_rank();
    void update_colors();

    U64 find_king_attacks(U64 square, int color);
    U64 find_knight_attacks(U64 square, int color);
    U64 find_pawn_attacks(U64 square, int color);
    U64 find_rook_attacks(U64 square, int color);
    U64 find_bishop_attacks(U64 square, int color);
    U64 find_queen_attacks(U64 square, int color);
    U64 find_all_attacks_from_piece(U64 piece, int color,
                                    U64 (Move::*function)(U64, int));
    U64 find_all_attacks_from_pawn(U64 piece, int color);
    U64 find_all_attacks(int color);

    U64 find_king_legal_moves(U64 square, int color);
    U64 find_knight_legal_moves(U64 square, int color);
    U64 find_pawn_legal_moves(U64 square, int color);
    U64 find_rook_legal_moves(U64 square, int color);
    U64 find_bishop_legal_moves(U64 square, int color);
    U64 find_queen_legal_moves(U64 square, int color);
    U64 remove_color(U64 board, int color);
    int check_check(int color);

    Move(Bitboard* b);

private:
    U64(Move::*function_calls[6])
    (U64, int) = {&Move::find_pawn_attacks,   &Move::find_knight_attacks,
                  &Move::find_bishop_attacks, &Move::find_rook_attacks,
                  &Move::find_queen_attacks,  &Move::find_king_attacks};
};
#endif