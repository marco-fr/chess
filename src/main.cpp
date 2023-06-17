#include "game.hpp"
#include <iostream>

int main()
{
    std::cout << "# Chess Engine" << std::endl;

    Game game(6);

    //game.start_commandline();
    game.start_xboard();

    return 0;
}