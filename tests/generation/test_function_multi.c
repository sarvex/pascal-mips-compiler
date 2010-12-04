#include <stdio.h>
#define MEM_MAX 65536
#define NUM_REGS 9
#define GLOBAL_REGION NUM_REGS
#define RS 0
#define RT 1
#define RD 2
#define SP 3
#define HP 4
#define FP 5
#define VA 6
#define OA 7
#define RA 8
int memory[MEM_MAX];

int main(){
	memory[SP] = MEM_MAX-1;
	memory[OA] = GLOBAL_REGION;
	memory[RA] = 0;
	memory[HP] = GLOBAL_REGION+2;
	memory[ memory[SP] ] = memory[FP];
	memory[SP]--;
	memory[ memory[SP] ] = memory[RA];
	memory[SP]--;
	memory[ memory[SP] ] = memory[OA];
	memory[SP]--;
	memory[FP] = memory[SP];
	memory[SP] = memory[SP] + 0;
	goto bbl_2;
bbl_1:
	memory[RD] = memory[FP];
	memory[RD] = memory[RD] - 1;
	memory[ memory[SP] ] = memory [memory[RD]];
	memory[SP]--;
	memory[RD] = memory[FP];
	memory[RD] = memory[RD] - 0;
	memory[ memory[SP] ] = memory [memory[RD]];
	memory[SP]--;
	memory[SP]++;
	memory[RS] = memory[ memory[SP] ];
	memory[RT] = memory[RS];
	memory[SP]++;
	memory[RS] = memory[ memory[SP] ];
	memory[RD] = (memory[RT] + memory[RS]);
	memory[ memory[SP] ] = memory[RD];
	memory[SP]--;
	memory[RD] = memory[OA];
	memory[RD] = memory[RD] + 1;
	memory[SP]++;
	memory[RS] = memory[ memory[SP] ];
	memory[ memory[RD] ] = memory[RS];
	memory[RD] = memory[OA];
	memory[RD] = memory[RD] + 1;
	memory[ memory[SP] ] = memory [memory[RD]];
	memory[SP]--;
	memory[SP]++;
	memory[RS] = memory[ memory[SP] ];
	printf("%d\n", memory[RS]);
	memory[RD] = 5;
	memory[ memory[SP] ] = memory [RD];
	memory[SP]--;
	memory[SP]++;
	memory[RS] = memory[ memory[SP] ];
	memory[VA] = memory[RS];
	if(memory[RA] == 0) goto _ra_1;
bbl_2:
	memory[SP]++;
	memory[RS] = memory[ memory[SP] ];
	memory[RT] = memory[RS];
	memory[SP]++;
	memory[RS] = memory[ memory[SP] ];
	memory[ memory[SP] ] = memory[FP];
	memory[SP]--;
	memory[ memory[SP] ] = memory[RA];
	memory[SP]--;
	memory[ memory[SP] ] = memory[OA];
	memory[SP]--;
	memory[FP] = memory[SP];
	memory[SP] = memory[SP] + 3;
	memory[memory[FP]-0] = +2;
	memory[memory[FP]-1] = +8;
	memory[RA] = 0;
	goto bbl_1;
_ra_1:
	memory[SP] = memory[SP] - 3;
	memory[SP]++;
	memory[RS] = memory[ memory[SP] ];
	memory[OA] = memory[RS];
	memory[SP]++;
	memory[RS] = memory[ memory[SP] ];
	memory[RA] = memory[RS];
	memory[SP]++;
	memory[RS] = memory[ memory[SP] ];
	memory[FP] = memory[RS];
	memory[RD] = memory[VA];
	memory[ memory[SP] ] = memory[RD];
	memory[SP]--;
	memory[RD] = memory[OA];
	memory[RD] = memory[RD] + 0;
	memory[SP]++;
	memory[RS] = memory[ memory[SP] ];
	memory[ memory[RD] ] = memory[RS];
	if(memory[RA] == 0) goto _ra_2;
_ra_2:	return 0;

}
