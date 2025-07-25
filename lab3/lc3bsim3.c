/*
    Name 1: Matthew Olan
    UTEID 1: MTO472
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
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
    COND1, COND0,
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

#define WORDS_IN_MEM    0x08000 
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
    P,		/* p condition bit */
    BEN;        /* ben register */

int READY;	/* ready bit */
  /* The ready bit is also latched as you dont want the memory system to assert it 
     at a bad point in the cycle*/

int REGS[LC_3b_REGS]; /* register file. */

int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

int STATE_NUMBER; /* Current State Number - Provided for debugging */ 
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

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
    printf("Cycle Count  : %d\n", CYCLE_COUNT);
    printf("PC           : 0x%.4x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%.4x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%.4x\n", BUS);
    printf("MDR          : 0x%.4x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%.4x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%.4x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%.4x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%.4x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%.4x\n", CURRENT_LATCHES.MAR);
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

/************************************************************/
/*                                                          */
/* Procedure : init_memory                                  */
/*                                                          */
/* Purpose   : Zero out the memory array                    */
/*                                                          */
/************************************************************/
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

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */ 
/*             and set up initial state of the machine.        */
/*                                                             */
/***************************************************************/
void initialize(char *ucode_filename, char *program_filename, int num_prog_files) { 
    int i;
    init_control_store(ucode_filename);

    init_memory();
    for ( i = 0; i < num_prog_files; i++ ) {
	load_program(program_filename);
	while(*program_filename++ != '\0');
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);

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
     if (argc < 3) {
	 printf("Error: usage: %s <micro_code_file> <program_file_1> <program_file_2> ...\n",
	        argv[0]);
	 exit(1);
     }
      initialize(argv[1], argv[2], argc - 2);
//     const char *micro_code_file = "ucode3";
//     const char *program_file = "isaprogram";
// initialize(micro_code_file,program_file,1);
    printf("LC-3b Simulator\n\n");
    
   

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
int flag = 0;

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

    /* 
   * Evaluate the address of the next state according to the 
   * micro sequencer logic. Latch the next microinstruction.
   */

  //The microsequencer phase uses 9 bits from the microinstruction register
  //and appropriate literals from the datapath to determine the next microinstruction.

  // IRD (1-BIT)
  // COND (2-BIT)
  // J-FIELD (6-BIT)
  // provided by current clock cycle   
    int nextState = -1;
    int jField = CURRENT_LATCHES.MICROINSTRUCTION[J0] | CURRENT_LATCHES.MICROINSTRUCTION[J1] << 1 | CURRENT_LATCHES.MICROINSTRUCTION[J2] << 2 | CURRENT_LATCHES.MICROINSTRUCTION[J3] << 3 | CURRENT_LATCHES.MICROINSTRUCTION[J4] << 4 | CURRENT_LATCHES.MICROINSTRUCTION[J5] << 5;
    int condBits = CURRENT_LATCHES.MICROINSTRUCTION[COND0] | CURRENT_LATCHES.MICROINSTRUCTION[COND1] << 1;
    int newIRD = GetIRD(CURRENT_LATCHES.MICROINSTRUCTION);
    if(newIRD){
        int opcode = (CURRENT_LATCHES.IR >> 12) & 0x000F;
        nextState = opcode; // gets opCode
    } else {
        switch(condBits){
            case 0: // pass
                nextState = Low16bits(jField); 
                break;
            case 1: // R
                if(CURRENT_LATCHES.READY){
                    nextState = Low16bits(jField | 0x0002);
                    NEXT_LATCHES.READY = 0;
                } else {
                    nextState = Low16bits(jField);
                }
                break;
            case 2: // BEN
                if(CURRENT_LATCHES.BEN){
                    nextState = Low16bits(jField | 0x0004);
                    NEXT_LATCHES.READY = 0;
                } else {
                    nextState = Low16bits(jField);
                }
                break;
            case 3: // IR[11]
                if((CURRENT_LATCHES.IR >> 11) & 0x0001){
                    nextState = Low16bits(jField | 0x0001);
                    NEXT_LATCHES.READY = 0;
                } else {
                    nextState = Low16bits(jField);
                }
                break;
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
                        if(!address){ // QUESTION, DON"T UNDERSTAND
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
  /* 
   * This function emulates memory and the WE logic. 
   * Keep track of which cycle of MEMEN we are dealing with.  
   * If fourth, we need to latch Ready bit at the end of 
   * cycle to prepare microsequencer for the fifth cycle.  
   */

}
void eval_bus_drivers() {
    if(CURRENT_LATCHES.MICROINSTRUCTION[GATE_MARMUX]){ 
        if(CURRENT_LATCHES.MICROINSTRUCTION[MARMUX]){ // MarMux1
            newBus = getAdder();
        } else {                                       // MarMux 0
            newBus = Low16bits((CURRENT_LATCHES.IR & 0x00FF) << 1);
        }
    } else if(CURRENT_LATCHES.MICROINSTRUCTION[GATE_PC]){
        newBus = CURRENT_LATCHES.PC;
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
        if(CURRENT_LATCHES.MICROINSTRUCTION[DATA_SIZE]){    // word
            newBus = Low16bits(CURRENT_LATCHES.MDR);    
        } else {        
            newBus = Low16bits(signExtend(CURRENT_LATCHES.MDR & 0x00FF)); // SR[7:0]~SR[7:0]??                                // byte
        }
    }
  /* 
   * Datapath routine emulating operations before driving the bus.
   * Evaluate the input of tristate drivers 
   *         Gate_MARMUX,
   *		 Gate_PC,
   *		 Gate_ALU,
   *		 Gate_SHF,
   *		 Gate_MDR.
   */    

}


void drive_bus() {
    if(CURRENT_LATCHES.MICROINSTRUCTION[GATE_MARMUX]){
        BUS = Low16bits(newBus);
    } else if(CURRENT_LATCHES.MICROINSTRUCTION[GATE_PC]){
        BUS = Low16bits(newBus);
    } else if(CURRENT_LATCHES.MICROINSTRUCTION[GATE_SHF]){
        BUS = Low16bits(newBus);
    } else if(CURRENT_LATCHES.MICROINSTRUCTION[GATE_MDR]){
        BUS = Low16bits(newBus);
    } else if (CURRENT_LATCHES.MICROINSTRUCTION[GATE_ALU]){
        BUS = Low16bits(newBus);
    } else {
        BUS = 0;
    }

  /* 
   * Datapath routine for driving the bus from one of the 5 possible 
   * tristate drivers. 
   */       

}


void latch_datapath_values() {

  /* 
   * Datapath routine for computing all functions that need to latch
   * values in the data path at the end of this cycle.  Some values
   * require sourcing the bus; therefore, this routine has to come 
   * after drive_bus.
   */       
    int DR1; 
    if(!CURRENT_LATCHES.MICROINSTRUCTION[DRMUX]){
        DR1 = Low16bits((CURRENT_LATCHES.IR & 0x0E00) >> 9);
    } else {
        DR1 = 0x0007;
    }
    if(CURRENT_LATCHES.MICROINSTRUCTION[LD_MAR]){
        NEXT_LATCHES.MAR = Low16bits(BUS);
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
            }
    }
    if(CURRENT_LATCHES.MICROINSTRUCTION[LD_REG]){
        NEXT_LATCHES.REGS[DR1] = Low16bits(BUS) & 0xFFFF;
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
}

// 
    // int SR1;
    //     if(!CURRENT_LATCHES.MICROINSTRUCTION[SR1MUX]){
    //         SR1 = CURRENT_LATCHES.REGS[(CURRENT_LATCHES.IR & 0x01C0) >> 6];
    //     } else {
    //         SR1 = CURRENT_LATCHES.REGS[(CURRENT_LATCHES.IR & 0x0E00) >> 9];
    //     }
    // if(CURRENT_LATCHES.MICROINSTRUCTION[R_W]){ // writing
    //     CURRENT_LATCHES.MDR = CURRENT_LATCHES.REGS[SR1];
    //     if(CURRENT_LATCHES.MICROINSTRUCTION[DATA_SIZE]){ // word
    //         MEMORY[CURRENT_LATCHES.MAR][0] = Low16bits(CURRENT_LATCHES.MDR & 0x00FF);
    //         MEMORY[CURRENT_LATCHES.MAR][1] = Low16bits((CURRENT_LATCHES.MDR & 0xFF00) >> 8);
    //     } else {
    //         MEMORY[CURRENT_LATCHES.MAR][0] = Low16bits((CURRENT_LATCHES.MDR & 0x00FF)); // byte
    //     }
    // } else { // reading 
    //     if(CURRENT_LATCHES.MICROINSTRUCTION[DATA_SIZE]){ // word
    //         int wordd = (((MEMORY[CURRENT_LATCHES.MAR][0]) & 0x00FF) | (((MEMORY[CURRENT_LATCHES.MAR][1]) & 0x00FF) << 8));
    //         CURRENT_LATCHES.MDR = wordd;
    //         CURRENT_LATCHES.REGS[DR1] = CURRENT_LATCHES.MDR;
    //     } else { // byte 
    //         CURRENT_LATCHES.MDR = Low16bits(signExtend(MEMORY[CURRENT_LATCHES.MAR][0]));
    //         CURRENT_LATCHES.REGS[DR1] = CURRENT_LATCHES.MDR;
    //     }
    // }


    // questions: 
        //  when to turn off ready bit
        // logic blocks
            // load, store logic block

        // LD.REG = BUS vs LD.REG = MDR


