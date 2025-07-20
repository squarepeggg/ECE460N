/*
    Remove all unnecessary lines (including this one)
    in this comment.
    REFER TO THE SUBMISSION INSTRUCTION FOR DETAILS

    Name 1: Gbolahan Dasilva Aggreh
    Name 2: Matthew Olan
    UTEID 1: gvd265
    UTEID 2: mto472
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
#define TRUE 1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A
*/

#define WORDS_IN_MEM 0x08000
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT; /* run bit */

typedef struct System_Latches_Struct
{

    int PC,               /* program counter */
        N,                /* n condition bit */
        Z,                /* z condition bit */
        P;                /* p condition bit */
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
void help()
{
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
void cycle()
{

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
void run(int num_cycles)
{
    int i;

    if (RUN_BIT == FALSE)
    {
        printf("Can't simulate, Simulator is halted\n\n");
        return;
    }

    printf("Simulating for %d cycles...\n\n", num_cycles);
    for (i = 0; i < num_cycles; i++)
    {
        if (CURRENT_LATCHES.PC == 0x0000)
        {
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
void go()
{
    if (RUN_BIT == FALSE)
    {
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
void mdump(FILE *dumpsim_file, int start, int stop)
{
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
void rdump(FILE *dumpsim_file)
{
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
void get_command(FILE *dumpsim_file)
{
    char buffer[20];
    int start, stop, cycles;

    printf("LC-3b-SIM> ");

    scanf("%s", buffer);
    printf("\n");

    switch (buffer[0])
    {
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
        else
        {
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
void init_memory()
{
    int i;

    for (i = 0; i < WORDS_IN_MEM; i++)
    {
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
void load_program(char *program_filename)
{
    FILE *prog;
    int ii, word, program_base;

    /* Open program file. */
    prog = fopen(program_filename, "r");
    if (prog == NULL)
    {
        printf("Error: Can't open program file %s\n", program_filename);
        exit(-1);
    }

    /* Read in the program. */
    if (fscanf(prog, "%x\n", &word) != EOF)
        program_base = word >> 1;
    else
    {
        printf("Error: Program file is empty\n");
        exit(-1);
    }

    ii = 0;
    while (fscanf(prog, "%x\n", &word) != EOF)
    {
        /* Make sure it fits. */
        if (program_base + ii >= WORDS_IN_MEM)
        {
            printf("Error: Program file %s is too long to fit in memory. %x\n",
                   program_filename, ii);
            exit(-1);
        }

        /* Write the word to memory array. */
        MEMORY[program_base + ii][0] = word & 0x00FF;
        MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
        ii++;
    }

    if (CURRENT_LATCHES.PC == 0)
        CURRENT_LATCHES.PC = (program_base << 1);

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
void initialize(char *program_filename, int num_prog_files)
{
    int i;

    init_memory();
    for (i = 0; i < num_prog_files; i++)
    {
        load_program(program_filename);
        while (*program_filename++ != '\0')
            ;
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
int main(int argc, char *argv[])
{
    FILE *dumpsim_file;

     /* Error Checking */
       if (argc < 2) {
         printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
                argv[0]);
         exit(1);
       }

    printf("LC-3b Simulator\n\n");

    initialize(argv[1], argc - 1);
    //initialize("source.txt", 1);

    if ((dumpsim_file = fopen("dumpsim", "w")) == NULL)
    {
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

   Begin your code here                  */

/***************************************************************/

// Idea: is to combine 2 bytes from memory to create a 16-bit instruction!
int fetch()
{
    NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 2; // increment PC
    int fetchingMemory = 0x0000;              // fetched instruction we plan to manipulate then return
    int memAddress = CURRENT_LATCHES.PC / 2;  // address of memory
    // little endian
    // 0x00FF | 0x0012 = 0x12FF
    int byte1 = MEMORY[memAddress][0]; // 1st byte: 0x00FF
    int byte2 = MEMORY[memAddress][1]; // 2nd byte: 0x0012
    byte2 = byte2 << 8;                // shift over the 2nd byte to be the high byte: 0x00FF -> 0xFF00
    fetchingMemory = byte1 + byte2;    // add byte 1 + byte 2 = word: 0xFF00 + 0x0012 = 0xFF12
    // printf("%x",fetchingMemory);
    return fetchingMemory; // return instruction
}

int signextend(int number)
{
    int mask = 0x8000;
    int val = mask & number;
    val = val >> 15;
    if (val)
    {
        number = 0xFFFF0000 | number;
        return number;
    }
    else
    {
        return number;
    }
}

void decodeAndExecute(int instruction)
{

    // opcode checker
    int opCode = instruction >> 12 & 0xF;
    int imm;
    int nzpMask = 0x0E00;
    int mask = 0;
    int pcOffSet;
    int dstReg = 0x0E00;
    int srcReg1 = 0x01C0;
    int srcReg2 = 0x0007;
    // FOR STORES
    int SR = 0x0E00;
    int baseR = 0x01C0;

    // check for all 14 differnet opcodes
    // figureout what state ur in + nzp bits
    switch (opCode)
    {
    case 0x0000: // BRANCH / NOP
        nzpMask = nzpMask & instruction;
        int nMask = nzpMask >> 11;
        int zMask = nzpMask >> 10;
        zMask = zMask & 0x0001;
        int pMask = nzpMask >> 9;
        pMask = pMask & 0x0001;
        if ((nMask == CURRENT_LATCHES.N && nMask != 0) || (zMask == CURRENT_LATCHES.Z && zMask != 0) || (pMask == CURRENT_LATCHES.P && pMask != 0))
        {
            // take branch
            mask = 0x0100 & instruction;
            if (mask)
            {
                pcOffSet = 0xFFFFFF00 | (instruction & 0x00FF);
                pcOffSet = pcOffSet << 1;
            }
            else
            {
                pcOffSet = instruction & 0x00FF;
                pcOffSet = pcOffSet << 1;
            }
            NEXT_LATCHES.PC = Low16bits(signextend(NEXT_LATCHES.PC) + pcOffSet);
        }
        NEXT_LATCHES.N = CURRENT_LATCHES.N;
        NEXT_LATCHES.P = CURRENT_LATCHES.P;
        NEXT_LATCHES.Z = CURRENT_LATCHES.Z;
        break;

    case 0x0001: // ADD
        dstReg = 0x0E00;
        dstReg = dstReg & instruction;
        dstReg = dstReg >> 9;
        srcReg1 = 0x01C0;
        srcReg1 = srcReg1 & instruction;
        srcReg1 = srcReg1 >> 6;
        mask = 0x0020;
        mask = mask & instruction;
        mask = mask >> 5;
        // figureout if REG or IMM
        if (!mask)
        {
            // reg
            // set CC based on val
            srcReg2 = instruction & 0x7;
            NEXT_LATCHES.REGS[dstReg] = Low16bits(signextend(CURRENT_LATCHES.REGS[srcReg1]) + signextend(CURRENT_LATCHES.REGS[srcReg2]));
        }
        else
        {
            // imm
            // set CC based on val

            mask = instruction & 0x0010;
            if (mask)
            {
                imm = 0xFFFFFFF0 | (instruction & 0x000F);
            }
            else
            {
                imm = instruction & 0x001F;
            }
            // CURRENT_LATCHES.REGS[srcReg1] = 0xFFFD;
            NEXT_LATCHES.REGS[dstReg] = Low16bits(signextend(CURRENT_LATCHES.REGS[srcReg1]) + imm);
            // printf("0x%.4x\n", NEXT_LATCHES.REGS[dstReg]);
        }
        if (signextend(NEXT_LATCHES.REGS[dstReg]) > 0)
        {
            NEXT_LATCHES.P = 1;
            NEXT_LATCHES.N = 0;
            NEXT_LATCHES.Z = 0;
        }
        else if (signextend(NEXT_LATCHES.REGS[dstReg]) < 0)
        {
            NEXT_LATCHES.N = 1;
            NEXT_LATCHES.P = 0;
            NEXT_LATCHES.Z = 0;
        }
        else if (signextend(NEXT_LATCHES.REGS[dstReg]) == 0)
        {
            NEXT_LATCHES.Z = 1;
            NEXT_LATCHES.P = 0;
            NEXT_LATCHES.N = 0;
        }

        break;
    case 0x0002: // LDB
                 // Decode
        mask = 0x0020;
        mask = mask & instruction; // get MSB bit of offset to check if pos or neg
        dstReg = dstReg & instruction;
        dstReg = dstReg >> 9; // Figureout what reg dst is
        srcReg1 = srcReg1 & instruction;
        srcReg1 = srcReg1 >> 6; // figureout what reg srcReg1 is

        // Execute
        // according to appendix A
        // Need to determine if PCOFFSET needs signExtend
        // Need to determine if MEMORY CONTENTS needs signExtend

        if (mask)
        {                                                                                                      // check if pc offset is negative
            mask = 0xFFFFFFE0 | (instruction & 0x003F);                                                        // sign extend pc offset
            int byte1 = Low16bits(MEMORY[(CURRENT_LATCHES.REGS[srcReg1] + mask) >> 1][(CURRENT_LATCHES.REGS[srcReg1] + mask) & 1]); // get 1st byte
            int mask1 = 0x0080;
            mask1 = (mask1 & byte1) >> 7;
            if (mask1)
            {                           // check if we need to signExtend actual memory contents
                byte1 = 0xFF00 | byte1; // signExtend memory contents
            }
            NEXT_LATCHES.REGS[dstReg] = Low16bits(byte1); // store
        }
        else // positive
        {
            mask = instruction & 0x003F;                                                                       // get pc offest bits
            int byte1 = Low16bits(MEMORY[(CURRENT_LATCHES.REGS[srcReg1] + mask) >> 1][(CURRENT_LATCHES.REGS[srcReg1] + mask) & 1]); // get 1st byte
            int mask1 = 0x0080;
            mask1 = (mask1 & byte1) >> 7;
            if (mask1)
            {                           // check if we need to signExtend actual memory contents
                byte1 = 0xFF00 | byte1; // signExtend memory contents
            }
            NEXT_LATCHES.REGS[dstReg] = Low16bits(byte1); // store
        }

        if (signextend(NEXT_LATCHES.REGS[dstReg]) > 0)
        {
            NEXT_LATCHES.P = 1;
            NEXT_LATCHES.N = 0;
            NEXT_LATCHES.Z = 0;
        }
        else if (signextend(NEXT_LATCHES.REGS[dstReg]) < 0)
        {
            NEXT_LATCHES.N = 1;
            NEXT_LATCHES.P = 0;
            NEXT_LATCHES.Z = 0;
        }
        else if (signextend(NEXT_LATCHES.REGS[dstReg]) == 0)
        {
            NEXT_LATCHES.Z = 1;
            NEXT_LATCHES.P = 0;
            NEXT_LATCHES.N = 0;
        }

        break;
    case 0x0003: // STB
        mask = 0x0020;
        mask = instruction & mask;              // check if MSB bit of PC is neg
        SR = SR & instruction;                  // gets sr reg[11:9]
        SR = SR >> 9;
        baseR = baseR & instruction;            // gets baseR[8:6]
        baseR = baseR >> 6; 
        SR = CURRENT_LATCHES.REGS[SR] & 0x00FF; // SR[7:0]
        if (mask)
        {                                               // checks if PCoffset is neg or pos
            mask = 0xFFFFFFC0 | (instruction & 0x003F); // if neg, sign extend pc offset
        }
        else
        {
            mask = instruction & 0x003F; // else if positive
        }
        MEMORY[(CURRENT_LATCHES.REGS[baseR] + mask) >> 1][(CURRENT_LATCHES.REGS[baseR] + mask) & 1] = Low16bits(SR); // put 1 byte into hte pcoffset + baseR

        NEXT_LATCHES.N = CURRENT_LATCHES.N;
        NEXT_LATCHES.P = CURRENT_LATCHES.P;
        NEXT_LATCHES.Z = CURRENT_LATCHES.Z;

        break;
    case 0x0004: // JSR mine
        mask = 0x0800;
        mask = mask & instruction;
        int temp = NEXT_LATCHES.PC;
        if (mask)
        {                  // jsr
            mask = 0x0400; // mask to check for negatives
            mask = mask & instruction;
            if (mask)
            { // negative case
                mask = 0xFFFFFC00 | (instruction & 0x03FF);
            }
            else
            { // non-negative case
                mask = instruction & 0x03FF;
            }
            NEXT_LATCHES.PC = Low16bits(NEXT_LATCHES.PC + (mask << 1));
        }
        else
        { // jsrr
            mask = 0x01C0;
            mask = mask & instruction;
            mask = mask >> 6;
            NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.REGS[mask]);
        }
        NEXT_LATCHES.REGS[7] = temp;

        NEXT_LATCHES.N = CURRENT_LATCHES.N;
        NEXT_LATCHES.P = CURRENT_LATCHES.P;
        NEXT_LATCHES.Z = CURRENT_LATCHES.Z;
        break;
    case 0x0005: // AND mine
        dstReg = 0x0E00;
        dstReg = dstReg & instruction;
        dstReg = dstReg >> 9;
        srcReg1 = 0x01C0;
        srcReg1 = srcReg1 & instruction;
        srcReg1 = srcReg1 >> 6;
        mask = 0x0020;
        mask = mask & instruction;
        mask = mask >> 5;

        // figureout if REG or IMM
        if (!mask)
        {
            // reg
            // set CC based on val
            srcReg2 = instruction & 0x7;
            NEXT_LATCHES.REGS[dstReg] = Low16bits(signextend(CURRENT_LATCHES.REGS[srcReg1]) & signextend(CURRENT_LATCHES.REGS[srcReg2]));
        }
        else
        {
            // imm
            // set CC based on val
            mask = instruction & 0x0010;
            if (mask)
            {
                imm = 0xFFFFFFF0 | (instruction & 0x000F);
            }
            else
            {
                imm = instruction & 0x001F;
            }
            // CURRENT_LATCHES.REGS[srcReg1] = 0xFFFD;
            NEXT_LATCHES.REGS[dstReg] = Low16bits(signextend(CURRENT_LATCHES.REGS[srcReg1]) & imm);
            // printf("0x%.4x\n", NEXT_LATCHES.REGS[dstReg]);
        }
        if (signextend(NEXT_LATCHES.REGS[dstReg]) > 0)
        {
            NEXT_LATCHES.P = 1;
            NEXT_LATCHES.N = 0;
            NEXT_LATCHES.Z = 0;
        }
        else if (signextend(NEXT_LATCHES.REGS[dstReg]) < 0)
        {
            NEXT_LATCHES.N = 1;
            NEXT_LATCHES.P = 0;
            NEXT_LATCHES.Z = 0;
        }
        else if (signextend(NEXT_LATCHES.REGS[dstReg]) == 0)
        {
            NEXT_LATCHES.Z = 1;
            NEXT_LATCHES.P = 0;
            NEXT_LATCHES.N = 0;
        }
        break;
    case 0x0006: // LDW mine
        mask = 0x0020;
        mask = mask & instruction;
        dstReg = 0x0E00;
        dstReg = dstReg & instruction;
        dstReg = dstReg >> 9;
        srcReg1 = 0x01C0;
        srcReg1 = srcReg1 & instruction;
        srcReg1 = srcReg1 >> 6;

        if (mask)
        { // negative
            mask = 0xFFFFFFE0 | (instruction & 0x003F);
            int byte1 = MEMORY[Low16bits((signextend(CURRENT_LATCHES.REGS[srcReg1]) + (signextend(mask) << 1))) / 2][0]; // debug
            int byte2 = MEMORY[Low16bits((signextend(CURRENT_LATCHES.REGS[srcReg1]) + (signextend(mask) << 1))) / 2][1];
            byte1 = byte1 & 0x00FF;
            byte2 = byte2 << 8;
            NEXT_LATCHES.REGS[dstReg] = Low16bits(byte1 + byte2);
        }

        else
        { // positive
            mask = instruction & 0x003F;
            int byte1 = MEMORY[Low16bits((signextend(CURRENT_LATCHES.REGS[srcReg1]) + (signextend(mask) << 1))) / 2][0];
            int byte2 = MEMORY[Low16bits((signextend(CURRENT_LATCHES.REGS[srcReg1]) + (signextend(mask) << 1))) / 2][1];
            byte1 = byte1 & 0x00FF;
            byte2 = byte2 << 8;
            NEXT_LATCHES.REGS[dstReg] = Low16bits(byte1 + byte2);
        }

        if (signextend(NEXT_LATCHES.REGS[dstReg]) > 0)
        {
            NEXT_LATCHES.P = 1;
            NEXT_LATCHES.N = 0;
            NEXT_LATCHES.Z = 0;
        }
        else if (signextend(NEXT_LATCHES.REGS[dstReg]) < 0)
        {
            NEXT_LATCHES.N = 1;
            NEXT_LATCHES.P = 0;
            NEXT_LATCHES.Z = 0;
        }
        else if (signextend(NEXT_LATCHES.REGS[dstReg]) == 0)
        {
            NEXT_LATCHES.Z = 1;
            NEXT_LATCHES.P = 0;
            NEXT_LATCHES.N = 0;
        }
        break;
    case 0x0007: // STW
        mask = 0x0020;
        mask = instruction & mask;                   // check if MSB bit of PC is neg
        SR = SR & instruction;                       // gets sr reg[11:9]
        SR = SR >> 9;
        baseR = baseR & instruction;                 // gets baseR[8:6]
        baseR = baseR >> 6;
        int SR1 = CURRENT_LATCHES.REGS[SR] & 0x00FF; // SR[7:0]
        int SR2 = CURRENT_LATCHES.REGS[SR] & 0xFF00;
        if (mask)
        {                                               // checks if PCoffset is neg or pos
            mask = 0xFFFFFFC0 | (instruction & 0x003F); // if neg, sign extend pc offset
        }
        else
        {
            mask = 0x003F & instruction; // else if positive
        }
        MEMORY[Low16bits(signextend(CURRENT_LATCHES.REGS[baseR]) + (mask << 1)) / 2][0] = SR1; // put byte 1 into hte pcoffset + baseR
        MEMORY[Low16bits(signextend(CURRENT_LATCHES.REGS[baseR]) + (mask << 1)) / 2][1] = (SR2 >> 8); // put byte 2 into the pcoffest + baseR
        // MEMORY[Low16bits((signextend(CURRENT_LATCHES.REGS[srcReg1]) + (signextend(mask) << 1))) / 2][0];
        NEXT_LATCHES.N = CURRENT_LATCHES.N;
        NEXT_LATCHES.P = CURRENT_LATCHES.P;
        NEXT_LATCHES.Z = CURRENT_LATCHES.Z;

        break;
    case 0x0009: // XOR
        dstReg = 0x0E00;
        dstReg = dstReg & instruction;
        dstReg = dstReg >> 9;
        srcReg1 = 0x01C0;
        srcReg1 = srcReg1 & instruction;
        srcReg1 = srcReg1 >> 6;
        mask = 0x0020;
        mask = mask & instruction;
        mask = mask >> 5;

        // figureout if REG or IMM
        if (!mask)
        {
            // reg
            // set CC based on val
            srcReg2 = instruction & 0x7;
            NEXT_LATCHES.REGS[dstReg] = Low16bits(signextend(CURRENT_LATCHES.REGS[srcReg1]) ^ signextend(CURRENT_LATCHES.REGS[srcReg2]));
        }
        else
        {
            // imm
            // set CC based on val
            mask = instruction & 0x0010;
            if (mask)
            {
                imm = 0xFFFFFFF0 | (instruction & 0x000F);
            }
            else
            {
                imm = instruction & 0x001F;
            }
            // CURRENT_LATCHES.REGS[srcReg1] = 0xFFFD;
            NEXT_LATCHES.REGS[dstReg] = Low16bits(signextend(CURRENT_LATCHES.REGS[srcReg1]) ^ imm);
            // printf("0x%.4x\n", NEXT_LATCHES.REGS[dstReg]);
        }

        if (signextend(NEXT_LATCHES.REGS[dstReg]) > 0)
        {
            NEXT_LATCHES.P = 1;
            NEXT_LATCHES.N = 0;
            NEXT_LATCHES.Z = 0;
        }
        else if (signextend(NEXT_LATCHES.REGS[dstReg]) < 0)
        {
            NEXT_LATCHES.N = 1;
            NEXT_LATCHES.P = 0;
            NEXT_LATCHES.Z = 0;
        }
        else if (signextend(NEXT_LATCHES.REGS[dstReg]) == 0)
        {
            NEXT_LATCHES.Z = 1;
            NEXT_LATCHES.P = 0;
            NEXT_LATCHES.N = 0;
        }
        break;
    case 0x000C: // JMP mine 
        baseR = 0x01C0;
        baseR = baseR & instruction;
        baseR = baseR >> 6;

        NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.REGS[baseR]);

        NEXT_LATCHES.N = CURRENT_LATCHES.N;
        NEXT_LATCHES.P = CURRENT_LATCHES.P;
        NEXT_LATCHES.Z = CURRENT_LATCHES.Z;

        break;
    case 0x000D: // SHF mine
        mask = 0x0030;
        mask = mask & instruction;
        mask = mask >> 4;
        dstReg = 0x0E00;
        dstReg = dstReg & instruction;
        dstReg = dstReg >> 9;
        srcReg1 = 0x01C0;
        srcReg1 = srcReg1 & instruction;
        srcReg1 = srcReg1 >> 6;
        int amount = 0x000F & instruction;

        if (mask == 0) // LSHF
        {
            NEXT_LATCHES.REGS[dstReg] = Low16bits(Low16bits(CURRENT_LATCHES.REGS[srcReg1]) << amount);
        }
        else if (mask == 1) // RSHF
        {
            NEXT_LATCHES.REGS[dstReg] = Low16bits(Low16bits(CURRENT_LATCHES.REGS[srcReg1]) >> amount);
        }
        else if (mask == 3) // RSHFA
        {
            mask = 0x8000 & Low16bits(CURRENT_LATCHES.REGS[srcReg1]);
            mask = mask >> 15;
            int val; 

            if (mask)
            {
                val = CURRENT_LATCHES.REGS[srcReg1]; // 1111111
                for(int i = 0; i < amount; i++)
                {
                    val = 0x8000 | (Low16bits(val) >> 1);

                }
                NEXT_LATCHES.REGS[dstReg] = Low16bits(val);
            }
            else
            {
                NEXT_LATCHES.REGS[dstReg] = Low16bits(Low16bits(CURRENT_LATCHES.REGS[srcReg1]) >> amount);
            }
        }
        if (signextend(NEXT_LATCHES.REGS[dstReg]) > 0)
        {
            NEXT_LATCHES.P = 1;
            NEXT_LATCHES.N = 0;
            NEXT_LATCHES.Z = 0;
        }
        else if (signextend(NEXT_LATCHES.REGS[dstReg]) < 0)
        {
            NEXT_LATCHES.N = 1;
            NEXT_LATCHES.P = 0;
            NEXT_LATCHES.Z = 0;
        }
        else if (signextend(NEXT_LATCHES.REGS[dstReg]) == 0)
        {
            NEXT_LATCHES.Z = 1;
            NEXT_LATCHES.P = 0;
            NEXT_LATCHES.N = 0;
        }
        break;
    case 0x000E: // LEA mine
        dstReg = 0x0E00;
        dstReg = dstReg & instruction;
        dstReg = dstReg >> 9;
        mask = 0x0100 & instruction;
        if (mask)
        {
            pcOffSet = 0xFFFFFF00 | (instruction & 0x00FF);
            pcOffSet = pcOffSet << 1;
        }
        else
        {
            pcOffSet = instruction & 0x00FF;
            pcOffSet = pcOffSet << 1;
        }
        NEXT_LATCHES.REGS[dstReg] = Low16bits(NEXT_LATCHES.PC + pcOffSet);

        NEXT_LATCHES.N = CURRENT_LATCHES.N;
        NEXT_LATCHES.P = CURRENT_LATCHES.P;
        NEXT_LATCHES.Z = CURRENT_LATCHES.Z;
        // set the previous conditons codes for current_latch
        break;
    case 0x000F:       // TRAP
        mask = 0x00FF; // trapvector 8
        mask = mask & instruction;
        NEXT_LATCHES.REGS[7] = NEXT_LATCHES.PC; // incremneted pc put into r7
        int byte1 = MEMORY[(mask << 1) / 2][0]; // zero extended 16 bit trap vector put
        int byte2 = MEMORY[(mask << 1) / 2][1];
        byte1 = byte1 & 0x00FF;
        byte2 = byte2 << 8;
        NEXT_LATCHES.PC = Low16bits(byte1 + byte2);

        break;

        // if we get some invalid input from auto grader
    default:
        return;
    }
}


void process_instruction()
{
    FILE *dumpsim_file; 

    // fetch
    // figureout what PC is ??
    // increment next_latch.pc with current_latch.pc + 2
    // access memory of current_latch
    // int memory = memory[current_latch.pc][0]
    // memory += memory[current_latch.pc][1] << 8
    int instruction = fetch();

    // decode
    // check first 4 bits for opcode
    // know what instruction your executing
    // figure out curr state + next state
    decodeAndExecute(instruction);
   
    /*  function: process_instruction
     *
     *    Process one instruction at a time
     *       -Fetch one instruction
     *       -Decode
     *       -Execute
     *       -Update NEXT_LATCHES
     */
}