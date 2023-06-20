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
            alg->start_alphabeta(color, 0);
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

void Game::test()
{
    board->user_input_fen();
    move->check_check(WHITE);
    move->check_check(BLACK);
    color = board->turn;
    board->print_board();
    XBoard xb(alg, board);
    std::cout << alg->depth << std::endl;
    while (alg->depth > 1)
    {
        std::cout << xb.square_to_xboard_move(alg->start_alphabeta(color, 0))
                  << std::endl;
        std::cout << "BOARD EVAL: " << alg->eval_board(color) << std::endl;
        color = !color;
        alg->depth--;
    }
    while (alg->max_quise < 0)
    {
        std::cout << xb.square_to_xboard_move(alg->start_alphabeta(color, 1))
                  << std::endl;
        std::cout << "BOARD EVAL: " << alg->eval_board(color) << std::endl;
        std::cout << "DEPTH: " << alg->depth << std::endl;
        color = !color;
        alg->max_quise++;
    }
    // alg->max_quise++;
    for (int i = 0; i < 10; i++)
    {
        std::cout << xb.square_to_xboard_move(alg->start_alphabeta(color, 1))
                  << std::endl;
        color = !color;
    }
}

Game::Game(int depth, int quise)
{
    board = new Bitboard(WHITE);
    move = new Move(board);
    board->reset();
    // board->user_input_fen();
    alg = new Alphabeta(move, depth, quise);
    Magic::init_hashes();
    move->generate_file_rank();
    move->update_colors();
    move->check_check(WHITE);
    move->check_check(BLACK);
    color = WHITE;
}