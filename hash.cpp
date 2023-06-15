#include "bitboard.cpp"
#include "magic.cpp"

class ZHash{
    public:
    void init_z_hash();
    U64 hash_board_U64(Bitboard b, int color);
    void allocate_table(int mb);
    void clear_table();
    void delete_table();
    U64 count_entries();
    ZHash(int size_mb);

    private:
    U64 bit_table[64][12];
    U64 depth[20];
    U64 black_move;
    U64 castling_flags[2][2];
    U64 en_p_flags[2];

    int *hash_table;
    U64 number_entries;
};

void ZHash::init_z_hash(){
    for(int i = 0; i < 64; i++){
        for(int j = 0; j < 12; j++){
            bit_table[i][j] = Magic::random_U64();
        }
    }
    black_move = Magic::random_U64();
    for(auto &i : depth) i = Magic::random_U64();
}

U64 ZHash::hash_board_U64(Bitboard b, int color){
    U64 result = 0ULL;
    int index;
    U64 copy;
    if(color) result ^= black_move;
    for(int i = 0; i < 12; i++){
        copy = b.board[i];
        while(copy != 0ULL){
            index = Magic::pop_first_bit(&copy);
            result ^= bit_table[index][i];
        }
    }
    return result;
}

void ZHash::delete_table(){
    delete [] hash_table;
}

void ZHash::clear_table(){
    for(int i = 0; i < number_entries; i++){
        hash_table[i] = 0ULL;
    }
}

void ZHash::allocate_table(int mb){
    U64 size_byte = mb * 1024 * 1024;
    std::size_t entry_size = sizeof(int);
    U64 indices = size_byte / entry_size;
    delete [] hash_table;
    try{
        hash_table = new int[indices];
        number_entries = indices;
    }
    catch (std::bad_alloc&){
        std::cout << "Failed to allocate hash table." << std::endl;
        exit(EXIT_FAILURE);
    }
}

ZHash::ZHash(int size_mb){
    number_entries = 0;
    init_z_hash();
    allocate_table(size_mb);
    clear_table();
}