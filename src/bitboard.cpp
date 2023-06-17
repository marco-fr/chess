#include "bitboard.hpp"
#include "hash.hpp"
#include <iostream>

void Bitboard::print_U64(U64 b)
{
    std::cout << "U64: " << std::endl;
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            std::cout << (get_bit(b, i * 8 + j) ? 1 : 0) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void Bitboard::copy_flags(Flags from, Flags& to)
{
    to.ep[0] = from.ep[0], to.ep[1] = from.ep[1];
    to.king_castling[0] = from.king_castling[0];
    to.king_castling[1] = from.king_castling[1];
    to.queen_castling[0] = from.queen_castling[0];
    to.queen_castling[1] = from.queen_castling[1];
}

void Bitboard::print_board()
{
    for (int i = 0; i < 8; i++)
    {
        std::cout << i << "   ";
        for (int j = 0; j < 8; j++)
        {
            int k;
            for (k = 0; k < 12; k++)
            {
                if (get_bit(board[k], (i * 8 + j)))
                {
                    std::cout << piece_naming[k] << " ";
                    break;
                }
            }
            if (k == 12)
                std::cout << "0 ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    std::cout << "    ";
    for (int i = 0; i < 8; i++)
    {
        std::cout << i << " ";
    }
    std::cout << std::endl << std::endl;
}

void Bitboard::print_flags(Flags f)
{
    std::cout << "Flags: ";
    std::cout << "WEP " << f.ep[WHITE] << " ";
    std::cout << "BEP " << f.ep[BLACK] << " ";
    std::cout << "WCastleKing " << f.king_castling[0] << " ";
    std::cout << "WCastleQueen " << f.queen_castling[0] << " ";
    std::cout << "BCastleKing " << f.king_castling[1] << " ";
    std::cout << "BCastleQueen " << f.queen_castling[1] << " " << std::endl;
}

void Bitboard::set_board(std::string FEN)
{
    int row = 0, col = 0;
    running = 1;
    for (auto& i : board)
        i = 0ULL;
    for (char& c : FEN)
    {
        int piece = -1;
        int cn = c - '0';
        if (cn < 9 && cn > 0)
        {
            col += cn;
        }
        else
        {
            for (int i = 0; i < 12; i++)
            {
                if (c == piece_naming[i])
                {
                    piece = i;
                    break;
                }
            }
            if (piece != -1)
            {
                set_bit(board[piece], row * 8 + col);
                col++;
            }
            else if (c == '/')
            {
                row++, col = 0;
            }
            else
            {
                std::cout << "Invalid board input." << std::endl;
            }
        }
    }
}

void Bitboard::reset()
{
    set_board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    fl->king_castling[0] = 1, fl->king_castling[1] = 1;
    fl->queen_castling[0] = 1, fl->queen_castling[1] = 1;
    turn_number = 0;
}

U64 Bitboard::find_white_pieces()
{
    U64 res = 0;
    for (int i = 0; i < 6; i++)
    {
        res |= board[i];
    }
    return res;
}

U64 Bitboard::find_black_pieces()
{
    U64 res = 0;
    for (int i = 6; i < 12; i++)
    {
        res |= board[i];
    }
    return res;
}

U64 Bitboard::find_all_pieces() { return (board[12] | board[13]); }

int Bitboard::find_piece_index(U64 piece, int color)
{
    if (color == WHITE)
    {
        for (int i = 0; i < 6; i++)
        {
            if (board[i] & piece)
                return i;
        }
    }
    else
    {
        for (int i = 6; i < 12; i++)
        {
            if (board[i] & piece)
                return i;
        }
    }
    return 0;
}

void Bitboard::remove_square(U64 square)
{
    for (int i = 0; i < 12; i++)
    {
        board[i] &= ~square;
    }
}

int Bitboard::piece_from_character(char c)
{
    for (int i = 1; i < 5; i++)
    {
        if (piece_naming[i + 6] == c)
        {
            return i;
        }
    }
    return -1;
}

char Bitboard::char_from_piece(int piece)
{
    if (piece < 6)
        piece += 6;
    return piece_naming[piece];
}

void Bitboard::input_move(int color)
{
    int fr, fc, tr, tc, piece;
    std::string ch;
    U64 from, to;
    std::cout << "FROM row and col: ";
    std::cin >> fr >> fc;
    std::cout << "TO row and col: ";
    std::cin >> tr >> tc;
    from = (1ULL << (fr * 8 + fc));
    to = (1ULL << (tr * 8 + tc));
    piece = find_piece_index(from, color);
    make_easy_move(from, to, color);
    if (is_promoting_pawn(to, piece))
    {
        std::cout << "Promote to: ";
        std::cin >> ch;
        promote(to, color, piece_from_character(ch[0]) + 6 * color);
    }
}

void Bitboard::add_piece_with_hash(U64 square, int piece)
{
    board[piece] |= square;
    hash_key = hash->modify_hash_U64(hash_key, piece, square);
}

void Bitboard::remove_all_piece_square_with_hash(U64 square)
{
    for (int i = 0; i < 12; i++)
    {
        if (board[i] & square)
            hash_key = hash->modify_hash_U64(hash_key, i, square);
        board[i] &= ~square;
    }
}

void Bitboard::remove_piece_with_hash(U64 square, int piece)
{
    board[piece] &= ~square;
    hash_key = hash->modify_hash_U64(hash_key, piece, square);
}

void Bitboard::make_easy_move(U64 from, U64 to, int color)
{
    make_move(from, to, color, find_piece_index(from, color));
}

void Bitboard::promote(U64 from, int color, int piece_to)
{
    // board[W_PAWN + 6 * color] &= ~from;
    // board[piece_to] |= from;
    remove_piece_with_hash(from, W_PAWN + 6 * color);
    add_piece_with_hash(from, piece_to);
}

void Bitboard::unpromote(U64 from, int color, int piece_to)
{
    // board[piece_to] &= ~from;
    // board[W_PAWN + 6 * color] |= from;
    remove_piece_with_hash(from, piece_to);
    add_piece_with_hash(from, W_PAWN + 6 * color);
}

int Bitboard::is_promoting_pawn(U64 square, int piece)
{
    if (piece == W_PAWN || piece == B_PAWN)
    {
        if (square & RANK_1 || square & RANK_8)
        {
            return 1;
        }
    }
    return 0;
}

void Bitboard::make_move(U64 from, U64 to, int color, int piece)
{
    // int piece = find_piece_index(from, color);
    if (piece == W_PAWN)
    {
        if ((from >> 16 == to))
            fl->ep[BLACK] = to;
        else if ((from & RANK_5) &&
                 (((from >> 1) == fl->ep[WHITE] && to == (from >> 9)) ||
                  ((from << 1) == fl->ep[WHITE] && to == (from >> 7))))
        {
            // board[B_PAWN] &= (~fl->ep[WHITE]);
            remove_piece_with_hash(fl->ep[WHITE], B_PAWN);
            fl->ep[WHITE] = 0ULL;
        }
        else
            fl->ep[BLACK] = 0ULL;
    }
    else if (piece == B_PAWN)
    {
        if ((from << 16 == to))
            fl->ep[WHITE] = to;
        else if ((from & RANK_4) &&
                 (((from >> 1) == fl->ep[BLACK] && to == (from << 7)) ||
                  ((from << 1) == fl->ep[BLACK] && to == (from << 9))))
        {
            // board[W_PAWN] &= (~fl->ep[BLACK]);
            remove_piece_with_hash(fl->ep[BLACK], W_PAWN);
            fl->ep[BLACK] = 0ULL;
        }
        else
            fl->ep[WHITE] = 0ULL;
    }
    else
    {
        fl->ep[WHITE] = 0ULL;
        fl->ep[BLACK] = 0ULL;
    }
    if (piece == W_KING || piece == B_KING)
    {
        if ((from << 2) == to)
        {
            if (fl->king_castling[color])
            {
                add_piece_with_hash((from << 1), (W_ROOK + 6 * color));
                remove_piece_with_hash((from << 3), (W_ROOK + 6 * color));
                // board[W_ROOK + 6 * color] |= (from << 1);
                // board[W_ROOK + 6 * color] &= ~(from << 3);
            }
        }
        else if ((from >> 2) == to)
        {
            if (fl->queen_castling[color])
            {
                add_piece_with_hash((from >> 1), (W_ROOK + 6 * color));
                remove_piece_with_hash((from >> 4), (W_ROOK + 6 * color));
                // board[W_ROOK + 6 * color] |= (from >> 1);
                // board[W_ROOK + 6 * color] &= ~(from >> 4);
            }
        }
        fl->king_castling[color] = 0;
        fl->queen_castling[color] = 0;
    }
    else if (piece == W_ROOK)
    {
        if (from == (1ULL << 56))
            fl->queen_castling[WHITE] = 0;
        if (from == (1ULL << 63))
            fl->king_castling[WHITE] = 0;
    }
    else if (piece == B_ROOK)
    {
        if (from == (1ULL << 0))
            fl->queen_castling[BLACK] = 0;
        if (from == (1ULL << 7))
            fl->king_castling[BLACK] = 0;
    }
    remove_all_piece_square_with_hash(from);
    remove_all_piece_square_with_hash(to);
    add_piece_with_hash(to, piece);
    // remove_square(from);
    // remove_square(to);
    // board[piece] |= to;
    update_color_boards();
    turn_number++;
    if (turn_number > endgame_turning)
        endgame = 1;
    else
        endgame = 0;
}

void Bitboard::update_color_boards()
{
    board[W_PIECES] = find_white_pieces();
    board[B_PIECES] = find_black_pieces();
    board[ALL_PIECES] = find_all_pieces();
}

Bitboard::Bitboard(int color)
{
    for (auto& i : board)
        i = 0ULL;
    fl = new Flags;
    hash = new ZHash(HASH_MB_SIZE);
    hash->output_size();
    reset();
    update_color_boards();
    hash_key = hash->hash_board_U64(this, color);
}

Bitboard::~Bitboard()
{
    delete fl;
    delete hash;
    // delete board;
}