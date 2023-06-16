#include<iostream>
#include<vector>
#include<cstring>

#define U64 uint64_t

namespace Magic{
	U64 generate_square_magic(int square, int bishop);

    const U64 bishop_mask[64]= 
    {
			(0x0040201008040200), (0x0000402010080400), (0x0000004020100A00), (0x0000000040221400),
			(0x0000000002442800), (0x0000000204085000), (0x0000020408102000), (0x0002040810204000),
			(0x0020100804020000), (0x0040201008040000), (0x00004020100A0000), (0x0000004022140000),
			(0x0000000244280000), (0x0000020408500000), (0x0002040810200000), (0x0004081020400000),
			(0x0010080402000200), (0x0020100804000400), (0x004020100A000A00), (0x0000402214001400),
			(0x0000024428002800), (0x0002040850005000), (0x0004081020002000), (0x0008102040004000),
			(0x0008040200020400), (0x0010080400040800), (0x0020100A000A1000), (0x0040221400142200),
			(0x0002442800284400), (0x0004085000500800), (0x0008102000201000), (0x0010204000402000),
			(0x0004020002040800), (0x0008040004081000), (0x00100A000A102000), (0x0022140014224000),
			(0x0044280028440200), (0x0008500050080400), (0x0010200020100800), (0x0020400040201000),
			(0x0002000204081000), (0x0004000408102000), (0x000A000A10204000), (0x0014001422400000),
			(0x0028002844020000), (0x0050005008040200), (0x0020002010080400), (0x0040004020100800),
			(0x0000020408102000), (0x0000040810204000), (0x00000A1020400000), (0x0000142240000000),
			(0x0000284402000000), (0x0000500804020000), (0x0000201008040200), (0x0000402010080400),
			(0x0002040810204000), (0x0004081020400000), (0x000A102040000000), (0x0014224000000000),
			(0x0028440200000000), (0x0050080402000000), (0x0020100804020000), (0x0040201008040200)
	};
    const U64 rook_mask[64] =
	{
			(0x000101010101017E), (0x000202020202027C), (0x000404040404047A), (0x0008080808080876),
			(0x001010101010106E), (0x002020202020205E), (0x004040404040403E), (0x008080808080807E),
			(0x0001010101017E00), (0x0002020202027C00), (0x0004040404047A00), (0x0008080808087600),
			(0x0010101010106E00), (0x0020202020205E00), (0x0040404040403E00), (0x0080808080807E00),
			(0x00010101017E0100), (0x00020202027C0200), (0x00040404047A0400), (0x0008080808760800),
			(0x00101010106E1000), (0x00202020205E2000), (0x00404040403E4000), (0x00808080807E8000),
			(0x000101017E010100), (0x000202027C020200), (0x000404047A040400), (0x0008080876080800),
			(0x001010106E101000), (0x002020205E202000), (0x004040403E404000), (0x008080807E808000),
			(0x0001017E01010100), (0x0002027C02020200), (0x0004047A04040400), (0x0008087608080800),
			(0x0010106E10101000), (0x0020205E20202000), (0x0040403E40404000), (0x0080807E80808000),
			(0x00017E0101010100), (0x00027C0202020200), (0x00047A0404040400), (0x0008760808080800),
			(0x00106E1010101000), (0x00205E2020202000), (0x00403E4040404000), (0x00807E8080808000),
			(0x007E010101010100), (0x007C020202020200), (0x007A040404040400), (0x0076080808080800),
			(0x006E101010101000), (0x005E202020202000), (0x003E404040404000), (0x007E808080808000),
			(0x7E01010101010100), (0x7C02020202020200), (0x7A04040404040400), (0x7608080808080800),
			(0x6E10101010101000), (0x5E20202020202000), (0x3E40404040404000), (0x7E80808080808000)
	};

	const U64 bishop_magics[64] = {(0x420C80100408202ULL), 
			(0x1204311202260108ULL), (0x2008208102030000ULL), (0x24081001000CAULL), (0x488484041002110ULL), 
			(0x1A080C2C010018ULL), (0x20A02A2400084ULL), (0x440404400A01000ULL), (0x8931041080080ULL), 
			(0x200484108221ULL), (0x80460802188000ULL), (0x4000090401080092ULL), (0x4000011040A00004ULL), 
			(0x20011048040504ULL), (0x2008008401084000ULL), (0x102422A101A02ULL), (0x2040801082420404ULL), 
			(0x8104900210440100ULL), (0x202101012820109ULL), (0x248090401409004ULL), (0x44820404A00020ULL), 
			(0x40808110100100ULL), (0x480A80100882000ULL), (0x184820208A011010ULL), (0x110400206085200ULL), 
			(0x1050010104201ULL), (0x4008480070008010ULL), (0x8440040018410120ULL), (0x41010000104000ULL), 
			(0x4010004080241000ULL), (0x1244082061040ULL), (0x51060000288441ULL), (0x2215410A05820ULL), 
			(0x6000941020A0C220ULL), (0xF2080100020201ULL), (0x8010020081180080ULL), (0x940012060060080ULL), 
			(0x620008284290800ULL), (0x8468100140900ULL), (0x418400AA01802100ULL), (0x4000882440015002ULL), 
			(0x420220A11081ULL), (0x401A26030000804ULL), (0x2184208000084ULL), (0xA430820A0410C201ULL), 
			(0x640053805080180ULL), (0x4A04010A44100601ULL), (0x10014901001021ULL), (0x422411031300100ULL), 
			(0x824222110280000ULL), (0x8800020A0B340300ULL), (0xA8000441109088ULL), (0x404000861010208ULL), 
			(0x40112002042200ULL), (0x2141006480B00A0ULL), (0x2210108081004411ULL), (0x2010804070100803ULL), 
			(0x7A0011010090AC31ULL), (0x18005100880400ULL), (0x8010001081084805ULL), (0x400200021202020AULL), 
			(0x4100342100A0221ULL), (0x404408801010204ULL), (0x6360041408104012ULL)};
	
	const U64 rook_magics[64] = {(0xA8002C000108020ULL), 
			(0x6C00049B0002001ULL), (0x100200010090040ULL), (0x2480041000800801ULL), (0x280028004000800ULL), 
			(0x900410008040022ULL), (0x280020001001080ULL), (0x2880002041000080ULL), (0xA000800080400034ULL), 
			(0x4808020004000ULL), (0x2290802004801000ULL), (0x411000D00100020ULL), (0x402800800040080ULL), 
			(0xB000401004208ULL), (0x2409000100040200ULL), (0x1002100004082ULL), (0x22878001E24000ULL), 
			(0x1090810021004010ULL), (0x801030040200012ULL), (0x500808008001000ULL), (0xA08018014000880ULL), 
			(0x8000808004000200ULL), (0x201008080010200ULL), (0x801020000441091ULL), (0x800080204005ULL), 
			(0x1040200040100048ULL), (0x120200402082ULL), (0xD14880480100080ULL), (0x12040280080080ULL), 
			(0x100040080020080ULL), (0x9020010080800200ULL), (0x813241200148449ULL), (0x491604001800080ULL), 
			(0x100401000402001ULL), (0x4820010021001040ULL), (0x400402202000812ULL), (0x209009005000802ULL), 
			(0x810800601800400ULL), (0x4301083214000150ULL), (0x204026458E001401ULL), (0x40204000808000ULL), 
			(0x8001008040010020ULL), (0x8410820820420010ULL), (0x1003001000090020ULL), (0x804040008008080ULL), 
			(0x12000810020004ULL), (0x1000100200040208ULL), (0x430000A044020001ULL), (0x280009023410300ULL), 
			(0xE0100040002240ULL), (0x200100401700ULL), (0x2244100408008080ULL), (0x8000400801980ULL), 
			(0x2000810040200ULL), (0x8010100228810400ULL), (0x2000009044210200ULL), (0x4080008040102101ULL), 
			(0x40002080411D01ULL), (0x2005524060000901ULL), (0x502001008400422ULL), (0x489A000810200402ULL), 
			(0x1004400080A13ULL), (0x4000011008020084ULL), (0x26002114058042ULL)};

	U64 bishop_hash[64][4096];
	U64 rook_hash[64][4096];

	int num_of_ones(U64 x) {
  		return __builtin_popcountll(x);
	}

	U64 random_U64() {
		U64 u1, u2, u3, u4;
		u1 = (U64)(random()) & 0xFFFF; u2 = (U64)(random()) & 0xFFFF;
		u3 = (U64)(random()) & 0xFFFF; u4 = (U64)(random()) & 0xFFFF;
		return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
	}

	U64 random_U64_fewbits() {
		return random_U64() & random_U64() & random_U64();
	}

	int pop_first_bit(U64 *b) {
		int index = __builtin_ffsll(*b) - 1;
		*b &= ~(1ULL << index);
		return index;
	}

	U64 generate_occupancy(int index, int bits, U64 mask){
		int i, j;
		U64 result = 0ULL;
		for(int i = 0; i < bits; i++){
			j = pop_first_bit(&mask);
			if(index & (1ULL << i)) result |= (1ULL << j);
		}
		return result;
	}

	U64 hash_magic(U64 board, U64 magic, int bits){
		return ((board * magic) >> (64 - bits));
	}
	
	U64 rook_attacks(int sq, U64 block){
		U64 result = 0ULL;
		int row = sq / 8, col = sq % 8, r, f;
		for(r = row+1; r <= 7; r++) {
			result |= (1ULL << (col + r*8));
			if(block & (1ULL << (col + r*8))) break;
		}
		for(r = row-1; r >= 0; r--) {
			result |= (1ULL << (col + r*8));
			if(block & (1ULL << (col + r*8))) break;
		}
		for(f = col+1; f <= 7; f++) {
			result |= (1ULL << (f + row*8));
			if(block & (1ULL << (f + row*8))) break;
		}
		for(f = col-1; f >= 0; f--) {
			result |= (1ULL << (f + row*8));
			if(block & (1ULL << (f + row*8))) break;
		}
		return result;
	}

	U64 bishop_attacks(int sq, U64 block){
		U64 result = 0ULL;
		int row = sq/8, col = sq%8, r, f;
		for(r = row+1, f = col+1; r <= 7 && f <= 7; r++, f++) {
			result |= (1ULL << (f + r*8));
			if(block & (1ULL << (f + r * 8))) break;
		}
		for(r = row+1, f = col-1; r <= 7 && f >= 0; r++, f--) {
			result |= (1ULL << (f + r*8));
			if(block & (1ULL << (f + r * 8))) break;
		}
		for(r = row-1, f = col+1; r >= 0 && f <= 7; r--, f++) {
			result |= (1ULL << (f + r*8));
			if(block & (1ULL << (f + r * 8))) break;
		}
		for(r = row-1, f = col-1; r >= 0 && f >= 0; r--, f--) {
			result |= (1ULL << (f + r*8));
			if(block & (1ULL << (f + r * 8))) break;
		}
		return result;
	}

	void generate_bishop_magics(){
		U64 magic;
		std::cout << "const U64 bishop_magics[64] = {";
		for(int i = 0; i < 64; i++){
			magic = generate_square_magic(i, 1);
			std::cout << "(0x" << std::hex << std::uppercase << magic << "ULL)";
			if(i != 63) std::cout << ", ";
			if(i % 4 == 0) std::cout << std::endl;
		}
		std::cout << "};" << std::endl;
	}

	void generate_rook_magics(){
		U64 magic;
		std::cout << "const U64 rook_magics[64] = {";
		for(int i = 0; i < 64; i++){
			magic = generate_square_magic(i, 0);
			std::cout << "(0x" << std::hex << std::uppercase << magic << "ULL)";
			if(i != 63) std::cout << ", ";
			if(i % 4 == 0) std::cout << std::endl;
		}
		std::cout << "};" << std::endl;
	}

	void init_bishop_hash(){
		U64 mask, magic, occ, attack;
		int bits, index_magic;
		for(int i = 0; i < 64; i++){
			mask = bishop_mask[i];
			magic = bishop_magics[i];
			bits = num_of_ones(mask);
			for(int j = 0; j < (1 << bits); j++){
				occ = generate_occupancy(j, bits, mask);
				index_magic = hash_magic(occ, magic, bits);
				attack = bishop_attacks(i, occ);
				bishop_hash[i][index_magic] = attack;
			}
		}
	}

	void init_rook_hash(){
		U64 mask, magic, occ, attack;
		int bits, index_magic;
		for(int i = 0; i < 64; i++){
			mask = rook_mask[i];
			magic = rook_magics[i];
			bits = num_of_ones(mask);
			for(int j = 0; j < (1 << bits); j++){
				occ = generate_occupancy(j, bits, mask);
				index_magic = hash_magic(occ, magic, bits);
				attack = rook_attacks(i, occ);
				rook_hash[i][index_magic] = attack;
			}
		}
	}

	void init_hashes(){
		init_bishop_hash();
		init_rook_hash();
	}

	U64 find_bishop_moves(U64 sq, U64 all_pieces_mask){
		int square = pop_first_bit(&sq);
		U64 mask = bishop_mask[square] & all_pieces_mask;
		int bits = num_of_ones(bishop_mask[square]);
		U64 magic = bishop_magics[square];
		return bishop_hash[square][hash_magic(mask, magic, bits)];
	}

	U64 find_rook_moves(U64 sq, U64 all_pieces_mask){
		int square = pop_first_bit(&sq);
		U64 mask = rook_mask[square] & all_pieces_mask;
		int bits = num_of_ones(rook_mask[square]);
		U64 magic = rook_magics[square];
		return rook_hash[square][hash_magic(mask, magic, bits)];
	}

	U64 generate_square_magic(int square, int bishop){
		U64 occupancies[4096], attacks[4096], used_attacks[4096];
		U64 magic;
		U64 attack_mask = (bishop)? bishop_mask[square] : rook_mask[square];
		int relevant_bits = num_of_ones(attack_mask);
		int i, j, k, fail;

		int occupancy_indicies = 1 << relevant_bits;

		for(int index = 0; index < occupancy_indicies; index++){
			occupancies[index] = generate_occupancy(index, relevant_bits, attack_mask);
			attacks[index] = (bishop) ? 
				bishop_attacks(square, occupancies[index]) : rook_attacks(square, occupancies[index]);
		}

		for(i = 0; i < 10000000; i++){
			magic = random_U64_fewbits();
			if(num_of_ones((attack_mask * magic) & 0xFF00000000000000ULL) < 6) continue;
			std::memset(used_attacks, 0ULL, sizeof(used_attacks));
			for(j = 0, fail = 0; fail == 0, j < occupancy_indicies; j++){
				k = (int)((occupancies[j] * magic) >> (64 - relevant_bits));
				if(used_attacks[k] == 0ULL) used_attacks[k] = attacks[j];
				else if(used_attacks[k] != attacks[j]) fail = 1;
			};
			if(!fail){
				return magic;
			}
		}
		std::cout << "No Magic found" << std::endl;
		return 0ULL;
	}
};