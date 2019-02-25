#define NUM_REGS 3
int regs[NUM_REGS];

int prog[] = {0x1064, 0x11c8, 0x2201, 0x0000};

int pc = 0; /* program counter */
int reg[NUM_REGS];
int fetch(){
	return prog[pc++];
}

int instructionN = 0;
int imm = 0;

void decode(int inst){
	instructionN = (inst & 0xF000) >> 12;
	reg[0] = (inst & 0xF00) >> 8;
	reg[1] = (inst & 0xF0) >> 4;
	reg[2] = (inst & 0xF);
	imm = (inst & 0xFF);

}

int running = 1;

void eval(){
	switch(instructionN){
	case 0:
		printf("halt\n");
		running = 0;
		break;
	case 1:
		printf("loadi r%d, #%d\n", reg[0], imm);
		regs[reg[0]] = imm;
		break;
	case 2:
		printf("add r%d r%d r%d\n", reg[0], reg[1], reg[2]);
		regs[reg[0]] = regs[reg[1]] + regs[reg[2]];
		break;
	}
}

void showRegs(){
	int i;
	printf("regs = ");
	for( i=0; i<NUM_REGS; i++ )
	    printf( "%04X ", regs[ i ] );
	  printf( "\n" );
}

void run(){
	while(running){
		showRegs();
		int instr = fetch();
		decode(instr);
		eval();
	}
	showRegs();
}

/*int main(int argc, char **argv){
	int i;
	for(i =0; i < NUM_REGS; i= i+1){
		reg[i] = 0;
	}
	run();
	return 0;
}*/
