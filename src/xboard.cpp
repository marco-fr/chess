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

void XBoard::xboard_move_to_move(std::string s, int color)
{
    int fr = 8 - (int)(s[1] - '0'), tr = 8 - (int)(s[3] - '0');
    int fc = (int)(s[0] - 'a');
    int tc = (int)(s[2] - 'a');
    int from = fr * 8 + fc;
    int to = tr * 8 + tc;
    U64 f = (1ULL << from), t = (1ULL << to);
    board->make_easy_move(f, t, color);
    if (s.length() > 4)
    {
        board->promote(t, color, board->piece_from_character(s[4]) + 6 * color);
    }
}

void XBoard::loop()
{
    std::string line, token;
    signal(SIGINT, SIG_IGN);
    std::cout.setf(std::ios::unitbuf);
    int color = WHITE;

    while (std::getline(std::cin, line))
    {
        std::istringstream is(line);
        token.clear();
        is >> std::skipws >> token;
        if (token == "protover")
        {
            std::cout << "feature usermove=1" << std::endl;
            std::cout << "done=1" << std::endl;
        }
        else if (token == "new")
        {
            board->reset();
            color = WHITE;
        }
        else if (token == "go")
        {
            std::cout << "feature usermove=1" << std::endl;
            Alphabeta::eval_move result = alg->start_alphabeta(color);
            std::cout << "move " << square_to_xboard_move(result) << std::endl;
            color = !color;
        }
        else if (token == "usermove")
        {
            is >> std::skipws >> token;
            xboard_move_to_move(token, color);
            color = !color;
            Alphabeta::eval_move result = alg->start_alphabeta(color);
            std::cout << "move " << square_to_xboard_move(result) << std::endl;
            color = !color;
        }
    }
}

XBoard::XBoard(Alphabeta* a, Bitboard* b)
{
    alg = a;
    board = b;
}