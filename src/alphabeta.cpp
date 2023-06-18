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

int Alphabeta::eval_board(int color)
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
    if (color)
        return -score;
    return score;
}

Alphabeta::eval_move Alphabeta::pieces_in_bitboard(
    U64* piece, int color, U64 (Move::*function)(U64, int), int depth,
    std::priority_queue<queue_item, std::vector<queue_item>, Compare>& list,
    int only_attacking)
{
    int place, move_place, prev_piece, tmp_score, done = 0;
    struct eval_move e;
    int type, promotion_it, is_promoting;
    U64 key_copy = curBoard->hash_key;
    U64 promoting_square;
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
            if (move & curBoard->board[W_PIECES + !color] || !only_attacking)
            {
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
                    if (curBoard->hash->not_good_for_deepening(
                            curBoard->hash_key, curBoard, depth))
                    {
                        qu.score = e.score;
                    }
                    else
                        qu.score =
                            curBoard->hash->access_table(curBoard->hash_key);
                    // if (color)
                    // qu.score = -qu.score;
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
        }
        *piece |= (square);
        curBoard->update_color_boards();
    }
    return e;
}

void Alphabeta::engine_make_move(Alphabeta::queue_item next, int color)
{
    curBoard->make_move(next.from, next.to, color, next.type);
    if (next.is_promoting)
    {
        curBoard->promote(next.to, color, next.promote);
    }
}

void Alphabeta::engine_remove_move(Alphabeta::queue_item next, int color,
                                   U64 key, Bitboard::Flags& fl_copy)
{
    if (next.is_promoting)
    {
        curBoard->unpromote(next.to, color, next.promote);
    }
    curBoard->turn_number--;
    curBoard->hash_key = key;
    curBoard->turn = !curBoard->turn;
    curBoard->board[next.prev_piece] = next.prev_board;
    curBoard->board[next.type] &= ~(next.to);
    curBoard->board[next.type] |= (next.from);
    curBoard->copy_flags(fl_copy, *curBoard->fl);
    curBoard->update_color_boards();
}

void Alphabeta::get_all_moves(
    int color, int depth,
    std::priority_queue<queue_item, std::vector<queue_item>, Compare>& list,
    int attacking)
{
    for (int i = 0; i < 6; i++)
    {
        pieces_in_bitboard(&(curBoard->board[i + 6 * color]), color,
                           function_calls[i], depth, list, attacking);
    }
}

Alphabeta::eval_move Alphabeta::root_alphabeta(int depth, int color, int alpha,
                                               int beta)
{
    U64 key = curBoard->hash_key;
    Bitboard::Flags fl_copy;
    int tmp_score;
    curBoard->copy_flags(*curBoard->fl, fl_copy);
    std::priority_queue<queue_item, std::vector<queue_item>, Compare> list;
    eval_move result;
    result.score = -MAX + (100 - depth);
    get_all_moves(color, depth, list, 0);
    while (!list.empty())
    {
        queue_item next = list.top();
        engine_make_move(next, color);
        if (!curBoard->hash->is_empty(curBoard->hash_key, curBoard, depth - 1))
        {
            tmp_score = curBoard->hash->access_table(curBoard->hash_key);
        }
        else
        {
            tmp_score = -alphabeta(depth - 1, !color, -beta, -alpha);
            // tmp_score = -quiescence(depth, !color, -beta, -alpha);
            curBoard->hash->set_table_index(curBoard->hash_key, tmp_score,
                                            curBoard, depth - 1);
        }
        // std::cout << tmp_score << std::endl;
        // curBoard->print_board();
        engine_remove_move(next, color, key, fl_copy);
        list.pop();
        if (result.score < tmp_score)
        {
            result.score = tmp_score;
            result.from = next.from, result.to = next.to;
            if (next.is_promoting)
                result.promote = next.promote;
        }
        alpha = std::max(alpha, result.score);
        if (alpha >= beta)
            break;
    }
    if (result.from == 0ULL && result.to == 0ULL)
    {
        if (!mo->check_check(color))
        {
            result.score = 0;
        }
    }
    return result;
}

int Alphabeta::quiescence(int depth, int color, int alpha, int beta)
{
    int standing_pat = eval_board(color);
    std::priority_queue<queue_item, std::vector<queue_item>, Compare> list;
    U64 key = curBoard->hash_key;
    Bitboard::Flags fl_copy;
    int tmp_score;
    int score = -MAX + (100 - depth);
    curBoard->copy_flags(*curBoard->fl, fl_copy);

    if (standing_pat >= beta)
        return beta;
    if (alpha < standing_pat)
        alpha = standing_pat;
    if (depth < max_quise)
        return alpha;

    get_all_moves(color, depth, list, 1);

    if(list.empty()){
        get_all_moves(color, depth, list, 0);
        if(list.empty()){
            if(!mo->check_check(color)) return 0;
            else return score;
        }
        return alpha;
    }

    while (!list.empty())
    {
        queue_item next = list.top();
        engine_make_move(next, color);
        if (!curBoard->hash->is_empty(curBoard->hash_key, curBoard, depth - 1))
        {
            tmp_score = curBoard->hash->access_table(curBoard->hash_key);
        }
        else
        {
            tmp_score = -quiescence(depth - 1, !color, -beta, -alpha);
            //curBoard->hash->set_table_index(curBoard->hash_key, tmp_score,
                                            //curBoard, depth - 1);
        }
        engine_remove_move(next, color, key, fl_copy);
        list.pop();
        if (tmp_score >= beta)
            return beta;
        if (tmp_score > alpha)
            alpha = score;
    }
    return alpha;
}

int Alphabeta::alphabeta(int depth, int color, int alpha, int beta)
{
    U64 key = curBoard->hash_key;
    Bitboard::Flags fl_copy;
    int tmp_score;
    curBoard->copy_flags(*curBoard->fl, fl_copy);
    std::priority_queue<queue_item, std::vector<queue_item>, Compare> list;
    int score = -MAX + (100 - depth);
    if (depth == 0)
    {
        // return eval_board(color);
        return quiescence(depth, color, alpha, beta);
    }
    get_all_moves(color, depth, list, 0);
    if(list.empty()){
        if(!mo->check_check(color)){
            score = 0;
        }
    }
    while (!list.empty())
    {
        queue_item next = list.top();
        engine_make_move(next, color);
        if (!curBoard->hash->is_empty(curBoard->hash_key, curBoard, depth - 1))
        {
            tmp_score = curBoard->hash->access_table(curBoard->hash_key);
        }
        else
        {
            tmp_score = -alphabeta(depth - 1, !color, -beta, -alpha);
            curBoard->hash->set_table_index(curBoard->hash_key, tmp_score,
                                            curBoard, depth - 1);
        }
        engine_remove_move(next, color, key, fl_copy);
        list.pop();

        score = std::max(score, tmp_score);
        alpha = std::max(alpha, score);
        if (alpha >= beta)
            break;
    }
    return score;
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
    std::cout << "Depth: " << depth << ", Quiescence depth: " << -max_quise
              << std::endl;
    std::chrono::steady_clock::time_point begin =
        std::chrono::steady_clock::now();

    curBoard->print_flags(*curBoard->fl);
    curBoard->hash->reset_times_accessed();
    std::cout << "Turn #" << curBoard->turn_number << std::endl;

    eval_move result = root_alphabeta(depth, color, -MAX, MAX);

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