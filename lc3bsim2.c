/*


    Name 1: Kevin Yee
    Name 2: Stefan Bordovsky
    UTEID 1: UT EID of the first partner
    UTEID 2: UT EID of the second partner
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Instruction Level Simulator                         */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3b machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

#define OP_ADD 0x1
#define OP_AND 0x5
#define OP_BR  0x0
#define OP_JMP 0xC
#define OP_JSR 0x4
#define OP_LDB 0x2
#define OP_LDW 0x6
#define OP_LEA 0xE
#define OP_RTI 0x8
#define OP_SHF 0xD
#define OP_STB 0x3
#define OP_STW 0x7
#define OP_TRAP 0xF
#define OP_XOR 0x9



/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)
#define Low8bits(x) ((x) & 0x00FF)
#define Low4bits 0x000F
/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A
*/

#define WORDS_IN_MEM    0x08000
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */


typedef struct System_Latches_Struct{

  int PC,		/* program counter */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P;		/* p condition bit */
  int REGS[LC_3b_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {
  printf("----------------LC-3b ISIM Help-----------------------\n");
  printf("go               -  run program to completion         \n");
  printf("run n            -  execute program for n instructions\n");
  printf("mdump low high   -  dump memory from low to high      \n");
  printf("rdump            -  dump the register & bus values    \n");
  printf("?                -  display this help menu            \n");
  printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {

  process_instruction();
  CURRENT_LATCHES = NEXT_LATCHES;
  INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {
  int i;

  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating for %d cycles...\n\n", num_cycles);
  for (i = 0; i < num_cycles; i++) {
    if (CURRENT_LATCHES.PC == 0x0000) {
	    RUN_BIT = FALSE;
	    printf("Simulator halted\n\n");
	    break;
    }
    cycle();
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed                 */
/*                                                             */
/***************************************************************/
void go() {
  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating...\n\n");
  while (CURRENT_LATCHES.PC != 0x0000)
    cycle();
  RUN_BIT = FALSE;
  printf("Simulator halted\n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {
  int address; /* this is a byte address */

  printf("\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
  printf("-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    printf("  0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  printf("\n");

  /* dump the memory contents into the dumpsim file */
  fprintf(dumpsim_file, "\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
  fprintf(dumpsim_file, "-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    fprintf(dumpsim_file, " 0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  fprintf(dumpsim_file, "\n");
  fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {
  int k;

  printf("\nCurrent register/bus values :\n");
  printf("-------------------------------------\n");
  printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
  printf("PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  printf("Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
  printf("\n");

  /* dump the state information into the dumpsim file */
  fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
  fprintf(dumpsim_file, "-------------------------------------\n");
  fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
  fprintf(dumpsim_file, "PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  fprintf(dumpsim_file, "Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    fprintf(dumpsim_file, "%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
  fprintf(dumpsim_file, "\n");
  fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {
  char buffer[20];
  int start, stop, cycles;

  printf("LC-3b-SIM> ");

  scanf("%s", buffer);
  printf("\n");

  switch(buffer[0]) {
  case 'G':
  case 'g':
    go();
    break;

  case 'M':
  case 'm':
    scanf("%i %i", &start, &stop);
    mdump(dumpsim_file, start, stop);
    break;

  case '?':
    help();
    break;
  case 'Q':
  case 'q':
    printf("Bye.\n");
    exit(0);

  case 'R':
  case 'r':
    if (buffer[1] == 'd' || buffer[1] == 'D')
	    rdump(dumpsim_file);
    else {
	    scanf("%d", &cycles);
	    run(cycles);
    }
    break;

  default:
    printf("Invalid Command\n");
    break;
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {
  int i;

  for (i=0; i < WORDS_IN_MEM; i++) {
    MEMORY[i][0] = 0;
    MEMORY[i][1] = 0;
  }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {
  FILE * prog;
  int ii, word, program_base;

  /* Open program file. */
  prog = fopen(program_filename, "r");
  if (prog == NULL) {
    printf("Error: Can't open program file %s\n", program_filename);
    exit(-1);
  }

  /* Read in the program. */
  if (fscanf(prog, "%x\n", &word) != EOF)
    program_base = word >> 1;
  else {
    printf("Error: Program file is empty\n");
    exit(-1);
  }

  ii = 0;
  while (fscanf(prog, "%x\n", &word) != EOF) {
    /* Make sure it fits. */
    if (program_base + ii >= WORDS_IN_MEM) {
	    printf("Error: Program file %s is too long to fit in memory. %x\n",
             program_filename, ii);
	    exit(-1);
    }

    /* Write the word to memory array. */
    MEMORY[program_base + ii][0] = word & 0x00FF;
    MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
    ii++;
  }

  if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

  printf("Read %d words from program into memory.\n\n", ii);
}

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) {
  int i;

  init_memory();
  for ( i = 0; i < num_prog_files; i++ ) {
    load_program(program_filename);
    while(*program_filename++ != '\0');
  }
  CURRENT_LATCHES.Z = 1;
  NEXT_LATCHES = CURRENT_LATCHES;

  RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {
  FILE * dumpsim_file;

  /* Error Checking */
  if (argc < 2) {
    printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
           argv[0]);
    exit(1);
  }

  printf("LC-3b Simulator\n\n");

  initialize(argv[1], argc - 1);

  if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
    printf("Error: Can't open dumpsim file\n");
    exit(-1);
  }

  while (1)
    get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code.
   You are allowed to use the following global variables in your
   code. These are defined above.

   MEMORY

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */

/***************************************************************/



int readInstructions(int currentaddress)
{

	/* MEMORY[A][0] stores the least significant byte of word at word address A
	   MEMORY[A][1] stores the most significant byte of word at word address A
	*/
	return Low16bits(((MEMORY[currentaddress][1]<<8) + MEMORY[currentaddress][0]));

}


int sext(int data, int position)
{
/*TODO: Fix sext*/
    int value = Low16bits(data);

	if(value < 0)
	{
		value+= 0xFFFF - (1U <<(position));
	}
	return value;
}

void setcc(int x)
{
    if(x>0) {NEXT_LATCHES.N=0;NEXT_LATCHES.Z=0;NEXT_LATCHES.P=1;}
    if(x==0) {NEXT_LATCHES.N=0;NEXT_LATCHES.Z=1;NEXT_LATCHES.P=0;}
    if(x<0) {NEXT_LATCHES.N=1;NEXT_LATCHES.Z=0;NEXT_LATCHES.P =0;}
}

void process_instruction(){
  /*  function: process_instruction
   *
   *    Process one instruction at a time
   *       -Fetch one instruction
   *       -Decode
   *       -Execute
   *       -Update NEXT_LATCHES
   */

	void execute_add(int);
	void execute_and(int);
	void execute_br(int);
	void execute_jmp(int);
	void execute_jsr(int);
        
        void execute_trap(int);
        void execute_xor(int);
=======
	void execute_ldb(int);
	void execute_ldw(int);
	void execute_lea(int);

	int currentaddress = CURRENT_LATCHES.PC/2;
	int instructions = Low16bits(readInstructions(currentaddress));
	int opCode = (instructions>>12)&Low4bits;

	switch(opCode) {

		case OP_ADD: execute_add(instructions);break;

		case OP_AND: execute_and(instructions);break;
			/*TODO: AND*/
		case OP_BR:  execute_br(instructions);break;
			/*TODO: BR*/
		case OP_JMP: execute_jmp(instructions);break;
			/*TODO: JMP*/
		case OP_JSR: execute_jsr(instructions);break;
			/*TODO JSR*/
		case OP_LDB: execute_ldb(instructions);break;
			/*TODO_LDB*/
		case OP_LDW: execute_ldw(instructions);break;
			/*TODO LDW*/
		case OP_LEA: execute_lea(instructions);break;
			/*TODO:LEA*/
		case OP_SHF:
			/*TODO: SHF*/
		case OP_STB: execute_stb(instructions);break;
			/*TODO:STB*/
		case OP_STW: execute_stw(instructions);break;
			/*TODO:STW*/
		case OP_TRAP: execute_trap(instructions);break;
			/*TODO TRAP*/
		case OP_XOR: execute_xor(instructions);break;
			/*TODO XOR*/
		default:
			printf("Invalid Op Code");

}
}


	void execute_add(int instructions)
	{
		int dr,sr1,sr2,imm5,bit5;
		int data;
		bit5 =(instructions >> 5) & 0x1;
		dr = (instructions >> 9) & 0x7; /*Mask lower three bits*/
		sr1 = (instructions >>6) & 0x7;

		if(bit5)
		{
			sr2 = (instructions >> 0) & 0x7;
			data = CURRENT_LATCHES.REGS[sr1] + CURRENT_LATCHES.REGS[sr2];
			NEXT_LATCHES.REGS[dr] = Low16bits(data);
		}
		else
		{
			imm5 = (instructions >>0) & 0x1F;
			data = CURRENT_LATCHES.REGS[sr1] + sext(imm5,5);
			NEXT_LATCHES.REGS[dr] = Low16bits(data);
		}
		setcc(data);
		NEXT_LATCHES.PC=CURRENT_LATCHES.PC+2;
	}

	void execute_and(int instructions)
	{
		int dr,sr1,sr2,imm5,bit5;
		int data;
		bit5 =(instructions >> 5) & 0x1;
		dr = (instructions >> 9) & 0x7; /*Mask lower three bits*/
		sr1 = (instructions >>6) & 0x7;

		if(bit5)
		{
			sr2 = (instructions >> 0) & 0x7;
			data = CURRENT_LATCHES.REGS[sr1] & CURRENT_LATCHES.REGS[sr2];
			NEXT_LATCHES.REGS[dr] = Low16bits(data);
		}
		else
		{
			imm5 = (instructions >>0) & 0x1F;
			data = CURRENT_LATCHES.REGS[sr1] & sext(imm5,5); /*TODO: Not sure if this is correct*/
			NEXT_LATCHES.REGS[dr] = Low16bits(data);
		}
		setcc(data);
		NEXT_LATCHES.PC=CURRENT_LATCHES.PC+2;
	}


	void execute_br(int instructions)
	{
		int NBit = instructions & 0x0800;
		int ZBit = instructions & 0x0400;
		int PBit = instructions & 0x0200;
		int Unconditional = ~(instructions & 0xE00);
		int PCOffset9 = instructions & 0x01FF;
		if((CURRENT_LATCHES.N && NBit)||
				(CURRENT_LATCHES.Z && ZBit)||
				(CURRENT_LATCHES.P && PBit)||
				Unconditional){
					NEXT_LATCHES.PC=CURRENT_LATCHES.PC+2+sext(PCOffset9, 9);
				}
		else{
			NEXT_LATCHES.PC = CURRENT_LATCHES.PC+2;
		}
	}

	void execute_jmp(int instructions)
	{
		if(instructions & 0x1C0)
		{
			/*Return*/
			NEXT_LATCHES.PC = CURRENT_LATCHES.REGS[7];
		}
		else
		{
			int BaseR = (instructions >> 6) & 0x7;
			NEXT_LATCHES.PC = CURRENT_LATCHES.REGS[BaseR];
		}
	}

	void execute_jsr(int instructions)
	{
		NEXT_LATCHES.PC  = NEXT_LATCHES.REGS[7];


		if(instructions & 0x800)
		{
			int BaseR = (instructions >> 6) & 0x7;
			NEXT_LATCHES.PC = CURRENT_LATCHES.REGS[BaseR];
		}
		else
		{
			int PCOffset11 = (instructions)&0XFFF;
			NEXT_LATCHES.PC = CURRENT_LATCHES.PC + (sext(PCOffset11,11)<1);
		}

	}
        
        void execute_stb(int instructions)
        {
            int sr, baser, offset6, memLocation, data, baserData;
            sr = (instructions >> 9) & 0x7;
            data = CURRENT_LATCHES.REGS[sr] & 0xFF;
            baser = (instructions >> 6) & 0x7;
            baserData = CURRENT_LATCHES.REGS[baser];
            offset6 = (instructions >> 0) & 0x3F; 
            memLocation = sext(offset6, 6) + baserData;
            if(memLocation & 0x1) /* If memory location is an odd number, index to MEMORY[x][1] */
            {
                MEMORY[Low16bits(memLocation >> 1)][1] = data;
            }
            else
            {
                MEMORY[Low16bits(memLocation >> 1)][0] = data;
            }
        }
        
        void execute_stw(int instructions)
        {
            int sr, baser, offset6, memLocation, srData, baserData;
            sr = (instructions >> 9) & 0x7;
            srData = CURRENT_LATCHES.REGS[sr];
            baser = (instructions >> 6) & 0x7;
            baserData = CURRENT_LATCHES.REGS[baser];
            offset6 = (instructions >> 0) & 0x3F;
            memLocation = sext(offset6, 6) + baserData; /* Check left shift. Probably don't need because of use of 2D Mem array. */
            MEMORY[memLocation][0] = srData & 0xFF;
            MEMORY[memLocation][1] = (srData & 0xFF00) >> 8;
        }
        
        void execute_trap(int instructions)
        {
            int trapvect8, PC, memLocation;
            PC = CURRENT_LATCHES.PC + 2;
            NEXT_LATCHES.REGS[7] = PC;
            trapvect8 = (instructions >> 0) & 0xFF; /* Check left shift. Probably don't need because of use of 2D Mem array. */
            PC = Low16bits(((MEMORY[trapvect8][1] << 8) + MEMORY[trapvect8][0]));
            NEXT_LATCHES.PC = PC;
        }
        
        void execute_xor(int instructions)
        {
            int dr,sr1,sr2,imm5,bit5;
            bit5 = (instructions >> 5) & 0x1;
            dr = (instructions >> 9) & 0x7; /*Mask lower three bits*/
            sr1 = (instructions >> 6) & 0x7;
            if(!bit5) /* Want immediate XOR or NOT */
            { /* DR = SR1 XOR SR2 */
                sr2 = (instructions >> 0) & 0x7;
                data = CURRENT_LATCHES.REGS[sr1] ^ CURRENT_LATCHES.REGS[sr2];
                NEXT_LATCHES.REGS[dr] = Low16bits(data);
            }
            else
            { /* DR = SR1 XOR SEXT(imm5) */
                imm5 = (instructions >> 0) & 0x1F;
                data = CURRENT_LATCHES.REGS[sr1] ^ sext(imm5,5);
                NEXT_LATCHES.REGS[dr] = Low16bits(data);
            }
            setcc(data);
            NEXT_LATCHES.PC=CURRENT_LATCHES.PC+2;
        }



	void execute_ldb(int instructions)
	{


		int dr, baser,boffset6,data;

		dr = (instructions>>9)&0x7;
		baser= (instructions>>9)&0x7;
		boffset6 = instructions & 0x3F;

		int memLocation = CURRENT_LATCHES.REGS[baser] + sext(boffset6,6);
		if(memLocation % 2)
		{
			data = Low8bits(MEMORY[memLocation][0]);
		}
		else
		{
			data = Low8bits(MEMORY[memLocation][1]);
		}


		NEXT_LATCHES.REGS[dr] = sext(data,8);
        setcc(data);

        NEXT_LATCHES.PC=CURRENT_LATCHES.PC+2;
	}



	void execute_ldw(int instructions)
	{


		int dr, baser,boffset6;

		dr = (instructions>>9)&0x7;
		baser= (instructions>>9)&0x7;
		boffset6 = instructions & 0x3F;

		int memLocation = CURRENT_LATCHES.REGS[baser] + (sext(boffset6,6));/*TODO Check left shift*/
		int data = Low16bits(((MEMORY[memLocation][1]<<8) + MEMORY[memLocation][0]));
		NEXT_LATCHES.REGS[dr] = data;
        setcc(data);

        NEXT_LATCHES.PC=CURRENT_LATCHES.PC+2;
	}


	void execute_lea(int instructions)
	{


		int dr, pcoffset9;

		dr = (instructions>>9)&0x7;
		pcoffset9 = (instructions)&0x3FF;


		dr = CURRENT_LATCHES.PC + (sext(pcoffset9,9)); /*TODO Check left shift*/

        NEXT_LATCHES.PC=CURRENT_LATCHES.PC+2;
	}


	void execute_shf(int instructions)
	{
		int dr,sr,amount4;

		dr = (instructions>9)&0x7;
		sr = (instructions>>6)&0x07;
		amount4 = (instructions)&0xF;

		if(!(instructions&0x10))
		{
			/*left shift*/

			int leftshift = CURRENT_LATCHES.REGS[sr]<<amount4;
		}
		else
		{

			if(!(instructions &0x20))
			{
				int rightshiftLogical = CURRENT_LATCHES.REGS[sr]>>amount4;
			}
			else
			{
				int rightshiftArithmetic = CURRENT_LATCHES.REGS[sr]>>amount4
			}
		}
	}
