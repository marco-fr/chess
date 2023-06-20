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

int Alphabeta::pieces_in_bitboard(
    U64* piece, int color, U64 (Move::*function)(U64, int), int depth,
    std::priority_queue<queue_item, std::vector<queue_item>, Compare>& list,
    int only_attacking)
{
    int place, move_place, prev_piece, tmp_score, done = 0;
    int type, promotion_it, is_promoting, is_attacking;
    U64 key_copy = curBoard->hash_key;
    queue_item qu;
    U64 promoting_square;
    U64 square, move, before, prev_board, piece_copy = *piece;
    Bitboard::Flags fl_copy;
    curBoard->copy_flags(*curBoard->fl, fl_copy);
    int any_moves = 0;

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
            is_attacking = 0;
            any_moves = 1;
            move_place = Magic::pop_first_bit(&possible_moves);
            move = (1ULL << move_place);
            if (move & curBoard->board[W_PIECES + !color])
                is_attacking = 1;
            if (is_attacking || !only_attacking)
            {
                // if(debug_test) curBoard->print_U64(move);
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
                        qu.score = -MAX;
                    }
                    else
                        qu.score =
                            curBoard->hash->access_table(curBoard->hash_key);
                    if (is_attacking)
                        qu.score = eval_table::piece_values[prev_piece] -
                                   eval_table::piece_values[type];
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
    return any_moves;
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

int Alphabeta::get_all_moves(
    int color, int depth,
    std::priority_queue<queue_item, std::vector<queue_item>, Compare>& list,
    int attacking)
{
    int any_moves = 0;
    for (int i = 0; i < 6; i++)
    {
        any_moves |=
            pieces_in_bitboard(&(curBoard->board[i + 6 * color]), color,
                               function_calls[i], depth, list, attacking);
    }
    return any_moves;
}

Alphabeta::eval_move Alphabeta::root_alphabeta(int depth, int color, int alpha,
                                               int beta, int attacking_only)
{
    U64 key = curBoard->hash_key;
    Bitboard::Flags fl_copy;
    curBoard->update_color_boards();
    int tmp_score;
    curBoard->copy_flags(*curBoard->fl, fl_copy);
    std::priority_queue<queue_item, std::vector<queue_item>, Compare> list;
    eval_move result;
    result.score = -MAX + (100 - depth);
    if (attacking_only)
        debug_test = 1;
    else
        debug_test = 0;
    if (mo->check_check(color))
        attacking_only = 0;
    get_all_moves(color, depth, list, attacking_only);
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
            else
                result.promote = -1;
        }
        if (result.score > alpha)
        {
            alpha = result.score;
            eval_move t;
            t.from = next.from, t.to = next.to, t.score = alpha,
            t.promote = next.promote;
            eval_tree[80 + depth] = t;
        }
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
    int in_check = mo->check_check(color), only_attacks = 1;
    U64 copy_attacks = curBoard->board[W_ALL_ATTACKS + !color];
    int standing_pat = eval_board(color);
    /*
    if(standing_pat == -145 || standing_pat == 145){
        curBoard->print_board();
    }*/
    std::priority_queue<queue_item, std::vector<queue_item>, Compare> list;
    U64 key = curBoard->hash_key;
    Bitboard::Flags fl_copy;
    int tmp_score;
    // int score = -MAX + (100 - depth);
    curBoard->copy_flags(*curBoard->fl, fl_copy);
    /*
    if(!in_check){
    if (alpha < standing_pat)
        alpha = standing_pat;
    if (depth < max_quise)
        return standing_pat;
    }*/
    if (in_check)
    {
        only_attacks = 0;
        if (debug_test)
            curBoard->print_board();
    }
    else
    {
        if (standing_pat >= beta)
            return standing_pat;
    }

    tmp_score =
        -quiescence(depth - 1, !color, -beta, -std::max(standing_pat, alpha));
    standing_pat = std::max(tmp_score, standing_pat);
    if (standing_pat >= beta)
        return standing_pat;
    if (tmp_score >= beta)
        return beta;
    if (tmp_score > alpha)
    {
        alpha = tmp_score;
    }
    int any_moves = get_all_moves(color, depth, list, only_attacks);

    /*
    if(!any_moves){
        if(!in_check) return 0;
        else return score;
    }*/

    while (!list.empty())
    {
        queue_item next = list.top();
        if (next.to & curBoard->board[W_KING + 6 * !color])
            return MAX - (100 - depth);
        if (depth <= max_quise &&
            !(next.from & curBoard->board[W_ALL_ATTACKS + !color] && !in_check))
        {
            // curBoard->print_board();
            list.pop();
            continue;
        }
        engine_make_move(next, color);
        tmp_score = -quiescence(depth - 1, !color, -beta,
                                -std::max(standing_pat, alpha));
        engine_remove_move(next, color, key, fl_copy);
        list.pop();
        standing_pat = std::max(tmp_score, standing_pat);
        curBoard->board[W_ALL_ATTACKS + !color];
        if (standing_pat >= beta)
            return standing_pat;
        if (tmp_score >= beta)
            return beta;
        if (tmp_score > alpha)
        {
            alpha = tmp_score;
        }
    }
    return standing_pat;
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
    if (!mo->check_check(color))
    {
        tmp_score = -alphabeta(depth - 1, !color, -beta, -alpha);
        if (tmp_score >= beta)
            return beta;
    }
    tmp_score = -alphabeta(depth - 1, !color, -beta, -alpha);
    score = std::max(score, tmp_score);
    if (score > alpha)
    {
        alpha = score;
    }
    if (alpha >= beta)
    {
        return score;
    }

    get_all_moves(color, depth, list, 0);
    if (list.empty())
    {
        if (!mo->check_check(color))
        {
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
        if (score > alpha)
        {
            alpha = score;
        }
        if (alpha >= beta)
        {
            break;
        }
    }
    return score;
}

void Alphabeta::move_alphabeta_eval(Alphabeta::eval_move result, int color)
{
    std::cout << "# Board Score: " << result.score << std::endl;
    curBoard->make_move(result.from, result.to, color,
                        curBoard->find_piece_index(result.from, color));
    if (result.promote != -1)
    {
        curBoard->promote(result.to, color, result.promote);
    }
    // std::cout << "# Current Board Evaluation: " << eval_board(color) <<
    // std::endl; curBoard->print_U64(curBoard->board[B_KNIGHT]);
}

Alphabeta::eval_move Alphabeta::start_alphabeta(int color, int attacking_only)
{
    std::chrono::steady_clock::time_point begin =
        std::chrono::steady_clock::now();
    if (curBoard->PRINT_INFO)
    {
        std::cout << "# Depth: " << depth
                  << ", Quiescence depth: " << -max_quise << std::endl;

        curBoard->print_flags(*curBoard->fl);
        curBoard->hash->reset_times_accessed();
        std::cout << "# Turn #" << curBoard->turn_number << std::endl;
    }
    eval_move result = root_alphabeta(depth, color, -MAX, MAX, attacking_only);
    move_alphabeta_eval(result, color);
    if (curBoard->PRINT_INFO)
    {
        std::cout << "# Hash table accessed "
                  << curBoard->hash->get_times_accessed()
                  << " times during this turn" << std::endl;
        std::chrono::steady_clock::time_point end =
            std::chrono::steady_clock::now();
        std::cout << "# Execution time: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(
                         end - begin)
                         .count()
                  << "ms" << std::endl;
        // if (std::chrono::duration_cast<std::chrono::milliseconds>(end -
        // begin)
        //       .count() < 300)
        //   depth++;
        curBoard->print_board();
    }
    return result;
}

Alphabeta::Alphabeta(Move* move, int depth, int quise)
{
    mo = move;
    curBoard = mo->curBoard;
    this->depth = depth;
    this->max_quise = quise;
    std::cout << eval_board(BLACK) << std::endl;
}
