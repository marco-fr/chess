#include "hash.hpp"
#include "bitboard.hpp"
#include "magic.hpp"

struct ZHash::entry
{
    int score, depth = 0, color;
    U64 white = 0ULL, black = 0ULL;
};

void ZHash::init_z_hash()
{
    for (int i = 0; i < 64; i++)
    {
        for (int j = 0; j < 12; j++)
        {
            bit_table[i][j] = Magic::random_U64();
        }
    }
    black_move = Magic::random_U64();
    for (auto& i : depth_table)
        i = Magic::random_U64();
}

U64 ZHash::modify_hash(U64 old, int piece, int square)
{
    if (!square)
        return old;
    return old ^ (bit_table[square][piece]);
}

U64 ZHash::modify_hash_U64(U64 old, int piece, U64 square)
{
    if (!square)
        return old;
    int place = Magic::pop_first_bit(&square);
    // std::cout  << "Place: " << place << std::endl;
    // std::cout << "Piece: " << piece << std::endl;
    return old ^ (bit_table[place][piece]);
}

U64 ZHash::hash_board_U64(Bitboard* b, int color)
{
    U64 result = 0ULL;
    int index;
    U64 copy;
    if (color)
        result ^= black_move;
    for (int i = 0; i < 12; i++)
    {
        copy = b->board[i];
        while (copy != 0ULL)
        {
            index = Magic::pop_first_bit(&copy);
            result ^= bit_table[index][i];
        }
    }
    // result ^= depth_table[depth];
    return result;
}

void ZHash::set_table_index(U64 key, int value, Bitboard* b, int depth)
{
    U64 new_key = resized_key(key);
    hash_table[new_key].score = value;
    hash_table[new_key].depth = depth;
    hash_table[new_key].white = b->board[W_PIECES];
    hash_table[new_key].black = b->board[B_PIECES];
    hash_table[new_key].color = b->turn;
}

int ZHash::access_table(U64 key)
{
    U64 new_key = resized_key(key);
    times_accessed++;
    return hash_table[new_key].score;
}

U64 ZHash::get_times_accessed() { return times_accessed; }

void ZHash::reset_times_accessed() { times_accessed = 0ULL; }

int ZHash::is_empty(U64 key, Bitboard* b, int depth)
{
    U64 new_key = resized_key(key);

    if (b->board[W_PIECES] != hash_table[new_key].white)
        return 1;
    if (b->board[B_PIECES] != hash_table[new_key].black)
        return 1;
    if (b->turn != hash_table[new_key].color)
        return 1;
    if (depth > hash_table[new_key].depth)
        return 1;
    return 0;
}

int ZHash::not_good_for_deepening(U64 key, Bitboard* b, int depth)
{
    U64 new_key = resized_key(key);

    if (b->board[W_PIECES] != hash_table[new_key].white)
        return 1;
    if (b->board[B_PIECES] != hash_table[new_key].black)
        return 1;
    if (b->turn != hash_table[new_key].color)
        return 1;
    if (depth > hash_table[new_key].depth + 1)
        return 1;
    return 0;
}

U64 ZHash::resized_key(U64 key) { return key & mask; }

void ZHash::delete_table() { delete[] hash_table; }

void ZHash::clear_table()
{
    for (int i = 0; i < number_entries; i++)
    {
        hash_table[i].score = 0ULL;
    }
}

void ZHash::set_table(int num)
{
    for (int i = 0; i < number_entries; i++)
    {
        hash_table[i].score = num;
    }
}

void ZHash::output_size()
{
    std::cout << "Total hash entries: " << number_entries << std::endl;
    std::cout << "Total hash size (bytes): " << number_entries * sizeof(entry)
              << std::endl;
}

void ZHash::allocate_table(int mb)
{
    U64 size_byte = mb * 1024 * 1024;
    std::size_t entry_size = sizeof(entry);
    U64 indices = size_byte / entry_size;
    delete[] hash_table;
    try
    {
        hash_table = new entry[indices];
        number_entries = indices;
    }
    catch (std::bad_alloc&)
    {
        std::cout << "Failed to allocate hash table." << std::endl;
        exit(EXIT_FAILURE);
    }
}

void ZHash::set_mask()
{
    int leading = __builtin_clzll((U64)number_entries);
    int back = 64 - leading - 1;
    mask = 0ULL;
    for (int i = 0; i < back; i++)
    {
        mask |= (1ULL << i);
    }
}

ZHash::ZHash(int size_mb)
{
    number_entries = 0;
    init_z_hash();
    allocate_table(size_mb);
    set_table(DEFAULT_VALUE);
    set_mask();
}
ZHash::~ZHash() { delete_table(); }