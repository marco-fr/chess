#ifndef ZHASH_INCLUDE
#define ZHASH_INCLUDE

#include "bitboard.hpp"
#define U64 uint64_t

class Bitboard;

class ZHash
{
public:
    struct entry;
    void init_z_hash();
    void allocate_table(int mb);
    void clear_table();
    void set_table(int num);
    void delete_table();
    void set_table_index(U64 key, int value, Bitboard* b, int depth);
    void output_size();
    void reset_times_accessed();
    int access_table(U64 key);
    int is_empty(U64 key, Bitboard* b, int depth);
    int not_good_for_deepening(U64 key, Bitboard *b, int depth);
    U64 modify_hash_U64(U64 old, int piece, U64 square);
    U64 hash_board_U64(Bitboard* b, int color);
    U64 modify_hash(U64 old, int piece, int square);
    U64 resized_key(U64 key);
    U64 get_times_accessed();
    ZHash(int size_mb);
    ~ZHash();

private:
    const int DEFAULT_VALUE = -1e8;
    entry* hash_table;
    U64 times_accessed;
    U64 bit_table[64][12];
    U64 depth_table[20];
    U64 black_move;
    U64 castling_flags[2][2];
    U64 en_p_flags[2];
    U64 number_entries;
    U64 mask;
    void set_mask();
};
#endif