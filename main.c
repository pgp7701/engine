#include <stdint.h>

uint64_t pow64 (unsigned int base, unsigned int index) {
	uint64_t val = 1;
	while (true) {
		if (index & 1)
			val *= 2;
		index >>= 1;
		if (index == 0)
			break;
		base *= base;
	}
	return val;
}


typedef struct board {
	uint64_t wking;
	uint64_t bking;
	uint64_t wqueen;
	uint64_t bqueen;
	uint64_t wknight;
	uint64_t bknight;
	uint64_t wbishop;
	uint64_t bbishop;
	uint64_t wrook;
	uint64_t brook;
	uint64_t wpawn;
	uint64_t bpawn;
	int wmov;
	int wpawnmov;
	int bpawnmov;
	uint64_t * pieces[12];
	uint64_t occsquares;
	uint64_t uoccsquares;
} BOARD;

enum PIECE {wk = 0, bk = 1, wq = 2, bq = 3, wn = 4, bn = 5, wb = 6, bb = 7, wr = 8, br = 9, wp = 10, bp = 11, nothing = 12};

typedef struct move {
	enum PIECE type1;
	enum PIECE type2;
	uint64_t after1;
	uint64_t after2;
} MOVE;

void compute_occ(BOARD * cboard) {
	cboard->occsquares = 0;
	for (int i = 0; i < 12; ++i)
		cboard->occsquares = *(cboard->pieces[i])^cboard->occsquares;
}

void compute_uocc(BOARD * cboard) {
	cboard->uoccsquares = 0xFFFFFFFFFFFFFFFF;
	for (int i = 0; i < 12; ++i)
		cboard->uoccsquares = *(cboard->pieces[i])^cboard->uoccsquares;
}

void move_gen(BOARD board, MOVE ** move_list) {
	int origin;
	int destination;
	uint64_t borigin;
	uint64_t post1; //to be copied to after1
	uint64_t post2; //to be copied to after2
	//wrook moves
	while ((origin = __builtin_ffsll(*(board.pieces[wr])))) {
		post1 = board.wrook;
		borigin = pow64(2, origin);
		uint64_t interm1 = borigin;
		uint64_t interm2 = borigin;
		int num_rshifts = origin / 8;
		for (int i = 0; i < num_rshifts; ++i) {
			interm2 = interm2 >> 8;
			interm1 = interm1 | interm2;
		}
		interm2 = borigin;
		int num_lshifts = (64 - origin) / 8;
		for (int i = 0; i < num_lshifts; ++i) {
			interm2 = interm2 << 8;
			interm1 = interm1 | interm2;
		}
		interm2 = borigin;
		num_lshifts = ((origin % 8) + 7) % 8;
		for (int i = 0; i < num_lshifts; ++i) {
			interm2 = interm2 << 1;
			interm1 = interm1 | interm2;
		}
		interm2 = borigin;
		num_rshifts = (8 - (origin % 8)) % 8;
		for (int i = 0; i < num_rshifts; ++i) {
			interm2 = interm2 >> 1;
			interm1 = interm1 | interm2;
		}
	}
}

void init_board(BOARD * c_board) {
	c_board->wmov = 1;
	c_board->wpawnmov = 0x00;
	c_board->bpawnmov = 0x00;
	c_board->wking = 0x0800000000000000;
	c_board->bking = 0x0000000000000008;
	c_board->wqueen = 0x1000000000000000;
	c_board->bqueen = 0x0000000000000010;
	c_board->wknight = 0x4200000000000000;
	c_board->bknight = 0x0000000000000042;
	c_board->wbishop = 0x2400000000000000;
	c_board->bbishop = 0x0000000000000024;
	c_board->wrook = 0x8100000000000000;
	c_board->brook = 0x0000000000000081;
	c_board->wpawn = 0x00FF000000000000;
	c_board->bpawn = 0x000000000000FF00;
	c_board->pieces[0] = &(c_board->wking);
	c_board->pieces[1] = &(c_board->bking);
	c_board->pieces[2] = &(c_board->wqueen);
	c_board->pieces[3] = &(c_board->bqueen);
	c_board->pieces[4] = &(c_board->wknight);
	c_board->pieces[5] = &(c_board->bknight);
	c_board->pieces[6] = &(c_board->wbishop);
	c_board->pieces[7] = &(c_board->bbishop);
	c_board->pieces[8] = &(c_board->wrook);
	c_board->pieces[9] = &(c_board->brook);
	c_board->pieces[10] = &(c_board->wpawn);
	c_board->pieces[11] = &(c_board->bpawn);
}

int main() {
	BOARD mainboard;
	init_board(&mainboard);
	compute_occ(&mainboard);
	compute_uocc(&mainboard);
	return 0;
}
