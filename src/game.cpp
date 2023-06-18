#include "game.hpp"
#include "magic.hpp"
#include "xboard.hpp"

enum
{
    HUMAN,
    COMPUTER
};

void Game::start_xboard()
{
    XBoard xb(alg, board);
    xb.loop();
}

void Game::start_commandline()
{
    int turn = COMPUTER;
    std::cout << "Play first? (y/n)" << std::endl;
    std::string in;
    std::cin >> in;
    if (in[0] == 'y')
        turn = HUMAN;

    board->print_board();
    // alg->root_alphabeta(5, color, -1e9, 1e9);

    while (board->running)
    {
        if (turn == COMPUTER)
        {
            alg->start_alphabeta(color);
        }
        else if (turn == HUMAN)
        {
            board->input_move(color);
            move->check_check(color);
            move->check_check(color);
            board->print_board();
        }
        turn = !turn;
        color = !color;
    }
}

Game::Game(int depth)
{
    board = new Bitboard(WHITE);
    move = new Move(board);
    board->reset();
    //board->user_input_fen();
    alg = new Alphabeta(move, depth);
    Magic::init_hashes();
    move->generate_file_rank();
    move->update_colors();
    move->check_check(WHITE);
    move->check_check(BLACK);
    color = WHITE;
}