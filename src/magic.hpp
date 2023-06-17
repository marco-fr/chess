#ifndef MAGIC_INCLUDE
#define MAGIC_INCLUDE

#include "bitboard.hpp"

class Magic
{
public:
    static const U64 bishop_mask[64];
    static const U64 rook_mask[64];
    static const U64 bishop_magics[64];
    static const U64 rook_magics[64];
    static U64 bishop_hash[64][4096];
    static U64 rook_hash[64][4096];
    static int num_of_ones(U64 x);
    static U64 random_U64();
    static U64 random_U64_fewbits();
    static int pop_first_bit(U64* b);
    static U64 generate_occupancy(int index, int bits, U64 mask);
    static U64 hash_magic(U64 board, U64 magic, int bits);
    static U64 rook_attacks(int sq, U64 block);
    static U64 bishop_attacks(int sq, U64 block);
    static void init_mask_and_magics();
    static void generate_bishop_magics();
    static void generate_rook_magics();
    static void init_bishop_hash();
    static void init_rook_hash();
    static void init_hashes();
    static U64 find_bishop_moves(U64 sq, U64 all_pieces_mask);
    static U64 find_rook_moves(U64 sq, U64 all_pieces_mask);
    static U64 generate_square_magic(int square, int bishop);
};

#endif