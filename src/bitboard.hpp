#ifndef BITBOARD_INCLUDE
#define BITBOARD_INCLUDE

#include <iostream>
#include "hash.hpp"

#define set_bit(b, i) ((b) |= (1ULL << i))
#define get_bit(b, i) ((b) & (1ULL << i))
#define clear_bit(b, i) ((b) &= ~(1ULL << i))
#define get_LSB(b) (__builtin_ctzll(b))

#define U64 uint64_t
enum
{
    W_PAWN,
    W_KNIGHT,
    W_BISHOP,
    W_ROOK,
    W_QUEEN,
    W_KING,
    B_PAWN,
    B_KNIGHT,
    B_BISHOP,
    B_ROOK,
    B_QUEEN,
    B_KING,
    W_PIECES,
    B_PIECES,
    ALL_PIECES,
    W_PAWN_ATTACK,
    W_KNIGHT_ATTACK,
    W_BISHOP_ATTACK,
    W_ROOK_ATTACK,
    W_QUEEN_ATTACK,
    W_KING_ATTACK,
    B_PAWN_ATTACK,
    B_KNIGHT_ATTACK,
    B_BISHOP_ATTACK,
    B_ROOK_ATTACK,
    B_QUEEN_ATTACK,
    B_KING_ATTACK,
    W_ALL_ATTACKS,
    B_ALL_ATTACKS
};

enum
{
    WHITE,
    BLACK
};

class ZHash;

class Bitboard
{
public:
    struct Flags;
    static void print_U64(U64 b);

    void print_board();
    void print_flags(Flags f);
    void set_board(std::string FEN);
    void copy_flags(Flags from, Flags& to);
    void reset();
    void input_move(int color);
    void make_move(U64 from, U64 to, int color, int piece);
    void make_easy_move(U64 form, U64 to, int color);
    void remove_square(U64 square);
    void promote(U64 from, int color, int piece_to);
    void unpromote(U64 from, int color, int piece_to);
    char char_from_piece(int piece);

    U64 find_white_pieces();
    U64 find_black_pieces();
    U64 find_all_pieces();
    int find_piece_index(U64 piece, int color);
    int piece_from_character(char c);
    int is_promoting_pawn(U64 square, int piece);
    void update_color_boards();
    void add_piece_with_hash(U64 square, int piece);
    void remove_all_piece_square_with_hash(U64 square);
    void remove_piece_with_hash(U64 square, int piece);

    U64 board[29];
    U64 hash_key;
    Flags* fl;

    ZHash *hash;

    U64 RANK_5, RANK_4, RANK_8, RANK_1;

    int turn_number = 1;
    int running = 1;
    int endgame = 0;
    int endgame_turning = 100;
    int turn = WHITE;
    char piece_naming[12] = {'P', 'N', 'B', 'R', 'Q', 'K',
                             'p', 'n', 'b', 'r', 'q', 'k'};

    Bitboard(int color);
    ~Bitboard();

private:
    int HASH_MB_SIZE = 128;
    // White: Pawn, Knight, Bishop, Rook, Queen, King...
};

struct Bitboard::Flags
{
    U64 ep[2] = {0, 0};
    int king_castling[2] = {1, 1};
    int queen_castling[2] = {1, 1};
};
#endif