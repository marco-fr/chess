#include "moves.hpp"
#include "bitboard.hpp"
#include "magic.hpp"

void Move::generate_file_rank()
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (j == 0)
                FILE_A |= (1ULL << (i * 8 + j));
            if (j == 0 || j == 1)
                FILE_AB |= (1ULL << (i * 8 + j));
            if (j == 7)
                FILE_H |= (1ULL << (i * 8 + j));
            if (j == 7 || j == 6)
                FILE_GH |= (1ULL << (i * 8 + j));
            if (i == 1)
                RANK_7 |= (1ULL << (i * 8 + j));
            if (i == 3)
                RANK_5 |= (1ULL << (i * 8 + j));
            if (i == 4)
                RANK_4 |= (1ULL << (i * 8 + j));
            if (i == 6)
                RANK_2 |= (1ULL << (i * 8 + j));
            if (i == 7)
                RANK_1 |= (1ULL << (i * 8 + j));
            if (i == 0)
                RANK_8 |= (1ULL << (i * 8 + j));
        }
    }
    KING_CASTLING[0] = (1ULL << 61) | (1ULL << 62);
    KING_CASTLING[1] = (1ULL << 5) | (1ULL << 6);
    QUEEN_CASTLING[0] = (1ULL << 57) | (1ULL << 58) | (1ULL << 59);
    QUEEN_CASTLING[1] = (1ULL << 1) | (1ULL << 2) | (1ULL << 3);
    curBoard->RANK_5 = RANK_5;
    curBoard->RANK_4 = RANK_4;
    curBoard->RANK_1 = RANK_1;
    curBoard->RANK_8 = RANK_8;
}

void Move::update_colors() { curBoard->update_color_boards(); }

U64 Move::find_knight_attacks(U64 square, int color)
{
    U64 attacks = (((square >> 6) | (square << 10)) & ~FILE_AB) |
                  (((square >> 10) | (square << 6)) & ~FILE_GH) |
                  (((square >> 15) | (square << 17)) & ~FILE_A) |
                  (((square >> 17) | (square << 15)) & ~FILE_H);
    curBoard->board[W_KNIGHT_ATTACK + 6 * color] |= attacks;
    return attacks;
}

U64 Move::find_king_attacks(U64 square, int color)
{
    U64 attacks = (((square >> 1) | (square >> 9) | (square << 7)) & ~FILE_H) |
                  (((square << 1) | (square << 9) | (square >> 7)) & ~FILE_A) |
                  ((square >> 8) | (square << 8));
    curBoard->board[W_KING_ATTACK + 6 * color] |= attacks;
    return attacks;
}
U64 Move::find_bishop_attacks(U64 square, int color)
{
    U64 attacks = Magic::find_bishop_moves(square, curBoard->find_all_pieces());
    curBoard->board[W_BISHOP_ATTACK + 6 * color] |= attacks;
    return attacks;
}

U64 Move::find_rook_attacks(U64 square, int color)
{
    U64 attacks = Magic::find_rook_moves(square, curBoard->find_all_pieces());
    curBoard->board[W_ROOK_ATTACK + 6 * color] |= attacks;
    return attacks;
}

U64 Move::find_queen_attacks(U64 square, int color)
{
    U64 attacks =
        (find_bishop_attacks(square, color) | find_rook_attacks(square, color));
    curBoard->board[W_QUEEN_ATTACK + 6 * color] |= attacks;
    return attacks;
}

U64 Move::find_pawn_attacks(U64 square, int color)
{
    U64 attacks;
    if (color == WHITE)
    {
        attacks = (((square >> 7) & ~FILE_A) | ((square >> 9) & ~FILE_H));
    }
    else
    {
        attacks = (((square << 7) & ~FILE_H) | ((square << 9) & ~FILE_A));
    }
    curBoard->board[W_PAWN_ATTACK + 6 * color] |= attacks;
    return attacks;
}

U64 Move::find_all_attacks_from_piece(U64 piece, int color,
                                      U64 (Move::*function)(U64, int))
{
    int square;
    U64 result = 0ULL;
    while (piece != 0ULL)
    {
        square = Magic::pop_first_bit(&piece);
        result |= (this->*function)((1ULL << square), color);
    }
    return result;
}

U64 Move::find_all_attacks(int color)
{
    U64 result = 0ULL;
    for (int i = 0; i < 12; i++)
    {
        curBoard->board[W_PAWN_ATTACK + i] = 0ULL;
    }
    for (int i = 0; i < 6; i++)
    {
        result |= find_all_attacks_from_piece(curBoard->board[i + 6 * color],
                                              color, function_calls[i]);
    }
    curBoard->board[W_ALL_ATTACKS + color] = result;
    return result;
}

int Move::check_check(int color)
{
    if (curBoard->board[W_KING + 6 * color] & find_all_attacks(!color))
        return 1;
    return 0;
}

U64 Move::find_king_legal_moves(U64 square, int color)
{
    U64 result = remove_color(find_king_attacks(square, color), color);
    U64 other = curBoard->board[W_ALL_ATTACKS + !color];
    int in_check = (other & curBoard->board[W_KING + 6 * color]);
    if (curBoard->fl->king_castling[color])
    {
        if (!in_check && !(other & KING_CASTLING[color]) &&
            !(curBoard->board[ALL_PIECES] & KING_CASTLING[color]) &&
            (curBoard->board[W_ROOK + 6 * color] & (square << 3)))
        {
            result |= (square << 2);
        }
    }
    if (curBoard->fl->queen_castling[color])
    {
        if (!in_check && !(other & QUEEN_CASTLING[color]) &&
            !(curBoard->board[ALL_PIECES] & QUEEN_CASTLING[color]) &&
            curBoard->board[W_ROOK + 6 * color] & (square >> 4))
        {
            result |= (square >> 2);
        }
    }
    return result;
}

U64 Move::find_knight_legal_moves(U64 square, int color)
{
    return remove_color(find_knight_attacks(square, color), color);
}

U64 Move::find_bishop_legal_moves(U64 square, int color)
{
    return remove_color(find_bishop_attacks(square, color), color);
}

U64 Move::find_rook_legal_moves(U64 square, int color)
{
    return remove_color(find_rook_attacks(square, color), color);
}

U64 Move::find_queen_legal_moves(U64 square, int color)
{
    return remove_color(find_queen_attacks(square, color), color);
}

U64 Move::find_pawn_legal_moves(U64 square, int color)
{
    U64 result = 0ULL;
    if (color == WHITE)
    {
        result |= ((square >> 8) & ~curBoard->board[ALL_PIECES]);
        if (result && (square & RANK_2))
        {
            result |= ((square >> 16) & ~curBoard->board[ALL_PIECES]);
        }
        result |=
            (find_pawn_attacks(square, color) & curBoard->board[B_PIECES]);
        if (square & RANK_5)
        {
            if ((square >> 1) & curBoard->fl->ep[WHITE])
                result |= (square >> 9);
            if ((square << 1) & curBoard->fl->ep[WHITE])
                result |= (square >> 7);
        }
    }
    else
    {
        result |= ((square << 8) & ~curBoard->board[ALL_PIECES]);
        if (result && (square & RANK_7))
        {
            result |= ((square << 16) & ~curBoard->board[ALL_PIECES]);
        }
        result |=
            (find_pawn_attacks(square, color) & curBoard->board[W_PIECES]);
        if (square & RANK_4)
        {
            if ((square >> 1) & curBoard->fl->ep[BLACK])
                result |= (square << 7);
            if ((square << 1) & curBoard->fl->ep[BLACK])
                result |= (square << 9);
        }
    }
    return result;
}

U64 Move::remove_color(U64 board, int color)
{
    if (color == WHITE)
        return (board & ~(curBoard->board[W_PIECES]));
    return (board & ~(curBoard->board[B_PIECES]));
}

Move::Move(Bitboard* b)
{
    curBoard = b;
    FILE_A = 0, FILE_AB = 0, FILE_GH = 0, FILE_H = 0, RANK_5 = 0, RANK_2 = 0,
    RANK_4 = 0, RANK_7 = 0, RANK_1 = 0, RANK_8 = 0;
    generate_file_rank();
    check_check(WHITE);
    check_check(BLACK);
}