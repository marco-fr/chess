#include "alphabeta.hpp"
#include "bitboard.hpp"
#include "moves.hpp"
#include <chrono>
#include <queue>

class Compare
{
public:
    bool operator()(Alphabeta::queue_item a, Alphabeta::queue_item b)
    {
        return (a.score < b.score);
    }
};

int Alphabeta::eval_piece(int piece)
{
    U64 copy = curBoard->board[piece];
    int score = 0;
    int place;
    while (copy != 0ULL)
    {
        place = Magic::pop_first_bit(&copy);
        score += eval_table::table[curBoard->endgame][piece][place];
        score += eval_table::piece_values[piece];
    }
    // score += eval_table::mobility_mult[curBoard->endgame][piece] *
    // Magic::num_of_ones(curBoard->board[W_PAWN_ATTACK + piece]);
    return score;
}

int Alphabeta::eval_board()
{
    int score = 0;
    for (int i = 0; i < 6; i++)
    {
        score += eval_piece(i);
    }
    for (int i = 6; i < 12; i++)
    {
        score -= eval_piece(i);
    }
    return score;
}

Alphabeta::eval_move Alphabeta::pieces_in_bitboard(
    U64* piece, int color, U64 (Move::*function)(U64, int), int depth,
    int alpha, int beta,
    std::priority_queue<queue_item, std::vector<queue_item>, Compare>& list)
{
    int place, move_place, prev_piece, tmp_score, done = 0;
    struct eval_move e;
    int type, promotion_it, is_promoting;
    U64 key_copy = curBoard->hash_key;
    U64 promoting_square;
    if (color)
        e.score = 1e9 - (100 - depth);
    else
        e.score += (100 - depth);
    U64 square, move, before, prev_board, piece_copy = *piece;
    Bitboard::Flags fl_copy;
    curBoard->copy_flags(*curBoard->fl, fl_copy);

    while (piece_copy != 0ULL && !done)
    {
        // Get piece
        place = Magic::pop_first_bit(&piece_copy);
        square = (1ULL << place);
        type = curBoard->find_piece_index(square, color);
        U64 possible_moves = (mo->*function)(square, color);
        *piece &= ~(square);
        promotion_it = 1, is_promoting = 0, promoting_square = 0ULL;

        while (possible_moves != 0ULL && !done)
        {
            // Get move
            move_place = Magic::pop_first_bit(&possible_moves);
            move = (1ULL << move_place);
            prev_piece = curBoard->find_piece_index(move, !color);
            if (type == W_PAWN && prev_piece == 0)
                prev_piece = B_PAWN;
            if (type == W_KING || type == B_KING)
            {
                if ((square << 2) == move || (square >> 2) == move)
                {
                    prev_piece = W_ROOK + 6 * color;
                }
            }
            prev_board = curBoard->board[prev_piece];
            // Make move
            is_promoting = curBoard->is_promoting_pawn(move, type);
            curBoard->make_move(square, move, color, type);
            if (is_promoting)
            {
                if (promoting_square != move)
                {
                    promotion_it = 1;
                    promoting_square = move;
                }
                if (promotion_it < 4)
                    possible_moves |= move;
                curBoard->promote(move, color, promotion_it + 6 * color);
            }
            if (!mo->check_check(color))
            {
                queue_item qu;
                qu.from = square, qu.to = move;
                qu.prev_board = prev_board, qu.type = type,
                qu.prev_piece = prev_piece;
                qu.is_promoting = is_promoting;
                if (is_promoting)
                {
                    qu.promote = promotion_it + 6 * color;
                }
                if (curBoard->hash->not_good_for_deepening(curBoard->hash_key,
                                                           curBoard, depth))
                {
                    qu.score = e.score;
                }
                else
                    qu.score = curBoard->hash->access_table(curBoard->hash_key);
                if (color)
                    qu.score = -qu.score;
                list.push(qu);
            }
            if (is_promoting)
            {
                curBoard->unpromote(move, color, promotion_it + 6 * color);
                promotion_it++;
            }

            // Reset move
            curBoard->remove_piece_with_hash(move, type);
            curBoard->turn_number--;
            curBoard->hash_key = key_copy;
            curBoard->board[prev_piece] = prev_board;
            curBoard->copy_flags(fl_copy, *curBoard->fl);
        }
        *piece |= (square);
        curBoard->update_color_boards();
    }
    return e;
}

void Alphabeta::comp_moves(int color, Alphabeta::eval_move* b,
                           Alphabeta::eval_move e)
{
    if ((b)->score < e.score && !color)
    {
        b->score = e.score;
        b->from = e.from, b->to = e.to, b->promote = e.promote;
    }
    else if (b->score > e.score && color)
    {
        b->score = e.score;
        b->from = e.from, b->to = e.to, b->promote = e.promote;
    }
}

Alphabeta::eval_move Alphabeta::alphabeta(int depth, int color, int alpha,
                                          int beta)
{
    U64 key = curBoard->hash_key;
    Bitboard::Flags fl_copy;
    int tmp_score;
    int done = 0;
    curBoard->copy_flags(*curBoard->fl, fl_copy);
    std::priority_queue<queue_item, std::vector<queue_item>, Compare> list;

    // std::cout << key << std::endl;
    eval_move result;
    if (color)
        result.score = 1e9 - (100 - depth);
    else
        result.score += (100 - depth);
    if (depth == 0)
    {
        mo->check_check(color);
        int p = eval_board();
        curBoard->hash->set_table_index(key, p, curBoard, depth);
        result.score = p;
        return result;
    }
        
    for (int i = 0; i < 6; i++)
    {
        pieces_in_bitboard(&(curBoard->board[i + 6 * color]), color,
                           function_calls[i], depth, alpha, beta, list);
    }
    while (!list.empty() && !done)
    {
        queue_item next = list.top();
        if (color)
            next.score = -next.score;
        curBoard->board[next.type] &= ~(next.from);
        curBoard->make_move(next.from, next.to, color, next.type);
        if (next.is_promoting)
        {
            curBoard->promote(next.to, color, next.promote);
        }
        if (!curBoard->hash->is_empty(curBoard->hash_key, curBoard, depth - 1))
        {
            tmp_score = curBoard->hash->access_table(curBoard->hash_key);
        }
        else{
            tmp_score = alphabeta(depth - 1, !color, alpha, beta).score;
            curBoard->hash->set_table_index(curBoard->hash_key, tmp_score, curBoard, depth - 1);
        }
        if (!color)
        {
            if (tmp_score > result.score)
            {
                result.score = tmp_score;
                result.from = next.from, result.to = next.to;
                if (next.is_promoting)
                    result.promote = next.promote;
            }
            if (tmp_score >= beta)
                done = 1;
            else if (tmp_score > alpha)
                alpha = tmp_score;
        }
        else
        {
            if (tmp_score < result.score)
            {
                result.score = tmp_score;
                result.from = next.from, result.to = next.to;
                if (next.is_promoting)
                    result.promote = next.promote;
            }
            if (tmp_score <= alpha)
                done = 1;
            else if (tmp_score < beta)
                beta = tmp_score;
        }
        if (next.is_promoting)
        {
            curBoard->unpromote(next.to, color, next.promote);
        }
        curBoard->turn_number--;
        curBoard->hash_key = key;
        curBoard->board[next.prev_piece] = next.prev_board;
        curBoard->board[next.type] &= ~(next.to);
        curBoard->board[next.type] |= (next.from);
        curBoard->copy_flags(fl_copy, *curBoard->fl);
        curBoard->update_color_boards();
        list.pop();
    }
    if(result.from == 0ULL && result.to == 0ULL){
        if(!mo->check_check(color)){
            result.score = 0;
        }
    }
    return result;
}

void Alphabeta::move_alphabeta_eval(Alphabeta::eval_move result, int color)
{
    std::cout << "Evaluation: " << result.score << std::endl;
    curBoard->make_move(result.from, result.to, color,
                        curBoard->find_piece_index(result.from, color));
    if (result.promote != -1)
    {
        curBoard->promote(result.to, color, result.promote);
    }
}

Alphabeta::eval_move Alphabeta::start_alphabeta(int color)
{
    std::cout << "Depth: " << depth << std::endl;
    std::chrono::steady_clock::time_point begin =
        std::chrono::steady_clock::now();

    curBoard->print_flags(*curBoard->fl);
    curBoard->hash->reset_times_accessed();
    std::cout << "Turn #" << curBoard->turn_number << std::endl;

    eval_move result = alphabeta(depth, color, INT32_MIN, INT32_MAX);

    std::cout << "Hash table accessed " << curBoard->hash->get_times_accessed()
              << " times during this turn" << std::endl;
    move_alphabeta_eval(result, color);
    std::chrono::steady_clock::time_point end =
        std::chrono::steady_clock::now();
    std::cout << "Execution time: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                       begin)
                     .count()
              << "ms" << std::endl;
    // if (std::chrono::duration_cast<std::chrono::milliseconds>(end - begin)
    //       .count() < 300)
    //   depth++;
    curBoard->print_board();
    return result;
}

Alphabeta::Alphabeta(Move* move, int depth)
{
    mo = move;
    curBoard = mo->curBoard;
    this->depth = depth;
}