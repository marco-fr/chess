#include <iostream>
//#include "alphabeta.cpp"
#include "xboard.cpp"

int main(){
    Bitboard b;
    //b.set_board("3n1n2/4P3/8/8/8/8/8/8");
    //b.set_board("1rb1kbQ1/1p1p1nq1/p1p3p1/4p3/2B1P1N1/P7/1P1P1PPP/R1B1K2R");
    b.print_board();
    Move m(&b);
    m.generate_file_rank();
    Magic::init_hashes();
    //b.make_move((1ULL << 63), (1ULL << 59), WHITE);
    m.update_colors();
    m.check_check(WHITE);
    m.check_check(BLACK);

    //Alphabeta alg(&m, 5);
    //std::cout << alg.eval_board(BLACK) << std::endl;
    //std::cout << "EVAL: "<< alg.eval_board(WHITE) << std::endl;
    //b.print_U64(m.find_pawn_legal_moves((1ULL<<34), WHITE));
    //ZHash z(16);
    //b.print_U64(z.hash_board_U64(b, WHITE));
    int color = WHITE;
    XBoard x(&b);
    //std::cout << x.square_to_xboard_move( alg.start_alphabeta(WHITE)) << std::endl;
    //x.xboard_move_to_move("e7d8q", color);
    x.loop();
    /*
    while(b.running){
        b.input_move(BLACK);
        m.check_check(WHITE);
        m.check_check(BLACK);
        b.print_board();
        alg.start_alphabeta(WHITE);
    }*/
    /*
    for(int i = 0; i < 40; i++){
        std::cout << "Color: " << color << std::endl;
        alg.start_alphabeta(color);
        color = !color;
    }*/
            //z.delete_table();
    //std::cin.rdbuf()->setbuf(NULL, 0);
    //std::cin.rdbuf()->in_avail();

    return 0;
}