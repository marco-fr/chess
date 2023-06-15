#include "moves.cpp"
#include "evaltables.hpp"
#include <chrono>

class Alphabeta{
    public:
    int alphabeta(int depth, int color, int alpha, int beta);
    void start_alphabeta(int color);
    struct eval_move;
    int eval_piece(int piece);
    int eval_board();
    void reset_flags(int king[2], int queen[2], U64 wep, U64 bep);
    void comp_moves(int color, Alphabeta::eval_move *b, Alphabeta::eval_move e);
    Alphabeta::eval_move pieces_in_bitboard(U64 *piece, int color, 
        U64 (Move::*function)(U64, int), int depth, int alpha, int beta);

    void pawn_in_bitboard(U64 *piece, int color);
    void moves_for_piece(U64 square, U64 moves, int color);
    Alphabeta(Move *move, int depth);

    private:
    int depth;
    Move *mo;
    Bitboard *curBoard;

    U64 (Move::*function_calls[6])(U64, int) = {
        &Move::find_pawn_legal_moves,
        &Move::find_knight_legal_moves,
        &Move::find_bishop_legal_moves,
        &Move::find_rook_legal_moves,
        &Move::find_queen_legal_moves,
        &Move::find_king_legal_moves
    };
};

struct Alphabeta::eval_move{
        int score = -1e9;
        U64 from = 0ULL, to = 0ULL;
};

int Alphabeta::eval_piece(int piece){
    U64 copy = curBoard->board[piece];
    int score = 0;
    int place;
    while(copy != 0ULL){
        place = Magic::pop_first_bit(&copy);
        score += eval_table::table[curBoard->endgame][piece][place];
        score += eval_table::piece_values[piece];
    }
    score += eval_table::mobility_mult[curBoard->endgame][piece] 
            * Magic::num_of_ones(curBoard->board[W_PAWN_ATTACK + piece]);
    return score;
}

int Alphabeta::eval_board(){
    int score = 0;
    for(int i = 0; i < 6; i++){
        score += eval_piece(i);
    }
    for(int i = 6; i < 12; i++){
        score -= eval_piece(i);
    }
    return score;
}

void Alphabeta::reset_flags(int king[2], int queen[2], U64 wep, U64 bep){
    curBoard->board[W_E_P] = wep;
    curBoard->board[B_E_P] = bep;
    curBoard->king_castling[0] = king[0], curBoard->king_castling[1] = king[1];
    curBoard->queen_castling[0] = queen[0], curBoard->queen_castling[1] = queen[1];
    if(curBoard->turn_number > 15) curBoard->endgame = 1;
    else curBoard->endgame = 0;
}

Alphabeta::eval_move Alphabeta::pieces_in_bitboard(U64 *piece, int color, U64 (Move::*function)(U64, int), int depth, int alpha, int beta){
    int place, move_place, prev_piece, tmp_score, done = 0,
        k_castle_copy[2], q_castle_copy[2];
    struct eval_move e;
    int type;
    if(color) e.score = 1e9;
    U64 square, move, before, prev_board, piece_copy = *piece,
        wep_copy, bep_copy;
    k_castle_copy[0] = curBoard->king_castling[0], k_castle_copy[1] = curBoard->king_castling[1];
    q_castle_copy[0] = curBoard->queen_castling[0], q_castle_copy[1] = curBoard->queen_castling[1];
    wep_copy = curBoard->board[W_E_P], bep_copy = curBoard->board[B_E_P];

    while(piece_copy != 0ULL && !done){
        //Get piece
        place = Magic::pop_first_bit(&piece_copy);
        square = (1ULL << place);
        type = curBoard->find_piece_index(square, color);
        U64 possible_moves = (mo->*function)(square, color);
        *piece &= ~(square);

        while(possible_moves!= 0ULL && !done){
            //Get move
            move_place = Magic::pop_first_bit(&possible_moves);
            move = (1ULL << move_place);
            prev_piece = curBoard->find_piece_index(move, !color);
            if(type == W_PAWN && prev_piece == 0) prev_piece = B_PAWN;
            if(type == W_KING || type == B_KING){
                if((square << 2) == move || (square >> 2) == move){
                    prev_piece = W_ROOK + 6 * color;
                }
            }
            prev_board = curBoard->board[prev_piece];
            //Make move
            curBoard->make_move(square, move, color, type);
            if(!mo->check_check(color)){
                tmp_score = alphabeta(depth - 1, !color, alpha, beta);
                if(!color){
                    if(tmp_score > e.score){
                        e.score = tmp_score;
                        e.from = square, e.to = move;
                    }
                    if(tmp_score >= beta) done = 1;
                    else if(tmp_score > alpha) alpha = tmp_score;
                }
                else{
                    if(tmp_score < e.score){
                        e.score = tmp_score;
                        e.from = square, e.to = move;
                    }
                     if(tmp_score <= alpha) done = 1;
                    else if(tmp_score < beta) beta = tmp_score;
                }
            }

            //Reset move
            *piece &= ~(move);
            curBoard->board[prev_piece] = prev_board;
            reset_flags(k_castle_copy, q_castle_copy, wep_copy, bep_copy);
        }
        *piece |= (square);
        curBoard->update_color_boards();
    }
    return e;
}

void Alphabeta::comp_moves(int color, Alphabeta::eval_move *b, Alphabeta::eval_move e){
    if((b)->score < e.score && !color){
        b->score = e.score;
        b->from = e.from, b->to = e.to;
    }
    else if(b->score > e.score && color){
        b->score = e.score;
        b->from = e.from, b->to = e.to;
    }
}

int Alphabeta::alphabeta(int depth, int color, int alpha, int beta){
    if(depth == 0){
        mo->check_check(color);
        return eval_board();
    }
    eval_move result;
    if(color) result.score = 1e9;
    for(int i = 0; i < 6; i++){
        comp_moves(color, &result,
            pieces_in_bitboard(&(curBoard->board[i + 6 * color]), 
            color, function_calls[i], depth, alpha, beta));
    }
    if(depth == this->depth){
        std::cout << "Score: " << result.score << std::endl;
        curBoard->make_move(result.from, result.to, color, 
            curBoard->find_piece_index(result.from, color));
    }
    return result.score;
}

void Alphabeta::start_alphabeta(int color){
    std::cout << "Depth: " << depth << std::endl;
    std::cout << "Flags: ";
    std::cout << "WEP " << curBoard->board[W_E_P] << " ";
    std::cout << "BEP " << curBoard->board[B_E_P] << " ";
    std::cout << "WCastleKing " << curBoard->king_castling[0] << " ";
    std::cout << "WCastleQueen " << curBoard->queen_castling[0] << " ";
    std::cout << "BCastleKing " << curBoard->king_castling[1] << " ";
    std::cout << "BCastleQueen " << curBoard->queen_castling[1] << " " << std::endl;
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    alphabeta(depth, color, INT32_MIN, INT32_MAX);

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Execution time: " << 
        std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() 
        << "ms" << std::endl;
    curBoard->print_board();
}

Alphabeta::Alphabeta(Move *move, int depth){
    this->depth = depth;
    mo = move;
    curBoard = mo->curBoard;
}