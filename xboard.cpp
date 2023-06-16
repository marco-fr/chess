#include <iostream>
#include <string>
#include <vector>
#include <csignal>
#include <sstream>
#include "alphabeta.cpp"

class XBoard{
    public:
    void loop();
    void process_command(std::string line);
    std::string square_to_xboard_move(Alphabeta::eval_move result);
    void xboard_move_to_move(std::string s, int color);
    XBoard(Bitboard *bit);
    private:
    Bitboard *b;
};

std::string XBoard::square_to_xboard_move(Alphabeta::eval_move result){
    std::string conversion = "abcdefgh";
    int from_ind = Magic::pop_first_bit(&result.from);
    int to_ind = Magic::pop_first_bit(&result.to);
    int fr = 8 - from_ind / 8, fc = from_ind % 8;
    int tr = 8 - to_ind / 8, tc = to_ind % 8;
    std::string send = (conversion[fc] + std::to_string(fr) + conversion[tc] + std::to_string(tr));
    if(result.promote != -1){
        send += b->char_from_piece(result.promote);
    }
    return send;
}

void XBoard::xboard_move_to_move(std::string s, int color){
    int fr = 8 - (int) (s[1] - '0'), tr = 8 - (int) (s[3] - '0');
    int fc = (int) (s[0] - 'a');
    int tc = (int) (s[2] - 'a');
    int from = fr * 8 + fc;
    int to = tr * 8 + tc;
    U64 f = (1ULL << from), t = (1ULL << to);
    b->make_easy_move(f, t, color);
    if(s.length() > 4){
        b->promote(t, color, b->piece_from_character(s[4]) + 6 * color);
    }
    //std::cout << "test " << from << " " << fc << std::endl;
}

void XBoard::loop(){
    std::string line, token;
    signal(SIGINT, SIG_IGN);
    std::cout.setf(std::ios::unitbuf);

    //b.set_board("r3k2r/pppp1ppp/1nbq1bn1/8/4p3/1NBQ1BN1/PPPPPPPP/R3K2R");
    //b.set_board("1rb1kbQ1/1p1p1nq1/p1p3p1/4p3/2B1P1N1/P7/1P1P1PPP/R1B1K2R");
    //b.print_board();
    Move m(b);
    m.generate_file_rank();
    Magic::init_hashes();
    //b.make_move((1ULL << 63), (1ULL << 59), WHITE);
    //std::cout << square_to_xboard_move((1ULL << 51), (1ULL << 36));
    m.update_colors();
    m.check_check(WHITE);
    m.check_check(BLACK);

    Alphabeta alg(&m, 5);
    int color = WHITE;

    while (std::getline(std::cin, line)) {
        std::istringstream is(line);
        token.clear();
        is >> std::skipws >> token;
        if(token == "protover"){
            std::cout << "feature usermove=1" << std::endl;
            std::cout << "done=1" << std::endl;
        }
        else if(token == "new"){
            b->reset();
            color = WHITE;
        }
        else if(token == "go"){
            std::cout << "feature usermove=1" << std::endl;
            Alphabeta::eval_move result = alg.start_alphabeta(color);
            std::cout << "move " << square_to_xboard_move(result)<< std::endl;
            color = !color;
        }
        else if(token == "usermove"){
            is >> std::skipws >> token;
            xboard_move_to_move(token, color);
            color = !color;
            Alphabeta::eval_move result = alg.start_alphabeta(color);
            std::cout << "move " << square_to_xboard_move(result) << std::endl;
            color = !color;
            //std::cout << "#" << token << std::endl;
        }

    }
}

XBoard::XBoard(Bitboard *bit){
    b = bit;
}