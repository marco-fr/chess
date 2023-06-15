#include<iostream>

#define set_bit(b, i) ((b) |= (1ULL << i))
#define get_bit(b, i) ((b) & (1ULL << i))  
#define clear_bit(b, i) ((b) &= ~(1ULL << i)) 
#define get_LSB(b) (__builtin_ctzll(b))

#define U64 uint64_t
enum{
    W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
    B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
    W_PIECES, B_PIECES, ALL_PIECES, W_E_P, B_E_P,
    W_PAWN_ATTACK, W_KNIGHT_ATTACK, W_BISHOP_ATTACK, W_ROOK_ATTACK, W_QUEEN_ATTACK, W_KING_ATTACK,
    B_PAWN_ATTACK, B_KNIGHT_ATTACK, B_BISHOP_ATTACK, B_ROOK_ATTACK, B_QUEEN_ATTACK, B_KING_ATTACK,
    W_ALL_ATTACKS, B_ALL_ATTACKS
};

enum{
    WHITE, BLACK
};

class Bitboard{
public:
    static void print_U64(U64 b);

    void print_board();
    void set_board(std::string FEN);
    void reset();
    void input_move(int color);
    void make_move(U64 from, U64 to, int color, int piece);
    void make_easy_move(U64 form, U64 to, int color);
    void remove_square(U64 square);

    U64 find_white_pieces();
    U64 find_black_pieces();
    U64 find_all_pieces();
    int find_piece_index(U64 piece, int color);
    void update_color_boards();
    U64 board[31];
    int king_castling[2] = {1,1};
    int queen_castling[2] = {1,1};

    U64 RANK_5, RANK_4;

    int turn_number = 1;
    int running = 1;
    int endgame = 0;
    int turn = WHITE;
    char piece_naming[12] = {'P', 'N', 'B', 'R', 'Q', 'K',
                            'p', 'n', 'b', 'r', 'q', 'k'};

    Bitboard();
private:
    //White: Pawn, Knight, Bishop, Rook, Queen, King...
};

void Bitboard::print_U64(U64 b){
    std::cout << "U64: " << std::endl;
    for(int i = 0; i < 8; i++){
        for(int j = 0; j < 8; j++){
            std::cout << (get_bit(b, i * 8 + j) ? 1 : 0) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void Bitboard::print_board(){
    for(int i = 0; i < 8; i++){
        std::cout << i << "   ";
        for(int j = 0; j < 8; j++){
            int k;
            for(k = 0; k < 12; k++){
                if(get_bit(board[k], (i * 8 + j))){
                    std::cout << piece_naming[k] << " ";
                    break;
                }
            }  
            if(k == 12) std::cout << "0 ";
        }
        std::cout << std:: endl;
    }
    std::cout << std::endl;
    std::cout << "    ";
    for(int i = 0; i < 8; i++){
        std::cout << i << " ";
    }
    std::cout << std::endl << std::endl;
}

void Bitboard::set_board(std::string FEN){
    int row = 0, col = 0;
    running = 1;
    for(auto &i : board)
        i = 0ULL;
    for(char &c : FEN){
        int piece = -1;
        int cn = c - '0';
        if(cn < 9 && cn > 0){
            col += cn;
        }
        else{
            for(int i = 0; i < 12; i++){
                if(c == piece_naming[i]){
                    piece = i;
                    break;
                }
            }
            if(piece != -1){
                set_bit(board[piece], row * 8 + col);
                col++;
            }
            else if(c == '/'){
                row++, col = 0;
            }
            else{
                std::cout << "Invalid board input." << std::endl;
            }
        }
    }
}

void Bitboard::reset(){
    set_board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    king_castling[0] = 1, king_castling[1] = 1;
    queen_castling[0] = 1, queen_castling[1] = 1;
}

U64 Bitboard::find_white_pieces(){
    U64 res = 0;
    for(int i = 0; i < 6; i++){
        res |= board[i];
    }
    return res;
}

U64 Bitboard::find_black_pieces(){
    U64 res = 0;
    for(int i = 6; i < 12; i++){
        res |= board[i];
    }
    return res;
}

U64 Bitboard::find_all_pieces(){
    return (board[12] | board[13]);
}

int Bitboard::find_piece_index(U64 piece, int color){
    if(color == WHITE){
        for(int i = 0; i < 6; i++){
            if(board[i] & piece) return i;
        }
    }
    else{
        for(int i = 6; i < 12; i++){
            if(board[i] & piece) return i;
        }
    }
    return 0;
}

void Bitboard::remove_square(U64 square){
    for(int i = 0; i < 12; i++){
        board[i] &= ~square;
    }
}

void Bitboard::input_move(int color){
    int fr, fc, tr, tc;
    U64 from, to;
    std::cout << "FROM row and col: ";
    std::cin >> fr >> fc;
    std::cout << "TO row and col: ";
    std::cin >> tr >> tc;
    from = (1ULL << (fr * 8 + fc));
    to = (1ULL << (tr * 8 + tc));
    make_easy_move(from, to, color);
}

void Bitboard::make_easy_move(U64 from, U64 to, int color){
    make_move(from, to, color, find_piece_index(from, color));
}

void Bitboard::make_move(U64 from, U64 to, int color, int piece){
    //int piece = find_piece_index(from, color);
    if(piece == W_PAWN){
       if((from >> 16 == to)) board[B_E_P] = to;
       else if((from & RANK_5) && 
                (((from >> 1) == board[W_E_P] && to == (from >> 9))
                || ((from << 1) == board[W_E_P] && to == (from >> 7)))){
        board[B_PAWN] &= (~board[W_E_P]);
        board[W_E_P] = 0ULL;
       }
    }
    else if(piece == B_PAWN){
        if((from << 16 == to)) board[W_E_P] = to;
        else if((from & RANK_4) && 
                (((from >> 1) == board[B_E_P] && to == (from << 7))
                || ((from << 1) == board[B_E_P] && to == (from << 9)))){
        board[W_PAWN] &= (~board[B_E_P]);
        board[B_E_P] = 0ULL;
       }
    }
    else if(piece == W_KING || piece == B_KING){
        if((from << 2) == to){
            if(king_castling[color]){
                board[W_ROOK + 6 * color] |= (from << 1);
                board[W_ROOK + 6 * color] &= ~(from << 3);
            }
        }
        else if((from >> 2) == to){
            if(queen_castling[color]){
                board[W_ROOK + 6 * color] |= (from >> 1);
                board[W_ROOK + 6 * color] &= ~(from >> 4);
            }
        }
        king_castling[color] = 0;
        queen_castling[color] = 0;
    }
    else if(piece == W_ROOK){
        if(from == (1ULL << 56)) queen_castling[WHITE] = 0;
        if(from == (1ULL << 63)) king_castling[WHITE] = 0;
    }
    else if(piece == B_ROOK){
        if(from == (1ULL << 0)) queen_castling[BLACK] = 0;
        if(from == (1ULL << 7)) king_castling[BLACK] = 0;
    }
    remove_square(from);
    remove_square(to);
    board[piece] |= to;
    update_color_boards();
    turn_number++;
    if(turn_number > 15) endgame = 1;
    else endgame = 0;
}

void Bitboard::update_color_boards(){
    board[W_PIECES] = find_white_pieces();
    board[B_PIECES] = find_black_pieces();
    board[ALL_PIECES] = find_all_pieces();
}

Bitboard::Bitboard(){
    for(auto &i : board)
        i = 0ULL;
    reset();
    update_color_boards();
}