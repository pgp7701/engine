#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
uint64_t pow64 (unsigned long int base, unsigned int index) {
	uint64_t val = 1;
	while (1) {
		if (index & 1)
			val *= base;
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

void get_mask(int piece, int * mask) {
	switch (piece) {
		case wr:
		case br:
			memcpy(mask, (const int [9]) {1, 8, 1, 8, 1, 0, 0, 0, 0}, 9 * sizeof(int));
			break;
		case wb: 
		case bb:
			memcpy(mask, (const int [9]) {1, 9, 7, 9, 7, 0, 0, 0, 0}, 9 * sizeof(int));
			break;
		case wq: 
		case bq:
			memcpy(mask, (const int [9]) {1, 8, 1, 8, 1, 9, 7, 9, 7}, 9 * sizeof(int));
			break;
		case wn:
		case bn:
			memcpy(mask, (const int [9]) {0, 6, 10, 6, 10, 15, 17, 15, 17}, 9* sizeof(int));
			break;
		case wk:
		case bk:
			memcpy(mask, (const int [9]) {0, 8, 1, 8, 1, 0, 0, 0, 0}, 9 * sizeof(int));
			break;
		case wp:
		case bp:
		default:
			memcpy(mask, (const int [9]) {0, 0, 0, 0, 0, 0, 0, 0, 0}, 9 * sizeof(int));
	}
}

int get_numshifts(int direction, int length, int origin) {
	int numshifts = 8; //init to 8 so that final numshifts ternary evaluates to false case when length is multiple of 8
	int room = 0;
	int shift = 0;
	if (length % 8) {
		shift = length % 8;
		shift = (shift > 5) ? shift - 8 : shift;
		if (direction == 0) /*leftshift*/ {
			room = (shift > 0) ? (8 - origin % 8) % 8 : ((1 - origin % 8) % 8) - 8;
			numshifts = (room / shift) % 8;
		}
		else if (direction == 1) /*rightshift*/ {
			room = (shift < 0) ? (8 - origin % 8) % 8 : ((1 - origin % 8) % 8) - 8;
			numshifts = (-room / shift) % 8;
		}
	}
	shift = (length - shift) / 8;
	if (direction == 0 && shift)
		room = (7 - ((origin - 1) / 8));
	else if (direction == 1 && shift)
		room = (origin - 1) / 8;
	if (shift)
		numshifts = (room / shift) > numshifts ? numshifts : (room / shift);
	return numshifts;
}

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



int move_gen(BOARD board, MOVE * move_list) {
	int origin;
	uint64_t borigin;
	uint64_t post1; //to be copied to after1
	uint64_t post2; //to be copied to after2
	int move_num = 0;
	int mask[9];
	int num_shifts = 1;
	for (int i = 0; i < 10; ++i) {
		while ((origin = __builtin_ffsll(*(board.pieces[i])))) {
			post1 = *(board.pieces[i]);
			borigin = pow64(2, origin - 1);
			uint64_t interm1 = borigin;
			uint64_t interm2 = borigin;
			get_mask(i, mask);
			if (mask[0]) {
				for (int j = 1; j < 9; ++j) {
					interm1 = borigin;
					interm2 = borigin;
					if (mask[j]) {
						if ((j % 4) % 3) { //testing movedirection
							num_shifts = get_numshifts(0, mask[j], origin);
							for (int k = 0; k < num_shifts; ++k) {
								interm2 = interm2 << mask[j];
								interm1 = interm1 | interm2;
								interm1 = interm1 & board.uoccsquares;
								if (interm1) {
									move_list = realloc(move_list, (move_num + 1) * sizeof(MOVE));
									move_list[move_num].type1 = i;
									move_list[move_num].type2 = nothing;
									post1 = *(board.pieces[i]) ^ borigin;
									post1 = post1 ^ interm1;
									post2 = 0;
									move_list[move_num].after1 = post1;
									move_list[move_num].after2 = post2;
									++move_num;
								}
								else {
									for (int l = (i + 1) % 2; l < 12; l += 2) {
										if (interm2 & *(board.pieces[l])) {
											move_list = realloc(move_list, (move_num + 1) * sizeof(MOVE));
											move_list[move_num].type1 = i;
											move_list[move_num].type2 = l;
											post1 = *(board.pieces[i]) ^ borigin;
											post1 = post1 ^ interm2;
											post2 = *(board.pieces[l]) ^ interm2;
											move_list[move_num].after1 = post1;
											move_list[move_num].after2 = post2;
											++move_num;
										}
									}
									break;
								}
							}
						}
						if (!((j % 4) % 3)) {
							interm1 = borigin;
							interm2 = borigin;
							num_shifts = get_numshifts(1, mask[j], origin);
							for (int k = 0; k < num_shifts; ++k) {
								interm2 = interm2 >> mask[j];
								interm1 = interm1 | interm2;
								interm1 = interm1 & board.uoccsquares;
								if (interm1) {
									move_list = realloc(move_list, (move_num + 1) * sizeof(MOVE));
									move_list[move_num].type1 = i;
									move_list[move_num].type2 = nothing;
									post1 = *(board.pieces[i]) ^ borigin;
									post1 = post1 ^ interm1;
									post2 = 0;
									move_list[move_num].after1 = post1;
									move_list[move_num].after2 = post2;
									++move_num;
								}
								else {
									for (int l = (i + 1) % 2; l < 12; l += 2) {
										if (interm2 & *(board.pieces[l])) {
											move_list = realloc(move_list, (move_num + 1) * sizeof(MOVE));
											move_list[move_num].type1 = i;
											move_list[move_num].type2 = l;
											post1 = *(board.pieces[i]) ^ borigin;
											post1 = post1 ^ interm2;
											post2 = *(board.pieces[l]) ^ interm2;
											move_list[move_num].after1 = post1;
											move_list[move_num].after2 = post2;
											++move_num;
										}
									}
									break;
								}
							}
						}
					}
				}
				*(board.pieces[i]) = *(board.pieces[i]) ^ borigin;
			}
			else {
				for (int j = 1; j < 9; ++j) {
					if (mask[j] && ((j % 4) % 3) && get_numshifts(0, mask[j], origin)) {
						interm2 = interm2 << mask[j];
						interm1 = interm1 | interm2;
						interm1 = interm1 & board.uoccsquares;
						if (interm1) {
							move_list = realloc(move_list, (move_num + 1) * sizeof(MOVE));
							move_list[move_num].type1 = i;
							move_list[move_num].type2 = nothing;
							post1 = *(board.pieces[i]) ^ borigin;
							post1 = post1 ^ interm1;
							post2 = 0;
							move_list[move_num].after1 = post1;
							move_list[move_num].after2 = post2;
							++move_num;
						}
						else {
							for (int l = (i + 1) % 2; l < 12; l += 2) {
								if (interm2 & *(board.pieces[l])) {
									move_list = realloc(move_list, (move_num + 1) * sizeof(MOVE));
									move_list[move_num].type1 = i;
									move_list[move_num].type2 = l;
									post1 = *(board.pieces[i]) ^ borigin;
									post1 = post1 ^ interm2;
									post2 = *(board.pieces[l]) ^ interm2;
									move_list[move_num].after1 = post1;
									move_list[move_num].after2 = post2;
									++move_num;
								}
							}
						}
					}
					interm1 = borigin;
					interm2 = borigin;
					if (mask[j] && ((j % 4) % 3) && get_numshifts(1, mask[j], origin)) {
						interm2 = interm2 >> mask[j];
						interm1 = interm1 | interm2;
						interm1 = interm1 & board.uoccsquares;
						if (interm1) {
							move_list = realloc(move_list, (move_num + 1) * sizeof(MOVE));
							move_list[move_num].type1 = i;
							move_list[move_num].type2 = nothing;
							post1 = *(board.pieces[i]) ^ borigin;
							post1 = post1 ^ interm1;
							post2 = 0;
							move_list[move_num].after1 = post1;
							move_list[move_num].after2 = post2;
							++move_num;
						}
						else {
							for (int l = (i + 1) % 2; l < 12; l += 2) {
								if (interm2 & *(board.pieces[l])) {
									move_list = realloc(move_list, (move_num + 1) * sizeof(MOVE));
									move_list[move_num].type1 = i;
									move_list[move_num].type2 = l;
									post1 = *(board.pieces[i]) ^ borigin;
									post1 = post1 ^ interm2;
									post2 = *(board.pieces[l]) ^ interm2;
									move_list[move_num].after1 = post1;
									move_list[move_num].after2 = post2;
									++move_num;
								}
							}
						}
					}
				}
				*(board.pieces[i]) = *(board.pieces[i]) ^ borigin;
			}
		}
	}
	return move_num;
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
	BOARD mainboard = {0};
	init_board(&mainboard);
	compute_occ(&mainboard);
	compute_uocc(&mainboard);
	MOVE * moves = (MOVE *) malloc(sizeof(MOVE));
	BOARD cp = mainboard;
	int test = move_gen(mainboard, moves);
	printf("result: %d\n", test);
	for (int i = 0; i < test; ++i)
		printf("move%d: %d, %d\n", i, moves[i].type1, moves[i].type2);
	return 0;
}
