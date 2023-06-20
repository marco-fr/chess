#include "xboard.hpp"
#include "alphabeta.hpp"
#include <csignal>
#include <iostream>
#include <sstream>
#include <string>

std::string XBoard::square_to_xboard_move(Alphabeta::eval_move result)
{
    std::string conversion = "abcdefgh";
    int from_ind = Magic::pop_first_bit(&result.from);
    int to_ind = Magic::pop_first_bit(&result.to);
    int fr = 8 - from_ind / 8, fc = from_ind % 8;
    int tr = 8 - to_ind / 8, tc = to_ind % 8;
    std::string send = (conversion[fc] + std::to_string(fr) + conversion[tc] +
                        std::to_string(tr));
    if (result.promote != -1)
    {
        send += board->char_from_piece(result.promote);
    }
    return send;
}

Alphabeta::eval_move XBoard::xboard_move_to_move(std::string s, int color)
{
    int fr = 8 - (int)(s[1] - '0'), tr = 8 - (int)(s[3] - '0');
    int fc = (int)(s[0] - 'a'), tc = (int)(s[2] - 'a');
    int from = fr * 8 + fc, to = tr * 8 + tc;
    U64 f = (1ULL << from), t = (1ULL << to);
    Alphabeta::eval_move opp_move;
    board->make_easy_move(f, t, color);
    opp_move.promote = -1;
    opp_move.from = f, opp_move.to = t;
    if (s.length() > 4)
    {
        board->promote(t, color, board->piece_from_character(s[4]) + 6 * color);
        opp_move.promote = board->piece_from_character(s[4]) + 6 * color;
    }
    return opp_move;
}

void XBoard::undo_move()
{
    if (!game_tree.empty())
        game_tree.pop_back();
    else
        std::cout << "Unable to remove move" << std::endl;
    color = WHITE;
    board->reset();
    for (auto& move : game_tree)
    {
        alg->move_alphabeta_eval(move, color);
        color = !color;
    }
}

void XBoard::loop()
{
    std::string line, token;
    signal(SIGINT, SIG_IGN);
    std::cout.setf(std::ios::unitbuf);
    int engine_can_play = 0;
    int force_mode = 0;
    Alphabeta::eval_move prev;

    while (std::getline(std::cin, line))
    {
        std::istringstream is(line);
        token.clear();
        is >> std::skipws >> token;
        if (token == "protover")
        {
            std::cout << "feature usermove=1" << std::endl;
            std::cout << "feature ping=1" << std::endl;
            std::cout << "feature setboard=1" << std::endl;
            std::cout << "done=1" << std::endl;
        }
        else if (token == "new")
        {
            board->reset();
            color = WHITE;
        }
        else if (token == "go")
        {
            // std::cout << "feature usermove=1" << std::endl;
            // std::cout << "feature ping=1" << std::endl;
            force_mode = 0;
            engine_can_play = 1;
        }
        else if (token == "usermove")
        {
            is >> std::skipws >> token;
            Alphabeta::eval_move opp_move = xboard_move_to_move(token, color);
            game_tree.push_back(opp_move);
            color = !color;
            if (!force_mode)
                engine_can_play = 1;
        }
        else if (token == "ping")
        {
            is >> std::skipws >> token;
            std::cout << "pong " << token << std::endl;
        }
        else if (token == "force")
        {
            force_mode = 1;
        }
        else if (token == "result")
        {
            // std::cout << "move " << square_to_xboard_move(prev) << std::endl;
            std::cout << "result" << std::endl;
        }
        else if (token == "undo")
        {
            if (force_mode)
            {
                undo_move();
            }
        }
        else if (token == "remove")
        {
            undo_move();
            undo_move();
            std::cout << "# Move removed" << std::endl;
            board->print_board();
        }
        else if (token == "setboard")
        {
            board->set_board(line.substr(9));
            std::cout << "# Using FEN: " << line.substr(9) << std::endl;
            std::cout << "# Turn: " << board->turn << std::endl;
            color = board->turn;
        }
        if (engine_can_play && !force_mode)
        {
            prev = alg->start_alphabeta(color, 0);
            game_tree.push_back(prev);
            std::cout << "move " << square_to_xboard_move(prev) << std::endl;
            std::cout << "# Tree size: " << game_tree.size() << std::endl;
            board->print_board();
            engine_can_play = 0;
            color = !color;
        }
    }
}

XBoard::XBoard(Alphabeta* a, Bitboard* b)
{
    alg = a;
    board = b;
    color = WHITE;
}