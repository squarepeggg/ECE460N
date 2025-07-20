/*
    Name 1: Your Name
    UTEID 1: Your UTEID
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N - Lab 5                                           */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         pagetable    page table in LC-3b machine language   */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {                                                 
   IRD,
   COND2, COND1, COND0,
   J5, J4, J3, J2, J1, J0,
   LD_MAR,
   LD_MDR,
   LD_IR,
   LD_BEN,
   LD_REG,
   LD_CC,
   LD_PC,
   GATE_PC,
   GATE_MDR,
   GATE_ALU,
   GATE_MARMUX,
   GATE_SHF,
   PCMUX1, PCMUX0,
   DRMUX,
   SR1MUX,
   ADDR1MUX,
   ADDR2MUX1, ADDR2MUX0,
   MARMUX,
   ALUK1, ALUK0,
   MIO_EN,
   R_W,
   DATA_SIZE,
   LSHF1,
   LD_PSR,
   GATE_PSR,
   PSRMUX,
   GATE_CC,
   CCMUX,
   GATE_SSP,
   GATE_USP,
   SSPMUX1, SSPMUX0,
   USPMUX1, USPMUX0,
   LD_SSP,
   LD_USP,
   LD_VECTOR,
   GATE_VECTOR,
   VECTORMUX1, VECTORMUX0,
   GATE_PTBR,
   GATE_VA,
   LD_VA,
   LD_PFN,
   PFN_MUX,
   GATE_TEMP,
   LD_TEMP,
/* MODIFY: you have to add all your new control signals */
   CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x)           { return(x[IRD]); }
int GetCOND(int *x)          { return((x[COND1] << 1) + x[COND0]); }
int GetJ(int *x)             { return((x[J5] << 5) + (x[J4] << 4) +
				      (x[J3] << 3) + (x[J2] << 2) +
				      (x[J1] << 1) + x[J0]); }
int GetLD_MAR(int *x)        { return(x[LD_MAR]); }
int GetLD_MDR(int *x)        { return(x[LD_MDR]); }
int GetLD_IR(int *x)         { return(x[LD_IR]); }
int GetLD_BEN(int *x)        { return(x[LD_BEN]); }
int GetLD_REG(int *x)        { return(x[LD_REG]); }
int GetLD_CC(int *x)         { return(x[LD_CC]); }
int GetLD_PC(int *x)         { return(x[LD_PC]); }
int GetGATE_PC(int *x)       { return(x[GATE_PC]); }
int GetGATE_MDR(int *x)      { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x)      { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x)   { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x)      { return(x[GATE_SHF]); }
int GetPCMUX(int *x)         { return((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetDRMUX(int *x)         { return(x[DRMUX]); }
int GetSR1MUX(int *x)        { return(x[SR1MUX]); }
int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x)        { return(x[MARMUX]); }
int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]); }
int GetR_W(int *x)           { return(x[R_W]); }
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]); } 
int GetLSHF1(int *x)         { return(x[LSHF1]); }
/* MODIFY: you can add more Get functions for your new control signals */

/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
   There are two write enable signals, one for each byte. WE0 is used for 
   the least significant byte of a word. WE1 is used for the most significant 
   byte of a word. */

#define WORDS_IN_MEM    0x2000 /* 32 frames */ 
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */
int BUS;	/* value of the bus */

typedef struct System_Latches_Struct{

int PC,		/* program counter */
    MDR,	/* memory data register */
    MAR,	/* memory address register */
    IR,		/* instruction register */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P,
    Vector,		/* p condition bit */
    TEMP, // temp to hold psr or pc
    BEN;        /* ben register */

int READY;	/* ready bit */
  /* The ready bit is also latched as you dont want the memory system to assert it 
     at a bad point in the cycle*/

int REGS[LC_3b_REGS]; /* register file. */

int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

int STATE_NUMBER; /* Current State Number - Provided for debugging */ 

/* For lab 4 */
int INTV; /* Interrupt vector register */
int EXCV; /* Exception vector register */
int SSP; /* Initial value of system stack pointer */
int USP; // user SP
int PSR; // PSR
int INT;
/* MODIFY: You
/* MODIFY: you should add here any other registers you need to implement interrupts and exceptions */

/* For lab 5 */
int PTBR; /* This is initialized when we load the page table */
int VA;   /* Temporary VA register */
int PFN; // pfn
/* MODIFY: you should add here any other registers you need to implement virtual memory */

} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/* For lab 5 */
#define PAGE_NUM_BITS 9
#define PTE_PFN_MASK 0x3E00
#define PTE_VALID_MASK 0x0004
#define PAGE_OFFSET_MASK 0x1FF

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {                                                    
    printf("----------------LC-3bSIM Help-------------------------\n");
    printf("go               -  run program to completion       \n");
    printf("run n            -  execute program for n cycles    \n");
    printf("mdump low high   -  dump memory from low to high    \n");
    printf("rdump            -  dump the register & bus values  \n");
    printf("?                -  display this help menu          \n");
    printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                
 if(CYCLE_COUNT == 10){
   CURRENT_LATCHES.INT = 1;
 }
  eval_micro_sequencer();   
  cycle_memory();
  eval_bus_drivers();
  drive_bus();
  latch_datapath_values();

  CURRENT_LATCHES = NEXT_LATCHES;

  CYCLE_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
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
/* Purpose   : Simulate the LC-3b until HALTed.                 */
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

    printf("\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	printf("  0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	fprintf(dumpsim_file, " 0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
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
    printf("Cycle Count  : %d\n", CYCLE_COUNT);
    printf("PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%0.4x\n", BUS);
    printf("MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%0.4x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	fprintf(dumpsim_file, "%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
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
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */ 
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {                 
    FILE *ucode;
    int i, j, index;
    char line[200];

    printf("Loading Control Store from file: %s\n", ucode_filename);

    /* Open the micro-code file. */
    if ((ucode = fopen(ucode_filename, "r")) == NULL) {
	printf("Error: Can't open micro-code file %s\n", ucode_filename);
	exit(-1);
    }

    /* Read a line for each row in the control store. */
    for(i = 0; i < CONTROL_STORE_ROWS; i++) {
	if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
	    printf("Error: Too few lines (%d) in micro-code file: %s\n",
		   i, ucode_filename);
	    exit(-1);
	}

	/* Put in bits one at a time. */
	index = 0;

	for (j = 0; j < CONTROL_STORE_BITS; j++) {
	    /* Needs to find enough bits in line. */
	    if (line[index] == '\0') {
		printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
		       ucode_filename, i);
		exit(-1);
	    }
	    if (line[index] != '0' && line[index] != '1') {
		printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
		       ucode_filename, i, j);
		exit(-1);
	    }

	    /* Set the bit in the Control Store. */
	    CONTROL_STORE[i][j] = (line[index] == '0') ? 0:1;
	    index++;
	}

	/* Warn about extra bits in line. */
	if (line[index] != '\0')
	    printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
		   ucode_filename, i);
    }
    printf("\n");
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
void load_program(char *program_filename, int is_virtual_base) {                   
    FILE * prog;
    int ii, word, program_base, pte, virtual_pc;

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

    if (is_virtual_base) {
      if (CURRENT_LATCHES.PTBR == 0) {
	printf("Error: Page table base not loaded %s\n", program_filename);
	exit(-1);
      }

      /* convert virtual_base to physical_base */
      virtual_pc = program_base << 1;
      pte = (MEMORY[(CURRENT_LATCHES.PTBR + (((program_base << 1) >> PAGE_NUM_BITS) << 1)) >> 1][1] << 8) | 
	     MEMORY[(CURRENT_LATCHES.PTBR + (((program_base << 1) >> PAGE_NUM_BITS) << 1)) >> 1][0];

      printf("virtual base of program: %04x\npte: %04x\n", program_base << 1, pte);
		if ((pte & PTE_VALID_MASK) == PTE_VALID_MASK) {
	      program_base = (pte & PTE_PFN_MASK) | ((program_base << 1) & PAGE_OFFSET_MASK);
   	   printf("physical base of program: %x\n\n", program_base);
	      program_base = program_base >> 1; 
		} else {
   	   printf("attempting to load a program into an invalid (non-resident) page\n\n");
			exit(-1);
		}
    }
    else {
      /* is page table */
     CURRENT_LATCHES.PTBR = program_base << 1;
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
	MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;;
	ii++;
    }

    if (CURRENT_LATCHES.PC == 0 && is_virtual_base) 
      CURRENT_LATCHES.PC = virtual_pc;

    printf("Read %d words from program into memory.\n\n", ii);
}

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */ 
/*             and set up initial state of the machine         */
/*                                                             */
/***************************************************************/
void initialize(char *argv[], int num_prog_files) { 
    int i;
    init_control_store(argv[1]);

    init_memory();
    load_program(argv[2],0);
    for ( i = 0; i < num_prog_files; i++ ) {
	load_program(argv[i + 3],1);
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.PSR = 0;
    CURRENT_LATCHES.PSR = CURRENT_LATCHES.PSR | 0x8002;
    CURRENT_LATCHES.INT = 0;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);
    CURRENT_LATCHES.SSP = 0x3000; /* Initial value of system stack pointer */

/* MODIFY: you can add more initialization code HERE */

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
    if (argc < 4) {
	printf("Error: usage: %s <micro_code_file> <page table file> <program_file_1> <program_file_2> ...\n",
	       argv[0]);
	exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv, argc - 3);

    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
	printf("Error: Can't open dumpsim file\n");
	exit(-1);
    }

    while (1)
	get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code, except for the places indicated 
   with a "MODIFY:" comment.
   You are allowed to use the following global variables in your
   code. These are defined above.

   CONTROL_STORE
   MEMORY
   BUS

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */
/***************************************************************/
int WE0;
int WE1;
int newBus = -1;
int memCycleCount = 0;
int memVal;
int IVET = 0x00000200;
int flag = 1;
int errorPC = 0;
int errorState = 0;
int interruptLevel = 0;
int protFlag = 0;
int unalignedFlag = 0;
int pageFault = 0;
int unknownFlag = 0;
int LS;
int lsFlag = 1;
int signExtendIMM(int number){
   int mask = 0x0010;
   int val = mask & number;
   val = val >> 4;
   if(val){
       number = 0xFFFFFFF0 | number;
       return number;
   } else {
       return number;
   }
}
int signExtend(int number)
{
   int mask = 0x0080;
   int val = mask & number;
   val = val >> 7;
   if (val)
   {
       number = 0xFFFFFF00 | number;
       return number;
   }
   else
   {
       return number;
   }
}
int signExtend2(int number)
{
   int mask = 0x0020;
   int val = mask & number;
   val = val >> 5;
   if (val)
   {
       number = 0xFFFFFFE0 | number;
       return number;
   }
   else
   {
       return number;
   }
}
int signExtend3(int number)
{
   int mask = 0x0100;
   int val = mask & number;
   val = val >> 8;
   if (val)
   {
       number = 0xFFFFFF00 | number;
       return number;
   }
   else
   {
       return number;
   }
}
int signExtend4(int number)
{
   int mask = 0x0400;
   int val = mask & number;
   val = val >> 10;
   if (val)
   {
       number = 0xFFFFFC00 | number;
       return number;
   }
   else
   {
       return number;
   }
}
int protectionChecker(){
   int test;
   test = (errorPC >> 12) & 0x000F;
   if((test == 2 || test == 1 || test == 0)){
       return 1;
   }
   return 0;
}
int getVectorAdder(){
   int result;
   int VECTOR_MUX = 0;
   VECTOR_MUX = CURRENT_LATCHES.MICROINSTRUCTION[VECTORMUX0] | CURRENT_LATCHES.MICROINSTRUCTION[VECTORMUX1] << 1;
   switch (VECTOR_MUX)
   {
       case 0:
           result = 0;
           break;
       case 1:
           return CURRENT_LATCHES.EXCV  = 0x05;
           break;
       case 2:
           return CURRENT_LATCHES.INTV = 0x01;
           break;
       case 3:
       //
       // unaligned
       if(errorState == 26 || errorState == 6 || errorState == 7){
           if((errorPC % 2 == 1)){
               return CURRENT_LATCHES.EXCV  = 0x03;
               break;
           }
       }
       if((Low16bits(CURRENT_LATCHES.PSR) >> 15) == 1 && protectionChecker()){
           return CURRENT_LATCHES.EXCV = 0x04;
           break;
       }
       if(CURRENT_LATCHES.MDR)
       
       break;
   }
 
}
int getAdder(){
   int result;
   int addr1;
           // addr1mux
           int SR1;
       if(CURRENT_LATCHES.MICROINSTRUCTION[SR1MUX]){
           SR1 = (CURRENT_LATCHES.IR & 0x01C0) >> 6;
       } else {
           SR1 = (CURRENT_LATCHES.IR & 0x0E00) >> 9;
       }
           if(CURRENT_LATCHES.MICROINSTRUCTION[ADDR1MUX]){
               addr1 = Low16bits(CURRENT_LATCHES.REGS[SR1]);
           } else {
               addr1 = Low16bits(CURRENT_LATCHES.PC);
           }
           //addr2mux
           int addr2;
           int addr2Mux = Low16bits(CURRENT_LATCHES.MICROINSTRUCTION[ADDR2MUX0] | (CURRENT_LATCHES.MICROINSTRUCTION[ADDR2MUX1] << 1));
           switch(addr2Mux){
               case 0: // 00
                   addr2 = 0;
                   break;
               case 1: // 01
                   addr2 = Low16bits(signExtend2((CURRENT_LATCHES.IR & 0x003F)));
                   break;
               case 2: // 10
                   addr2 = Low16bits(signExtend3((CURRENT_LATCHES.IR & 0x01FF)));
                   break;
               case 3: // 11
                   addr2 = Low16bits(signExtend4((CURRENT_LATCHES.IR & 0x07FF)));
                   break;
           }
           if(CURRENT_LATCHES.MICROINSTRUCTION[LSHF1]){
               addr2 = addr2 << 1;
           }
           return  result = Low16bits(addr1 + addr2); // add or | bits together? // sign extend after?
}
void eval_micro_sequencer() {
   int nextState = -1;
   int jField = CURRENT_LATCHES.MICROINSTRUCTION[J0] | CURRENT_LATCHES.MICROINSTRUCTION[J1] << 1 | CURRENT_LATCHES.MICROINSTRUCTION[J2] << 2 | CURRENT_LATCHES.MICROINSTRUCTION[J3] << 3 | CURRENT_LATCHES.MICROINSTRUCTION[J4] << 4 | CURRENT_LATCHES.MICROINSTRUCTION[J5] << 5;
   int condBits = CURRENT_LATCHES.MICROINSTRUCTION[COND0] | CURRENT_LATCHES.MICROINSTRUCTION[COND1] << 1 | CURRENT_LATCHES.MICROINSTRUCTION[COND2] << 2;
   int newIRD = GetIRD(CURRENT_LATCHES.MICROINSTRUCTION);
   if(CURRENT_LATCHES.INT){
       interruptLevel++;
   }
   
   if(newIRD){
       int opcode = (CURRENT_LATCHES.IR >> 12) & 0x000F;
       if ((opcode == 10 || opcode == 11)){ // unknown 
           unknownFlag = 1;
           nextState = 61;
       }else {
           nextState = opcode; // gets opCode
       }
   } else if(CURRENT_LATCHES.STATE_NUMBER == 59){
        nextState = LS;
        lsFlag = 1;
   }  else if(CURRENT_LATCHES.STATE_NUMBER == 36  && CURRENT_LATCHES.INT != 1){
        if(lsFlag){
            LS = 42;
            lsFlag = 0;
        }
        nextState = 51;
   } else if(CURRENT_LATCHES.STATE_NUMBER == 40 && CURRENT_LATCHES.INT != 1){
        if(lsFlag){
            LS = 49;
            lsFlag = 0;
        }
        nextState = 51;
   } else if(CURRENT_LATCHES.INT != 1 && (CURRENT_LATCHES.STATE_NUMBER == 18 || CURRENT_LATCHES.STATE_NUMBER == 19)){
        if(lsFlag){
            LS = 33;
            lsFlag = 0;
        }
        nextState = 47;
   } else if(CURRENT_LATCHES.INT != 1 && CURRENT_LATCHES.STATE_NUMBER == 43){
        if(lsFlag){
            LS = 18;
            lsFlag = 0;
        }
        nextState = 47;
   } else if(CURRENT_LATCHES.INT != 1 && CURRENT_LATCHES.STATE_NUMBER == 8){
        if(lsFlag){
            LS = 50;
            lsFlag = 0;
        }
        nextState = 47;
   } else if(CURRENT_LATCHES.INT != 1 && CURRENT_LATCHES.STATE_NUMBER == 53){
        if(lsFlag){
            LS = 54;
            lsFlag = 0;
        }
        nextState = 47;
   } else if(CURRENT_LATCHES.INT != 1 && CURRENT_LATCHES.STATE_NUMBER == 2 ){
        if(lsFlag){
            LS = 29;
            lsFlag = 0;
        }
        nextState = 47;
   } else if(CURRENT_LATCHES.INT != 1 && CURRENT_LATCHES.STATE_NUMBER == 3){
        if(lsFlag){
            LS = 24;
            lsFlag = 0;
        }
        nextState = 51;
   } else if(CURRENT_LATCHES.INT != 1 && CURRENT_LATCHES.STATE_NUMBER == 6){
        if(lsFlag){
            LS = 25;
            lsFlag = 0;
        }
        nextState = 47;
   } else if(CURRENT_LATCHES.INT != 1 && CURRENT_LATCHES.STATE_NUMBER == 7){
        if(lsFlag){
            LS = 23;
            lsFlag = 0;
        }
        nextState = 51;
   } else if(CURRENT_LATCHES.INT != 1 && CURRENT_LATCHES.STATE_NUMBER == 15){
        if(lsFlag){
            LS = 28;
            lsFlag = 0;
        }
        nextState = 47;
   }
   else {
       switch(condBits){
           case 0: // pass
               nextState = Low16bits(jField);
               break;
           case 1: // R
               if(CURRENT_LATCHES.READY){
                   nextState = Low16bits(jField + 0x0002);
                   NEXT_LATCHES.READY = 0;
               } else {
                   nextState = Low16bits(jField);
               }
               break;
           case 2: // BEN
               if(CURRENT_LATCHES.BEN){
                   nextState = Low16bits(jField + 0x0004);
               } else {
                   nextState = Low16bits(jField);
               }
               break;
           case 3: // IR[11]
               if((CURRENT_LATCHES.IR >> 11) & 0x0001){
                   nextState = Low16bits(jField + 0x0001);
               } else {
                   nextState = Low16bits(jField);
               }
               break;
           case 4:
               if((CURRENT_LATCHES.PSR >> 15) & 0x0001){
                   nextState = Low16bits(jField + 0x0008);
                  
               } else {
                   nextState = Low16bits(jField);
               }
               break;
           case 6:
               if((interruptLevel)){
                   nextState = Low16bits(jField + 0x0010);
                   interruptLevel--;
               } else {
                   nextState = Low16bits(jField);
               }
       }
      
   }
   NEXT_LATCHES.STATE_NUMBER = Low16bits(nextState);
   memcpy(NEXT_LATCHES.MICROINSTRUCTION,CONTROL_STORE[nextState],sizeof(int) * CONTROL_STORE_BITS);
}
void cycle_memory() {
   // make run 4 cycles for memory
   if(CURRENT_LATCHES.MICROINSTRUCTION[MIO_EN]){
           memCycleCount++;
       if (memCycleCount == 4){
           NEXT_LATCHES.READY = 1;
           return;
       } 
       if(memCycleCount == 5){
           memCycleCount = 0;
            // we bit logic
           // MUST DO m[MAR] is correct + BUS IS CORRECT
           int address = CURRENT_LATCHES.MAR & 0x0001;
           if(CURRENT_LATCHES.MICROINSTRUCTION[R_W]){ // writing
                   if(CURRENT_LATCHES.MICROINSTRUCTION[DATA_SIZE]){ // writing word
                       MEMORY[CURRENT_LATCHES.MAR/2][0] = Low16bits(CURRENT_LATCHES.MDR & 0x00FF);
                       MEMORY[CURRENT_LATCHES.MAR/2][1] = Low16bits((CURRENT_LATCHES.MDR & 0xFF00) >> 8);
                   } else { // writing byte
                       if(!address){
                           MEMORY[CURRENT_LATCHES.MAR/2][0] = Low16bits(CURRENT_LATCHES.MDR & 0x00FF); // byte
                       } else {
                           MEMORY[CURRENT_LATCHES.MAR/2][1] = Low16bits(CURRENT_LATCHES.MDR & 0x00FF);
                       }
                   }
           } else { // reading
               if(CURRENT_LATCHES.MICROINSTRUCTION[DATA_SIZE]){ // read word
                   memVal = (((MEMORY[CURRENT_LATCHES.MAR/2][0]) & 0x00FF) | (((MEMORY[CURRENT_LATCHES.MAR/2][1]) & 0x00FF) << 8));
               } else { // read byte
                   if(!address){
                        memVal = Low16bits(MEMORY[CURRENT_LATCHES.MAR/2][0]);
                   } else {
                        memVal = Low16bits(MEMORY[CURRENT_LATCHES.MAR/2][1]);
                   }
               }
           }
           return;
       }
   }
}
void eval_bus_drivers() {
   if(CURRENT_LATCHES.MICROINSTRUCTION[GATE_SSP]){
       newBus = CURRENT_LATCHES.SSP;
   }
   if(CURRENT_LATCHES.MICROINSTRUCTION[GATE_PSR]){
       newBus = CURRENT_LATCHES.PSR & 0x8003;
   }
   if(CURRENT_LATCHES.MICROINSTRUCTION[GATE_VECTOR]){
       newBus = CURRENT_LATCHES.Vector;
   }
   if(CURRENT_LATCHES.MICROINSTRUCTION[GATE_USP]){
       newBus = CURRENT_LATCHES.USP;
   }
   if(CURRENT_LATCHES.MICROINSTRUCTION[GATE_CC]){
       newBus = CURRENT_LATCHES.N << 2 | CURRENT_LATCHES.Z << 1 | CURRENT_LATCHES.P;
   }
   if(CURRENT_LATCHES.MICROINSTRUCTION[GATE_MARMUX]){
       if(CURRENT_LATCHES.MICROINSTRUCTION[MARMUX]){  // MarMux1
           newBus = getAdder();
       } else {                                       // MarMux 0
           newBus = Low16bits((CURRENT_LATCHES.IR & 0x00FF) << 1);
       }
   } else if(CURRENT_LATCHES.MICROINSTRUCTION[GATE_PC]){
        if(CURRENT_LATCHES.STATE_NUMBER == 63 || CURRENT_LATCHES.STATE_NUMBER == 42 || CURRENT_LATCHES.STATE_NUMBER == 59 || CURRENT_LATCHES.STATE_NUMBER == 60){
            newBus = CURRENT_LATCHES.PC - 2;
        }else {
            newBus = CURRENT_LATCHES.PC;
        }
   } else if(CURRENT_LATCHES.MICROINSTRUCTION[GATE_ALU]){
       int SR1;
       if(CURRENT_LATCHES.MICROINSTRUCTION[SR1MUX]){
           SR1 = CURRENT_LATCHES.REGS[(CURRENT_LATCHES.IR & 0x01C0) >> 6]; // Source
       } else {
           SR1 = CURRENT_LATCHES.REGS[(CURRENT_LATCHES.IR & 0x0E00) >> 9]; // DR
       }
       int SR2;
       int aluResult;
    // issue were having is when we add a negative number to a positive number when neg > pos. we result in a pos
           // this may occur due to the the fact we don't sign extend properly
       if((CURRENT_LATCHES.IR >> 5) & 0x0001){
           SR2 = signExtendIMM(CURRENT_LATCHES.IR & 0x001F); // IMM
       } else {
           SR2 = CURRENT_LATCHES.REGS[CURRENT_LATCHES.IR & 0x0007]; // REG
       }
           switch(GetALUK(CURRENT_LATCHES.MICROINSTRUCTION)){
           case 0:
               aluResult = SR1 + SR2;
           break;
           case 1:
            aluResult = SR1 & SR2;
           break;
           case 2:
               if((CURRENT_LATCHES.IR >> 4) & 0x0001){ // NOT
                   if(SR1){    //  postiive SR1
                       if((SR1 & 0x8000) >> 15){ // NOT
                           SR1 = SR1 | 0xFFFF0000;
                           aluResult = ~SR1;
                       } else {
                           aluResult = ~SR1; //
                       }
                   } else { // zero SR 1
                       aluResult = ~SR1;
                   }
               } else { // XOR
                   aluResult = SR1 ^ SR2;
               }
           break;
           case 3:
           if(CURRENT_LATCHES.STATE_NUMBER == 23){
               aluResult = SR1;
           } else {
               aluResult = SR1 & 0x00FF;
           }
           break;
       }
       newBus = aluResult;
   } else if(CURRENT_LATCHES.MICROINSTRUCTION[GATE_SHF]){
       int SR1;
       if(CURRENT_LATCHES.MICROINSTRUCTION[SR1MUX]){
           SR1 = (CURRENT_LATCHES.IR & 0x01C0) >> 6;
       } else {
           SR1 = (CURRENT_LATCHES.IR & 0x0E00) >> 9;
       }
       int regVal = CURRENT_LATCHES.REGS[SR1];
       int shiftAmount = CURRENT_LATCHES.IR & 0x000F;
       int mask;
       switch((CURRENT_LATCHES.IR >> 4) & 0x3){
           case 0: // LSHFL
               regVal = regVal << shiftAmount;
               break;
           case 1: // RSHFL
               regVal = regVal >> shiftAmount;
               break;
           case 3: // RSHFA
               mask = 0x8000 & regVal;
               mask = mask >> 15;
               int val;
               if (mask)
               {
                   val = regVal; // 1111111
                   for(int i = 0; i < shiftAmount; i++)
                       {
                           val = 0x8000 | (Low16bits(val) >> 1);
                       }
               } else {
                   val = regVal >> shiftAmount;
               }
               regVal = val;
               break;
       }
       newBus = regVal;
   } else if (CURRENT_LATCHES.MICROINSTRUCTION[GATE_MDR]){
       // reading logic block
       // int address = CURRENT_LATCHES.MAR & 0x0001;
       if(CURRENT_LATCHES.STATE_NUMBER == 39){
        NEXT_LATCHES.MDR = CURRENT_LATCHES.MDR | 0x0001;
        newBus = ((CURRENT_LATCHES.MDR & 0x3E00) >> 9);
       } else if(CURRENT_LATCHES.MICROINSTRUCTION[DATA_SIZE]){    // word
           newBus = Low16bits(CURRENT_LATCHES.MDR);   
       } else {       
           newBus = Low16bits(signExtend(CURRENT_LATCHES.MDR & 0x00FF)); // SR[7:0]~SR[7:0]??                                // byte
       }
   } else if(CURRENT_LATCHES.MICROINSTRUCTION[GATE_PTBR]){
        newBus = CURRENT_LATCHES.PTBR + ((CURRENT_LATCHES.VA & 0xFE00) >> 8);
   }else if(CURRENT_LATCHES.MICROINSTRUCTION[GATE_VA]){
        if(CURRENT_LATCHES.STATE_NUMBER == 8){
            newBus = CURRENT_LATCHES.REGS[6];
        } else if(CURRENT_LATCHES.STATE_NUMBER == 40 || CURRENT_LATCHES.STATE_NUMBER == 36){
            newBus = CURRENT_LATCHES.REGS[6] - 2;
        } else if(CURRENT_LATCHES.STATE_NUMBER == 53){
            newBus = CURRENT_LATCHES.REGS[6] + 2;
        }
   }else if(CURRENT_LATCHES.MICROINSTRUCTION[GATE_TEMP]){
        newBus = CURRENT_LATCHES.REGS[6] - 2;
   }
}

void drive_bus() {
    
    if(CURRENT_LATCHES.STATE_NUMBER == 39){
        if(!(newBus & 0x0004) >> 2){
            pageFault = 1;
        }
    }

   if(CURRENT_LATCHES.MICROINSTRUCTION[GATE_MARMUX]){
        // protection 
        if(CURRENT_LATCHES.STATE_NUMBER == 39 || CURRENT_LATCHES.STATE_NUMBER == 62){
            if(CURRENT_LATCHES.PSR >> 15){
                if(newBus <= 0x2FFF){
                    protFlag = 1;
                }
            }
        }
       // unaligned access exception
       if( CURRENT_LATCHES.STATE_NUMBER == 47){
               if(newBus % 2){
                   unalignedFlag = 1;
               }
       }
       BUS = Low16bits(newBus);
   } else if(CURRENT_LATCHES.MICROINSTRUCTION[GATE_SSP]){
       BUS = Low16bits(newBus);
   } else if(CURRENT_LATCHES.MICROINSTRUCTION[GATE_VECTOR]){
       BUS = Low16bits(newBus);
   } else if(CURRENT_LATCHES.MICROINSTRUCTION[GATE_CC]){
       BUS = Low16bits(newBus);
   } else if(CURRENT_LATCHES.MICROINSTRUCTION[GATE_PSR]){
       BUS = Low16bits(newBus);
   } else if(CURRENT_LATCHES.MICROINSTRUCTION[GATE_USP]){
       BUS = Low16bits(newBus);
   }else if(CURRENT_LATCHES.MICROINSTRUCTION[GATE_PC]){
       BUS = Low16bits(newBus);
   } else if(CURRENT_LATCHES.MICROINSTRUCTION[GATE_SHF]){
       BUS = Low16bits(newBus);
   } else if(CURRENT_LATCHES.MICROINSTRUCTION[GATE_MDR]){
    
    if(CURRENT_LATCHES.STATE_NUMBER == 39){
        if(!CURRENT_LATCHES.MDR & 0x0008){ // protection
            protFlag = 1;
        } else if (!CURRENT_LATCHES.MDR & 0x0004){// page fault
            pageFault = 1;
        } 
        BUS = Low16bits(newBus | 0x0001);
    } else if(CURRENT_LATCHES.STATE_NUMBER == 62 || NEXT_LATCHES.STATE_NUMBER == 57){
        if(!CURRENT_LATCHES.MDR & 0x0008){ // protection
            protFlag = 1;
        } else if (!CURRENT_LATCHES.MDR & 0x0004){// page fault
            pageFault = 1;
        } 
        BUS = Low16bits(newBus | 0x0003);
    } else {
        BUS = Low16bits(newBus);
    }
       
   } else if (CURRENT_LATCHES.MICROINSTRUCTION[GATE_ALU]){
       BUS = Low16bits(newBus);
   } else if (CURRENT_LATCHES.MICROINSTRUCTION[GATE_TEMP]){
        BUS = Low16bits(newBus);
   }  else if (CURRENT_LATCHES.MICROINSTRUCTION[GATE_VA]){
        BUS = Low16bits(newBus);
   }  else if (CURRENT_LATCHES.MICROINSTRUCTION[GATE_PTBR]){
    if( CURRENT_LATCHES.STATE_NUMBER == 47){
               if(newBus % 2){
                   unalignedFlag = 1;
               }
       }
        BUS = Low16bits(newBus);
   } else { 
       BUS = 0;
   }
   if(unalignedFlag){
    NEXT_LATCHES.STATE_NUMBER = 58; // maybe?
       errorPC = newBus;
       errorState = CURRENT_LATCHES.STATE_NUMBER;
       memcpy(NEXT_LATCHES.MICROINSTRUCTION,CONTROL_STORE[NEXT_LATCHES.STATE_NUMBER],sizeof(int) * CONTROL_STORE_BITS);
       flag = 0;
       unalignedFlag = 0;
       protFlag = 0;
       pageFault = 0;
   }
   if(protFlag){
        NEXT_LATCHES.STATE_NUMBER = 58;
       errorPC = newBus;
       errorState = CURRENT_LATCHES.STATE_NUMBER;
       memcpy(NEXT_LATCHES.MICROINSTRUCTION,CONTROL_STORE[NEXT_LATCHES.STATE_NUMBER],sizeof(int) * CONTROL_STORE_BITS);
       flag = 0;
       protFlag = 0;
       pageFault = 0;
   } else if(pageFault){
    NEXT_LATCHES.STATE_NUMBER = 58;
       errorPC = newBus;
       errorState = CURRENT_LATCHES.STATE_NUMBER;
       memcpy(NEXT_LATCHES.MICROINSTRUCTION,CONTROL_STORE[NEXT_LATCHES.STATE_NUMBER],sizeof(int) * CONTROL_STORE_BITS);
       flag = 0;
       pageFault = 0;
   } else if(unknownFlag){
    NEXT_LATCHES.STATE_NUMBER = 61;
       errorPC = newBus;
       errorState = CURRENT_LATCHES.STATE_NUMBER;
       memcpy(NEXT_LATCHES.MICROINSTRUCTION,CONTROL_STORE[NEXT_LATCHES.STATE_NUMBER],sizeof(int) * CONTROL_STORE_BITS);
       flag = 0;
       unknownFlag = 0;
   }
}


    
void latch_datapath_values() {
    // if an exception skip with flag bit    
 if(flag){
   int DR1;
   // find DR register
   if(!CURRENT_LATCHES.MICROINSTRUCTION[DRMUX]){
       DR1 = Low16bits((CURRENT_LATCHES.IR & 0x0E00) >> 9);
   } else {
       DR1 = 0x0007;
   }
   if(CURRENT_LATCHES.MICROINSTRUCTION[LD_PSR]){
    if(NEXT_LATCHES.STATE_NUMBER == 36){
        NEXT_LATCHES.PSR = NEXT_LATCHES.PSR & 0x0003;
    } else {
        switch(CURRENT_LATCHES.MICROINSTRUCTION[PSRMUX]){
           case 0:
               NEXT_LATCHES.PSR = CURRENT_LATCHES.PSR & 0x00000003; 
               break;
           case 1:
               NEXT_LATCHES.PSR = BUS; 
               break;
       }
       if(CURRENT_LATCHES.STATE_NUMBER == 52){
        int bits = CURRENT_LATCHES.PSR;
        switch(bits){
            case 0:
            NEXT_LATCHES.Z = 0;
            NEXT_LATCHES.N = 0;
            NEXT_LATCHES.P = 1;
            break;
            case 1:
            NEXT_LATCHES.Z = 1;
            NEXT_LATCHES.N = 0;
            NEXT_LATCHES.P = 0;
            break;
            case 2:
            NEXT_LATCHES.Z = 0;
            NEXT_LATCHES.N = 1;
            NEXT_LATCHES.P = 0;
            break;
        }
       }
    }
       
   }
   if(CURRENT_LATCHES.MICROINSTRUCTION[LD_VECTOR]){
       NEXT_LATCHES.Vector = IVET + (getVectorAdder() << 1);
   }
   if(CURRENT_LATCHES.MICROINSTRUCTION[LD_USP]){
       int uspmux = CURRENT_LATCHES.MICROINSTRUCTION[USPMUX0] | CURRENT_LATCHES.MICROINSTRUCTION[USPMUX1] << 1;
       switch(uspmux){
           case 0:
           NEXT_LATCHES.REGS[6] = Low16bits(CURRENT_LATCHES.REGS[6] - 2);
           break;
           case 1:
           NEXT_LATCHES.REGS[6] = Low16bits(CURRENT_LATCHES.REGS[6] + 2);
           break;
           case 2:
           NEXT_LATCHES.USP = Low16bits(CURRENT_LATCHES.REGS[6]);
           break;
       }
     
   }
   
   if(CURRENT_LATCHES.MICROINSTRUCTION[LD_SSP]){
        int SSPmux = CURRENT_LATCHES.MICROINSTRUCTION[SSPMUX0] | CURRENT_LATCHES.MICROINSTRUCTION[SSPMUX1] << 1;
       switch(SSPmux){
           case 0:
           NEXT_LATCHES.REGS[6] = Low16bits(CURRENT_LATCHES.REGS[6] - 2);
           break;
           case 1:
           NEXT_LATCHES.REGS[6] = Low16bits(CURRENT_LATCHES.REGS[6] + 2);
           break;
           case 2:
           NEXT_LATCHES.SSP = Low16bits(CURRENT_LATCHES.REGS[6]);
           break;
       }
   }
   if(CURRENT_LATCHES.MICROINSTRUCTION[LD_MAR]){
    if(NEXT_LATCHES.STATE_NUMBER == 49 || NEXT_LATCHES.STATE_NUMBER == 38 || NEXT_LATCHES.STATE_NUMBER == 50 || NEXT_LATCHES.STATE_NUMBER == 54 || NEXT_LATCHES.STATE_NUMBER == 55){
        NEXT_LATCHES.MAR = Low16bits(NEXT_LATCHES.REGS[6]);
    } else if(CURRENT_LATCHES.MICROINSTRUCTION[PFN_MUX]){
        NEXT_LATCHES.MAR = (CURRENT_LATCHES.VA & 0x01FF) + (CURRENT_LATCHES.PFN << 9);
    }
    else {
        NEXT_LATCHES.MAR = Low16bits(BUS);
        if(CURRENT_LATCHES.STATE_NUMBER == 2 || CURRENT_LATCHES.STATE_NUMBER == 3 || CURRENT_LATCHES.STATE_NUMBER == 6 || CURRENT_LATCHES.STATE_NUMBER == 7 || CURRENT_LATCHES.STATE_NUMBER == 15){
            NEXT_LATCHES.VA = BUS;
        }
    }
       
   }
       int address = CURRENT_LATCHES.MAR & 0x0001;
   // storing logic block
   if(CURRENT_LATCHES.MICROINSTRUCTION[LD_MDR]){
       if(CURRENT_LATCHES.MICROINSTRUCTION[MIO_EN]){
           if(CURRENT_LATCHES.READY){
               if(CURRENT_LATCHES.MICROINSTRUCTION[DATA_SIZE]){
                   NEXT_LATCHES.MDR = Low16bits(memVal);
               } else {
                   NEXT_LATCHES.MDR = Low16bits(memVal & 0x00FF); // sign extend
               }
           }
       } else {
           if(CURRENT_LATCHES.MICROINSTRUCTION[DATA_SIZE]){
               NEXT_LATCHES.MDR = Low16bits(BUS);
           } else {
               NEXT_LATCHES.MDR = Low16bits(BUS & 0x00FF);
           }
       }
   }
   if(CURRENT_LATCHES.MICROINSTRUCTION[LD_BEN]){
       NEXT_LATCHES.BEN = ((CURRENT_LATCHES.N && ((CURRENT_LATCHES.IR & 0x0800) >> 11)) |
                           (CURRENT_LATCHES.Z && ((CURRENT_LATCHES.IR & 0x0400) >> 10)) |
                           (CURRENT_LATCHES.P && ((CURRENT_LATCHES.IR & 0x0200) >> 9)));
   }
   if(CURRENT_LATCHES.MICROINSTRUCTION[LD_IR]){
       NEXT_LATCHES.IR = Low16bits(BUS);
   }
   if(CURRENT_LATCHES.MICROINSTRUCTION[LD_PC]){
       int pcMUX = ((CURRENT_LATCHES.MICROINSTRUCTION[PCMUX0]) | (CURRENT_LATCHES.MICROINSTRUCTION[PCMUX1] << 1));
           switch(pcMUX){
               case 0: // pc + 2
                   NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 2);
                   break;
               case 1: // bus
                   NEXT_LATCHES.PC = Low16bits(BUS);
                   break;
               case 2: // adder
                   NEXT_LATCHES.PC = getAdder();
               break;
               case 3:
               NEXT_LATCHES.PC = CURRENT_LATCHES.PC - 2;
               break;
           }
       }
   if(CURRENT_LATCHES.MICROINSTRUCTION[LD_REG]){

    if(CURRENT_LATCHES.STATE_NUMBER == 26){
        NEXT_LATCHES.REGS[6] = CURRENT_LATCHES.USP;
    } else if(CURRENT_LATCHES.STATE_NUMBER == 44){
        NEXT_LATCHES.REGS[6] = CURRENT_LATCHES.SSP;
    } else {
        NEXT_LATCHES.REGS[DR1] = Low16bits(BUS) & 0xFFFF;
    }
   }
   if(CURRENT_LATCHES.MICROINSTRUCTION[LD_CC]){
       if((newBus & 0x8000)){
           newBus = 0xFFFF0000 | newBus;
       }
       if(newBus > 0){
           NEXT_LATCHES.N = 0;
           NEXT_LATCHES.Z = 0;
           NEXT_LATCHES.P = 1;
       } else if(newBus < 0){
           NEXT_LATCHES.N = 1;
           NEXT_LATCHES.Z = 0;
           NEXT_LATCHES.P = 0;
       } else {
           NEXT_LATCHES.N = 0;
           NEXT_LATCHES.Z = 1;
           NEXT_LATCHES.P = 0;
       }
   }

    if(CURRENT_LATCHES.MICROINSTRUCTION[LD_TEMP]){
    NEXT_LATCHES.TEMP = BUS;;
   }
   if(CURRENT_LATCHES.MICROINSTRUCTION[LD_VA]){
    NEXT_LATCHES.VA = BUS;
   }
   if(CURRENT_LATCHES.MICROINSTRUCTION[LD_PFN]){
    NEXT_LATCHES.PFN = BUS;
   }


 }
 flag = 1;
}

