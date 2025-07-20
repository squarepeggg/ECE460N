#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include<ctype.h>
#include<limits.h>	
FILE* infile = NULL;
FILE* outfile = NULL;
#define MAX_LINE_LENGTH 255
#define MAX_LABEL_LEN 20
#define MAX_SYMBOLS 255


typedef struct{
    char *opcode;
    int machine_code;
} operandCode;
operandCode operandTable[] = {
    {"br", 0x0},
    {"brp", 0x0},
    {"brz", 0x0},
    {"brzp", 0x0},
    {"brn", 0x0},
    {"brnp", 0x0},
    {"brnz", 0x0},
    {"brnzp", 0x0},
    {"nop", 0x0},
    {"add", 0x1},
    {"ldb", 0x2},
    {"stb", 0x3},
    {"jsr", 0x4},
    {"jsrr", 0x4},
    {"and", 0x5},
    {"ldw", 0x6},
    {"std", 0x7},
    {"stw", 0x7},
    {"rti", 0x8},
    {"xor", 0x9},
    {"not", 0x9},
    {"jmp", 0xC},
    {"ret", 0xC},
    {"lshf", 0xD},
    {"rshfl", 0xD},
    {"rshfa", 0xD},
    {"lea", 0xE},
    {"trap", 0xF}
};

typedef struct{
    char* regName;
    int regValue;
} registerCode;
registerCode registerTable[] = {
    {"r0", 0x0000},
    {"r1", 0x0001},
    {"r2", 0x0002},
    {"r3", 0x0003},
    {"r4", 0x0004},
    {"r5", 0x0005},
    {"r6", 0x0006},
    {"r7", 0x0007}
};

typedef struct {
	int address;
	char label[MAX_LABEL_LEN + 1];	/* Question for the reader: Why do we need to add 1? */
} TableEntry;
TableEntry symbolTable[MAX_SYMBOLS];
int currentLabel = 0;
int PC = 0x0000;
int pass = 0;
    
    enum
	{
	   DONE, OK, EMPTY_LINE
	};
    enum opCode{
        AND = 1, ADD , BR, BRP, BRZ, BRZP, BRN, BRNP, BRNZ, BRNZP , HALT , JMP , JSR,  JSRR , LDB , LDW , LEA , NOP , NOT , RET , LSHF , RSHFL ,RSHFA, RTI , STB , STW , XOR
    };
    enum pesudoCode {
        ORIG, END, FILL
    };


    // checks if opcode
    int isOpcode(char* argv){
        char *new_ptr = argv;
        if(!strcmp(new_ptr, "and")){
            return AND;
        } else if(!strcmp(new_ptr, "add")){
            return ADD;
        } else if(!strcmp(new_ptr, "br")){
            return BRZ;
        } else if(!strcmp(new_ptr, "brp")){
            return BRP;
        } else if(!strcmp(new_ptr, "brz")){
            return BRZ;
        } else if(!strcmp(new_ptr, "brzp")){
            return BRZP;
        } else if(!strcmp(new_ptr, "brn")){
            return BRN;
        } else if(!strcmp(new_ptr, "brnp")){
            return BRNP;
        } else if(!strcmp(new_ptr, "brnz")){
            return BRNZ;
        } else if(!strcmp(new_ptr, "brnzp")){
            return BRNZP;
        } else if(!strcmp(new_ptr, "halt")){
            return HALT;
        } else if(!strcmp(new_ptr, "jmp")){
            return JMP;
        }else if(!strcmp(new_ptr,"jsr")){
            return JSR;
        } else if(!strcmp(new_ptr, "jsrr")){
            return JSRR;
        } else if(!strcmp(new_ptr, "ldb")){
            return LDB;
        } else if(!strcmp(new_ptr, "ldw")){
            return LDW;
        } else if(!strcmp(new_ptr, "lea")){
            return LEA;
        } else if(!strcmp(new_ptr, "nop")){
            return NOP;
        } else if(!strcmp(new_ptr, "not")){
            return NOT;
        } else if(!strcmp(new_ptr, "ret")){
            return RET;
        } else if(!strcmp(new_ptr, "lshf")){
            return LSHF;
        } else if(!strcmp(new_ptr, "rshfl")){
            return RSHFL;
        } else if(!strcmp(new_ptr,"rshfa")){
            return RSHFA;
        } else if(!strcmp(new_ptr, "rti")){
            return RTI;
        } else if(!strcmp(new_ptr, "stb")){
            return STB;
        } else if(!strcmp(new_ptr, "stw")){
            return STW;
        } else if(!strcmp(new_ptr, "xor")){
            return XOR;
        }
        return -1;
    }


int
	readAndParse( FILE * pInfile, char * pLine, char ** pLabel, char
	** pOpcode, char ** pArg1, char ** pArg2, char ** pArg3, char ** pArg4
	)
	{
	   char * lRet, * lPtr;
	   int i;
	   if( !fgets( pLine, MAX_LINE_LENGTH, pInfile ) )
		return( DONE );
	   for( i = 0; i < strlen( pLine ); i++ )
		pLine[i] = tolower( pLine[i] );
	   
           /* convert entire line to lowercase */
	   *pLabel = *pOpcode = *pArg1 = *pArg2 = *pArg3 = *pArg4 = pLine + strlen(pLine);
    
	   /* ignore the comments */
	   lPtr = pLine;

	   while( *lPtr != ';' && *lPtr != '\0' &&
	   *lPtr != '\n' ) 
		lPtr++;

	   *lPtr = '\0';
	   if( !(lPtr = strtok( pLine, "\t\n ," ) ) ) 
		return( EMPTY_LINE );

	   if( isOpcode( lPtr ) == -1 && lPtr[0] != '.' ) /* found a label */
	   {
		*pLabel = lPtr;
		if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );
	   }
	   
           *pOpcode = lPtr;

	   if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );
	   
           *pArg1 = lPtr;
	   
           if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

	   *pArg2 = lPtr;
	   if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

	   *pArg3 = lPtr;

	   if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

	   *pArg4 = lPtr;

	   return( OK );
	}

	/* Note: MAX_LINE_LENGTH, OK, EMPTY_LINE, and DONE are defined values */



// takes instruction string -> hex or decimal
int ToNum(char* pStr){
   char * t_ptr;
   char * orig_pStr;
   int t_length,k;
   int lNum, lNeg = 0;
   long int lNumLong;

   orig_pStr = pStr;
   if( *pStr == '#' )				/* decimal */
   { 
     pStr++;
     if( *pStr == '-' )				/* dec is negative */
     {
       lNeg = 1;
       pStr++;
     }
     t_ptr = pStr;
     t_length = strlen(t_ptr);
     for(k=0;k < t_length;k++)
     {
       if (!isdigit(*t_ptr))
       {
	 printf("Error: invalid decimal operand, %s\n",orig_pStr);
	 exit(4);
       }
       t_ptr++;
     }
     lNum = atoi(pStr);
     if (lNeg)
       lNum = -lNum;
 
     return lNum;
   }
   else if( *pStr == 'x' )	/* hex     */
   {
     pStr++;
     if( *pStr == '-' )				/* hex is negative */
     {
       lNeg = 1;
       pStr++;
     }
     t_ptr = pStr;
     t_length = strlen(t_ptr);
     for(k=0;k < t_length;k++)
     {
       if (!isxdigit(*t_ptr))
       {
	 printf("Error: invalid hex operand, %s\n",orig_pStr);
	 exit(4);
       }
       t_ptr++;
     }
     lNumLong = strtol(pStr, NULL, 16);    /* convert hex string into integer */
     lNum = (lNumLong > INT_MAX)? INT_MAX : lNumLong;
     if( lNeg )
       lNum = -lNum;
     return lNum;
   }
   else
   {
	printf( "Error: invalid operand, %s\n", orig_pStr);
	exit(4);  /* This has been changed from error code 3 to error code 4, see clarification 12 */
   }
}



// checks valid label, then stores label string + address 
void labelToSymbol(char* lineLabel, int PC){
        if(*lineLabel == 'x'){
            return;
        } else if(!strcmp(lineLabel,"in")){
            return;
        } else if(!strcmp(lineLabel,"out")){
            return;
        } else if(!strcmp(lineLabel,"getc")){
            return;
        } else if(!strcmp(lineLabel,"out")){
            return;
        } else if(!strcmp(lineLabel,"puts")){
            return;
        }
        symbolTable[currentLabel].address = PC;
        symbolTable[currentLabel].label[0] = *strcpy(symbolTable[currentLabel].label,lineLabel); 
        currentLabel++;
    }





void func(FILE* inputFile, FILE* outputFile) 
	{
	   char lLine[MAX_LINE_LENGTH + 1], *lLabel, *lOpcode, *lArg1,
	        *lArg2, *lArg3, *lArg4;       // declare strings for Line, labels, opcodes, arg1, arg2, arg3, arg4

	   int lRet;    // return

	//    FILE * lInfile; // file

	//    lInfile = fopen( "data.in", "r" );	/* open the input file */ 
	   do
	   {
		lRet = readAndParse( inputFile, lLine, &lLabel, // return gets value from parse 
			&lOpcode, &lArg1, &lArg2, &lArg3, &lArg4 );
		if( lRet != DONE && lRet != EMPTY_LINE ) // if return value is 0 then you are done
		{
        // IDEA IS TO FIND THE ORIGIN, then set the PC to the origin
        // increment PC constantly

        if(pass == 0){
            if(!strcmp(lOpcode,".orig")){
                PC = ToNum(lArg1);
            }
            if(*lLabel != '\0'){
                labelToSymbol(lLabel, PC);
            }
            
        } else {
            // char* line = "0x0000";
            // FILE * pOutfile;
	        // pOutfile = fopen( "data.out", "w" );
            if(!strcmp(lOpcode,".orig")){
                PC = ToNum(lArg1);
                fprintf(outputFile, "0x%X\n",PC);
            } else if(!strcmp(lOpcode, ".fill")){
                int label = ToNum(lArg1);
                fprintf(outputFile,"0x%04X\n",label);
            } else if(!strcmp(lOpcode,"halt")){
                fprintf(outputFile,"0xF025\n");
            } else if(!strcmp(lOpcode, ".end")){
            }
            for(int i = 0; i < 28; i++){
                if(!strcmp(lOpcode,operandTable[i].opcode)){
                    fprintf(outputFile, "0x%X", operandTable[i].machine_code);
                }
            }  
            int binaryArgOne = 0;
            int binaryArgTwo = 0;
            int binaryArgThree = 0;
            int immFlag = 0x0000;
            // int labelValue;
            //     for (int i = 0; i < currentLabel; i++) {
            //         if (!strcmp(lLabel, symbolTable[i].label)) {
            //            labelValue = symbolTable[i].address;
            //         }
            //     }
            //     labelValue = PC - labelValue;


            // question about lLabel vs arg1 being a label,
                // reconsider implementation of functions
                    // check
            // question about fill and LEA and BR 
                // check
            // question about clion and how to do input files
            // question about part 2 of lab




            // READS ASM CODE THEN PASSES HEX VALUES
            if(*lArg1 == '\0' && *lArg2 == '\0' && *lArg3 == '\0'){
                if(!strcmp(lOpcode, "RTI")){
                    fprintf(outputFile,"000\n");
                } else if(!strcmp(lOpcode,"nop")){
                    fprintf(outputFile,"000\n");
                } else if(!strcmp(lOpcode, "ret")){
                    // argument 1
                    fprintf(outputFile,"000\n");
                }
            } else if(*lArg1 != '\0' && *lArg2 == '\0' && *lArg3 == '\0'){
                // br, jmp, ret, jsr, jsrr, rti, trap
                if(!strcmp(lOpcode, "br")){
                    int labelValue;
                for (int i = 0; i < currentLabel; i++) {
                    if (!strcmp(lArg1, symbolTable[i].label)) {
                       labelValue = symbolTable[i].address;
                       break;
                    }
                }
                labelValue = labelValue - PC;
                    // argument 1
                binaryArgOne = labelValue - 0x0002;
                binaryArgOne /= 2;
                } else if(!strcmp(lOpcode, "brp")){
                    int labelValue;
                for (int i = 0; i < currentLabel; i++) {
                    if (!strcmp(lArg1, symbolTable[i].label)) {
                       labelValue = symbolTable[i].address;
                       break;
                    }
                }
                labelValue = labelValue - PC;
                    // argument 1
                binaryArgOne = labelValue - 0x0002;
                binaryArgOne /= 2;
                } else if(!strcmp(lOpcode, "brz")){
                    int labelValue;
                for (int i = 0; i < currentLabel; i++) {
                    if (!strcmp(lArg1, symbolTable[i].label)) {
                       labelValue = symbolTable[i].address;
                       break;
                    }
                }
                labelValue = labelValue - PC;
                    // argument 1
                binaryArgOne = labelValue - 0x0002;
                binaryArgOne /= 2;
                } else if(!strcmp(lOpcode, "brzp")){
                    int labelValue;
                for (int i = 0; i < currentLabel; i++) {
                    if (!strcmp(lArg1, symbolTable[i].label)) {
                       labelValue = symbolTable[i].address;
                       break;
                    }
                }
                labelValue = labelValue - PC;
                    // argument 1
                binaryArgOne = labelValue - 0x0002;
                binaryArgOne /= 2;
                } else if(!strcmp(lOpcode, "brn")){
                    int labelValue;
                for (int i = 0; i < currentLabel; i++) {
                    if (!strcmp(lArg1, symbolTable[i].label)) {
                       labelValue = symbolTable[i].address;
                       break;
                    }
                }
                labelValue = labelValue - PC;
                    // argument 1
                binaryArgOne = labelValue - 0x0002;
                binaryArgOne /= 2;
                } else if(!strcmp(lOpcode, "brnp")){
                    int labelValue;
                for (int i = 0; i < currentLabel; i++) {
                    if (!strcmp(lArg1, symbolTable[i].label)) {
                       labelValue = symbolTable[i].address;
                       break;
                    }
                }
                labelValue = labelValue - PC;
                    // argument 1
                binaryArgOne = labelValue - 0x0002;
                binaryArgOne /= 2;
                } else if(!strcmp(lOpcode, "brnz")){
                    int labelValue;
                for (int i = 0; i < currentLabel; i++) {
                    if (!strcmp(lArg1, symbolTable[i].label)) {
                       labelValue = symbolTable[i].address;
                       break;
                    }
                }
                labelValue = labelValue - PC;
                    // argument 1
                binaryArgOne = labelValue - 0x0002;
                binaryArgOne /= 2;
                } else if(!strcmp(lOpcode, "brnzp")){
                    int labelValue;
                for (int i = 0; i < currentLabel; i++) {
                    if (!strcmp(lArg1, symbolTable[i].label)) {
                       labelValue = symbolTable[i].address;
                       break;
                    }
                }
                labelValue = labelValue - PC;
                    // argument 1
                binaryArgOne = labelValue - 0x0002;
                binaryArgOne /= 2;
                }       
                else if(!strcmp(lOpcode, "jmp")){
                    // argument 1
                    for(int i = 0; i < 8; i++){
                        if(!strcmp(lArg1,registerTable[i].regName)){
                            binaryArgOne = registerTable[i].regValue;
                            break;
                        }
                    }
                } else if(!strcmp(lOpcode, "jsr")){
                    // argument 1
                    int labelValue;
                    for (int i = 0; i < currentLabel; i++) {
                        if (!strcmp(lArg1, symbolTable[i].label)) {
                            labelValue = symbolTable[i].address;
                            break;
                        }
                    }
                    labelValue = labelValue - PC;
                    binaryArgOne = labelValue - 0x0002;
                    binaryArgOne /= 2;
                } else if(!strcmp(lOpcode, "jsrr")){
                    // argument 1
                    for(int i = 0; i < 8; i++){
                        if(!strcmp(lArg1,registerTable[i].regName)){
                            binaryArgOne = registerTable[i].regValue;
                            break;
                        }
                    }
                }else if(!strcmp(lOpcode, "trap")){
                    // argument 1
                    int val = ToNum(lArg1);
                    int mask = 0x0027;
                    mask = mask & val;
                    binaryArgOne = mask;
                }
                
            } else if(*lArg1 != '\0' && *lArg2 != '\0' && *lArg3 == '\0'){
                // lea, not
                if(!strcmp(lOpcode,"lea")){
                    // argument 1
                    for(int i = 0; i < 8; i++){
                        if(!strcmp(lArg1,registerTable[i].regName)){
                            binaryArgOne = registerTable[i].regValue;
                            break;
                        }
                    }
                    // argument 2
                    int labelValue;
                    for (int i = 0; i < currentLabel; i++) {
                        if (!strcmp(lArg2, symbolTable[i].label)) {
                            labelValue = symbolTable[i].address;
                            break;
                        }
                    }
                    labelValue = labelValue - PC;
                    binaryArgTwo = labelValue - 0x0002;
                    binaryArgTwo /= 2;
                } else if(!strcmp(lOpcode,"not")){
                    // argument 1
                    for(int i = 0; i < 8; i++){
                        if(!strcmp(lArg1,registerTable[i].regName)){
                            binaryArgOne = registerTable[i].regValue;
                            break;
                        }
                    }
                    // argument 2
                    for(int i = 0; i < 8; i++){
                        if(!strcmp(lArg2,registerTable[i].regName)){
                            binaryArgTwo = registerTable[i].regValue;
                            break;
                        }
                    }
                }                
            } else if(*lArg1 != '\0' && *lArg2 != '\0' && *lArg3 != '\0'){
                if(!strcmp(lOpcode, "and")){
                    // argument 1
                    for(int i = 0; i < 8; i++){
                        if(!strcmp(lArg1,registerTable[i].regName)){
                            binaryArgOne = registerTable[i].regValue;
                            break;
                        }
                    }
                    // argument 2
                    for(int i = 0; i < 8; i++){
                        if(!strcmp(lArg2,registerTable[i].regName)){
                            binaryArgTwo = registerTable[i].regValue;
                            break;
                        }
                    }
                    // argument 3
                    if(*lArg3 == '#' || *lArg3 == 'x'){ // immediately
                        immFlag = 0x0020;
                        binaryArgThree = ToNum(lArg3);
                    } else { // register
                        for(int i = 0; i < 8; i++){
                        if(!strcmp(lArg3,registerTable[i].regName)){
                            binaryArgThree = registerTable[i].regValue;
                            break;
                        }
                        }
                    }
                } else if(!strcmp(lOpcode, "add")){
                    // argument 1
                    for(int i = 0; i < 8; i++){
                        if(!strcmp(lArg1,registerTable[i].regName)){
                            binaryArgOne = registerTable[i].regValue;
                            break;
                        }
                    }
                    // argument 2
                    for(int i = 0; i < 8; i++){
                        if(!strcmp(lArg2,registerTable[i].regName)){
                            binaryArgTwo = registerTable[i].regValue;
                            break;
                        }
                    }
                    // argument 3
                    if(*lArg3 == '#' || *lArg3 == 'x'){ // immediately
                        // immFlag = 0x0020;
                        immFlag = 0x0020;
                        binaryArgThree = ToNum(lArg3);
                    } else { // register
                        for(int i = 0; i < 8; i++){
                        if(!strcmp(lArg3,registerTable[i].regName)){
                            binaryArgThree = registerTable[i].regValue;
                            break;
                        }
                        }
                    }
                } else if(!strcmp(lOpcode, "ldb")){
                    // argument 1
                    for(int i = 0; i < 8; i++){
                        if(!strcmp(lArg1,registerTable[i].regName)){
                            binaryArgOne = registerTable[i].regValue;
                            break;
                        }
                    }
                    // argument 2
                    for(int i = 0; i < 8; i++){
                        if(!strcmp(lArg2,registerTable[i].regName)){
                            binaryArgTwo = registerTable[i].regValue;
                            break;
                        }
                    }
                    // argument 3
                    binaryArgThree = ToNum(lArg3);
                } else if(!strcmp(lOpcode, "ldw")){
                    // argument 1
                    for(int i = 0; i < 8; i++){
                        if(!strcmp(lArg1,registerTable[i].regName)){
                            binaryArgOne = registerTable[i].regValue;
                            break;
                        }
                    }
                    // argument 2
                    for(int i = 0; i < 8; i++){
                        if(!strcmp(lArg2,registerTable[i].regName)){
                            binaryArgTwo = registerTable[i].regValue;
                            break;
                        }
                    }
                    // argument 3
                    binaryArgThree = ToNum(lArg3);
                } else if(!strcmp(lOpcode, "lshf")){
                    // argument 1
                    for(int i = 0; i < 8; i++){
                        if(!strcmp(lArg1,registerTable[i].regName)){
                            binaryArgOne = registerTable[i].regValue;
                            break;
                        }
                    }
                    // argument 2
                    for(int i = 0; i < 8; i++){
                        if(!strcmp(lArg2,registerTable[i].regName)){
                            binaryArgTwo = registerTable[i].regValue;
                            break;
                        }
                    }
                    // argument 3
                    binaryArgThree = ToNum(lArg3);
                } else if(!strcmp(lOpcode, "rshfl")){
                    // argument 1
                    for(int i = 0; i < 8; i++){
                        if(!strcmp(lArg1,registerTable[i].regName)){
                            binaryArgOne = registerTable[i].regValue;
                            break;
                        }
                    }
                    // argument 2
                    for(int i = 0; i < 8; i++){
                        if(!strcmp(lArg2,registerTable[i].regName)){
                            binaryArgTwo = registerTable[i].regValue;
                            break;
                        }
                    }
                    // argument 3
                    binaryArgThree = ToNum(lArg3);
                } else if(!strcmp(lOpcode, "rshfa")){
                    // argument 1
                    for(int i = 0; i < 8; i++){
                        if(!strcmp(lArg1,registerTable[i].regName)){
                            binaryArgOne = registerTable[i].regValue;
                            break;
                        }
                    }
                    // argument 2
                    for(int i = 0; i < 8; i++){
                        if(!strcmp(lArg2,registerTable[i].regName)){
                            binaryArgTwo = registerTable[i].regValue;
                            break;
                        }
                    }
                    // argument 3
                    binaryArgThree = ToNum(lArg3);
                } else if(!strcmp(lOpcode, "stb")){
                    // argument 1
                    for(int i = 0; i < 8; i++){
                        if(!strcmp(lArg1,registerTable[i].regName)){
                            binaryArgOne = registerTable[i].regValue;
                            break;
                        }
                    }
                    // argument 2
                    for(int i = 0; i < 8; i++){
                        if(!strcmp(lArg2,registerTable[i].regName)){
                            binaryArgTwo = registerTable[i].regValue;
                            break;
                        }
                    }
                    // argument 3
                    binaryArgThree = ToNum(lArg3);
                } else if(!strcmp(lOpcode, "stw")){
                    // argument 1
                    for(int i = 0; i < 8; i++){
                        if(!strcmp(lArg1,registerTable[i].regName)){
                            binaryArgOne = registerTable[i].regValue;
                            break;
                        }
                    }
                    // argument 2
                    for(int i = 0; i < 8; i++){
                        if(!strcmp(lArg2,registerTable[i].regName)){
                            binaryArgTwo = registerTable[i].regValue;
                            break;
                        }
                    }
                    // argument 3
                    binaryArgThree = ToNum(lArg3);
                } else if(!strcmp(lOpcode, "xor")){
                    // argument 1
                    for(int i = 0; i < 8; i++){
                        if(!strcmp(lArg1,registerTable[i].regName)){
                            binaryArgOne = registerTable[i].regValue;
                            break;
                        }
                    }
                    // argument 2
                    for(int i = 0; i < 8; i++){
                        if(!strcmp(lArg2,registerTable[i].regName)){
                            binaryArgTwo = registerTable[i].regValue;
                            break;
                        }
                    }
                    // argument 3
                    if(*lArg3 == '#' || *lArg3 == 'x'){ // immediately
                        immFlag = 0x0020;
                        binaryArgThree = ToNum(lArg3);
                    } else { // register
                        for(int i = 0; i < 8; i++){
                        if(!strcmp(lArg3,registerTable[i].regName)){
                            binaryArgThree = registerTable[i].regValue;
                            break;
                        }
                        }
                    }
                }        
            }
            
            

            // MANIPULATES HEX VALUES
            if(!strcmp(lOpcode,"add")){
                    // 1st hex print
                binaryArgOne = binaryArgOne << 1; // make 4 bits
                int mask = 0x0004;
                mask = mask & binaryArgTwo; // get msb of arg2
                mask = mask >> 2;   // shift to lsb spot
                binaryArgOne = binaryArgOne + mask; // add msb of arg2 to arg1 0101 = 1101 
                fprintf(outputFile, "%X", binaryArgOne); // print that hex
                    // 2nd hex print        
                mask = 0x0003;  // get rid of msb in arg2        
                binaryArgTwo = binaryArgTwo & mask; // 0111 -> 0011            
                binaryArgTwo = binaryArgTwo << 2;  // shift over 2 spots so we can have 4 bits                 
                mask = 0x0030; 
                mask = mask & binaryArgThree; // arg3: - 000111 -> 000110
                mask = mask >> 4; // 000110 -> 000011
                if(immFlag == 0x0020){
                    immFlag = immFlag >> 4;
                    binaryArgTwo = binaryArgTwo + mask + immFlag; // 1100 + 000011 = 1111!
                } else {
                    binaryArgTwo = binaryArgTwo + mask; // 1100 + 000011 = 1111!
                }
                fprintf(outputFile,"%X", binaryArgTwo);
                immFlag = 0x0000;
                // ADD: Register or immediate
                // mask = 0x0020; 
                // mask = mask & binaryArgThree; // we check 6th bit
                // ADD is a register
                // if(mask == 0){
                //     mask = 0x0007;
                //     mask = mask & binaryArgThree;
                //     fprintf(outputFile,"%X\n", mask);
                // } else { // ADD is an immediate 
                mask = 0x000F;
                mask = mask & binaryArgThree;
                fprintf(outputFile,"%X\n", mask);
                // }
            } else if(!strcmp(lOpcode,"and")){
                    // 1st hex print
                binaryArgOne = binaryArgOne << 1; // make 4 bits
                int mask = 0x0004;
                mask = mask & binaryArgTwo; // get msb of arg2
                mask = mask >> 2;   // shift to lsb spot
                binaryArgOne = binaryArgOne + mask; // add msb of arg2 to arg1 0101 = 1101 
                fprintf(outputFile, "%X", binaryArgOne); // print that hex
                    // 2nd hex print
                mask = 0x0003;  // get rid of msb in arg2        
                binaryArgTwo = binaryArgTwo & mask; // 0111 -> 0011            
                binaryArgTwo = binaryArgTwo << 2;  // shift over 2 spots so we can have 4 bits                 
                mask = 0x0030; 
                mask = mask & binaryArgThree; // arg3: - 000111 -> 000110
                mask = mask >> 4; // 000110 -> 000011
                if(immFlag == 0x0020){
                    immFlag = immFlag >> 4;
                    binaryArgTwo = binaryArgTwo + mask + immFlag; // 1100 + 000011 = 1111!
                } else {
                    binaryArgTwo = binaryArgTwo + mask; // 1100 + 000011 = 1111!
                }
                binaryArgTwo = binaryArgTwo + mask; // 1100 + 000011 = 1111!
                fprintf(outputFile,"%X", binaryArgTwo);
                immFlag = 0x0000;
                // AND: Register or immediate
                // mask = 0x0020; 
                // mask = mask & binaryArgThree; // we check 6th bit
                // AND is a register
                // if(mask == 0){
                    // mask = 0x0007;
                    // mask = mask & binaryArgThree;
                    // fprintf(outputFile,"%X\n", mask);
                // } else { // AND is an immediate 
                mask = 0x000F;
                mask = mask & binaryArgThree;
                fprintf(outputFile,"%X\n", mask);
                // }          
            } else if(!strcmp(lOpcode,"jmp")){
               int mask = 0x0004;
               mask = mask & binaryArgOne;
               mask = mask >> 2;
               fprintf(outputFile, "%X", mask);
               mask = 0x0003;
               mask = mask & binaryArgOne;
               mask = mask << 2;
               fprintf(outputFile,"%X0\n", mask);
            } else if(!strcmp(lOpcode,"br")){
                int mask1 = 0x0100; // mask msb byte
                mask1 = mask1 & binaryArgOne; // find its value
                int ccBit = 0x0E00;
                mask1 = mask1 + ccBit;
                mask1 = mask1 >> 8;
                fprintf(outputFile,"%X",mask1);
                int mask2 = 0x00FF; // last byte
                mask2 = binaryArgOne & mask2;
                fprintf(outputFile,"%02X\n", mask2);
            } else if(!strcmp(lOpcode,"brp")){
                int mask1 = 0x0100; // mask msb byte
                mask1 = mask1 & binaryArgOne; // find its value
                int ccBit = 0x0200;
                mask1 = mask1 + ccBit;
                mask1 = mask1 >> 8;
                fprintf(outputFile,"%X",mask1);
                int mask2 = 0x00FF; // last byte
                mask2 = binaryArgOne & mask2;
                fprintf(outputFile,"%02X\n", mask2);
            } else if(!strcmp(lOpcode,"brz")){
                int mask1 = 0x0100; // mask msb byte
                mask1 = mask1 & binaryArgOne; // find its value
                int ccBit = 0x0400;
                mask1 = mask1 + ccBit;
                mask1 = mask1 >> 8;
                fprintf(outputFile,"%X",mask1);
                int mask2 = 0x00FF; // last byte
                mask2 = binaryArgOne & mask2;
                fprintf(outputFile,"%02X\n", mask2);
            } else if(!strcmp(lOpcode,"brzp")){
               int mask1 = 0x0100; // mask msb byte
                mask1 = mask1 & binaryArgOne; // find its value
                int ccBit = 0x0600;
                mask1 = mask1 + ccBit;
                mask1 = mask1 >> 8;
                fprintf(outputFile,"%X",mask1);
                int mask2 = 0x00FF; // last byte
                mask2 = binaryArgOne & mask2;
                fprintf(outputFile,"%02X\n", mask2);
            } else if(!strcmp(lOpcode,"brn")){
                int mask1 = 0x0100; // mask msb byte
                mask1 = mask1 & binaryArgOne; // find its value
                int ccBit = 0x0800;
                mask1 = mask1 + ccBit;
                mask1 = mask1 >> 8;
                fprintf(outputFile,"%X",mask1);
                int mask2 = 0x00FF; // last byte
                mask2 = binaryArgOne & mask2;
                fprintf(outputFile,"%02X\n", mask2);
            } else if(!strcmp(lOpcode,"brnp")){
                int mask1 = 0x0100; // mask msb byte
                mask1 = mask1 & binaryArgOne; // find its value
                int ccBit = 0x0A00;
                mask1 = mask1 + ccBit;
                mask1 = mask1 >> 8;
                fprintf(outputFile,"%X",mask1);
                int mask2 = 0x00FF; // last byte
                mask2 = binaryArgOne & mask2;
                fprintf(outputFile,"%02X\n", mask2);
            } else if(!strcmp(lOpcode,"brnz")){
                int mask1 = 0x0100; // mask msb byte
                mask1 = mask1 & binaryArgOne; // find its value
                int ccBit = 0x0C00;
                mask1 = mask1 + ccBit;
                mask1 = mask1 >> 8;
                fprintf(outputFile,"%X",mask1);
                int mask2 = 0x00FF; // last byte
                mask2 = binaryArgOne & mask2;
                fprintf(outputFile,"%02X\n", mask2);
            } else if(!strcmp(lOpcode,"brnzp")){
                int mask1 = 0x0100; // mask msb byte
                mask1 = mask1 & binaryArgOne; // find its value
                int ccBit = 0x0E00;
                mask1 = mask1 + ccBit;
                mask1 = mask1 >> 8;
                fprintf(outputFile,"%X",mask1);
                int mask2 = 0x00FF; // last byte
                mask2 = binaryArgOne & mask2;
                fprintf(outputFile,"%02X\n", mask2);
            } else if(!strcmp(lOpcode,"ldb")){
                binaryArgOne = binaryArgOne << 1; // make 4 bits
                int mask = 0x0004;
                mask = mask & binaryArgTwo; // get msb of arg2
                mask = mask >> 2;   // shift to lsb spot
                binaryArgOne = binaryArgOne + mask; // add msb of arg2 to arg1 0101 = 1101
                fprintf(outputFile,"%X",binaryArgOne);
                // 2nd hex print
                mask = 0xFFFB;  // get rid of msb in arg2
                binaryArgTwo = binaryArgTwo & mask; // 0111 -> 0011
                binaryArgTwo = binaryArgTwo << 2;  // shift over 2 spots so we can have 4 bits
                mask = 0x0030;
                mask = mask & binaryArgThree; // arg3: - 000111 -> 000110
                mask = mask >> 4; // 000110 -> 000011
                binaryArgTwo = binaryArgTwo + mask; // 1100 + 000011 = 1111!
                fprintf(outputFile,"%X",binaryArgTwo);
                // set nzp bits ?
                mask = 0x000F;
                mask = mask & binaryArgThree;
                fprintf(outputFile,"%X\n",mask);

            } else if(!strcmp(lOpcode,"ldw")){
                // 1st hex print
                binaryArgOne = binaryArgOne << 1; // make 4 bits
                int mask = 0x0004;
                mask = mask & binaryArgTwo; // get msb of arg2
                mask = mask >> 2;   // shift to lsb spot
                binaryArgOne = binaryArgOne + mask; // add msb of arg2 to arg1 0101 = 1101
                fprintf(outputFile,"%X",binaryArgOne);
                // 2nd hex print
                mask = 0xFFFB;  // get rid of msb in arg2
                binaryArgTwo = binaryArgTwo & mask; // 0111 -> 0011
                binaryArgTwo = binaryArgTwo << 2;  // shift over 2 spots so we can have 4 bits
                mask = 0x0030;
                mask = mask & binaryArgThree; // arg3: - 000111 -> 000110
                mask = mask >> 4; // 000110 -> 000011
                binaryArgTwo = binaryArgTwo + mask; // 1100 + 000011 = 1111!
                fprintf(outputFile,"%X",binaryArgTwo);
                mask = 0x000F;
                mask = mask & binaryArgThree;
                fprintf(outputFile,"%X\n",mask);
            } else if(!strcmp(lOpcode,"lea")){
                int mask = 0x00FF; // last 2 bytes
                binaryArgTwo = binaryArgTwo & mask; // pc offest
                mask = 0x0100; 
                mask = mask & binaryArgTwo; // takes msb of 0x00X0
                binaryArgOne = binaryArgOne << 1;
                binaryArgOne = binaryArgOne + mask; // does 0x0X00
                fprintf(outputFile,"%X",binaryArgOne);
                fprintf(outputFile,"%02X\n",binaryArgTwo);
            } else if(!strcmp(lOpcode,"lshf")){
                binaryArgOne = binaryArgOne << 1; // make 4 bits
                int mask = 0x0004;
                mask = mask & binaryArgTwo; // get msb of arg2
                mask = mask >> 2;   // shift to lsb spot
                binaryArgOne = binaryArgOne + mask; // add msb of arg2 to arg1 0101 = 1101 
                fprintf(outputFile, "%X", binaryArgOne); // print that hex
                    // 2nd hex print
                mask = 0xFFFB;  // get rid of msb in arg2        
                binaryArgTwo = binaryArgTwo & mask; // 0111 -> 0011            
                binaryArgTwo = binaryArgTwo << 2;  // shift over 2 spots so we can have 4 bits                 
                fprintf(outputFile,"%X", binaryArgTwo);
                mask = 0x000F;
                binaryArgThree = mask & binaryArgThree;
                fprintf(outputFile,"%X\n", binaryArgThree);
            } else if(!strcmp(lOpcode,"rshfl")){
                binaryArgOne = binaryArgOne << 1; // make 4 bits
                int mask = 0x0004;
                mask = mask & binaryArgTwo; // get msb of arg2
                mask = mask >> 2;   // shift to lsb spot
                binaryArgOne = binaryArgOne + mask; // add msb of arg2 to arg1 0101 = 1101 
                fprintf(outputFile, "%X", binaryArgOne); // print that hex
                    // 2nd hex print
                mask = 0xFFFB;  // get rid of msb in arg2        
                binaryArgTwo = binaryArgTwo & mask; // 0111 -> 0011            
                binaryArgTwo = binaryArgTwo << 2;  // shift over 2 spots so we can have 4 bits                 
                mask = 0x0001; 
                binaryArgTwo = binaryArgTwo + mask; // 1100 + 000011 = 1111!
                fprintf(outputFile,"%X", binaryArgTwo);
                mask = 0x001F;
                binaryArgThree = mask & binaryArgThree;
                fprintf(outputFile,"%X\n", binaryArgThree);
            } else if(!strcmp(lOpcode,"rshfa")){
                binaryArgOne = binaryArgOne << 1; // make 4 bits
                int mask = 0x0004;
                mask = mask & binaryArgTwo; // get msb of arg2
                mask = mask >> 2;   // shift to lsb spot
                binaryArgOne = binaryArgOne + mask; // add msb of arg2 to arg1 0101 = 1101 
                fprintf(outputFile, "%X", binaryArgOne); // print that hex
                    // 2nd hex print
                mask = 0xFFFB;  // get rid of msb in arg2        
                binaryArgTwo = binaryArgTwo & mask; // 0111 -> 0011            
                binaryArgTwo = binaryArgTwo << 2;  // shift over 2 spots so we can have 4 bits                 
                mask = 0x0003; 
                binaryArgTwo = binaryArgTwo + mask; // 1100 + 000011 = 1111!
                fprintf(outputFile,"%X", binaryArgTwo);
                mask = 0x003F;
                binaryArgThree = mask & binaryArgThree;
                fprintf(outputFile,"%X\n", binaryArgThree);
            } else if(!strcmp(lOpcode,"stb")){
                
                binaryArgOne = binaryArgOne << 1; // make 4 bits
                int mask = 0x0004;
                mask = mask & binaryArgTwo; // get msb of arg2
                mask = mask >> 2;   // shift to lsb spot
                binaryArgOne = binaryArgOne + mask; // add msb of arg2 to arg1 0101 = 1101 
                fprintf(outputFile, "%X", binaryArgOne); // print that hex
                    // 2nd hex print
                mask = 0xFFFB;  // get rid of msb in arg2        
                binaryArgTwo = binaryArgTwo & mask; // 0111 -> 0011            
                binaryArgTwo = binaryArgTwo << 2;  // shift over 2 spots so we can have 4 bits                 
                mask = 0x0030; 
                mask = mask & binaryArgThree; // arg3: - 000111 -> 000110
                mask = mask >> 4; // 000110 -> 000011
                binaryArgTwo = binaryArgTwo + mask; // 1100 + 000011 = 1111!
                fprintf(outputFile,"%X", binaryArgTwo);
                mask = 0x000F;
                binaryArgThree = mask & binaryArgThree;
                fprintf(outputFile,"%X\n", binaryArgThree);
            } else if(!strcmp(lOpcode,"stw")){
                
                binaryArgOne = binaryArgOne << 1; // make 4 bits
                int mask = 0x0004;
                mask = mask & binaryArgTwo; // get msb of arg2
                mask = mask >> 2;   // shift to lsb spot
                binaryArgOne = binaryArgOne + mask; // add msb of arg2 to arg1 0101 = 1101 
                fprintf(outputFile, "%X", binaryArgOne); // print that hex
                    // 2nd hex print
                mask = 0xFFFB;  // get rid of msb in arg2        
                binaryArgTwo = binaryArgTwo & mask; // 0111 -> 0011            
                binaryArgTwo = binaryArgTwo << 2;  // shift over 2 spots so we can have 4 bits                 
                mask = 0x0030; 
                mask = mask & binaryArgThree; // arg3: - 000111 -> 000110
                mask = mask >> 4; // 000110 -> 000011
                binaryArgTwo = binaryArgTwo + mask; // 1100 + 000011 = 1111!
                fprintf(outputFile,"%X", binaryArgTwo);
                mask = 0x000F;
                binaryArgThree = mask & binaryArgThree;
                fprintf(outputFile,"%X\n", binaryArgThree);
            } else if(!strcmp(lOpcode,"xor") || !strcmp(lOpcode,"not")){
                binaryArgOne = binaryArgOne << 1; // make 4 bits
                int mask = 0x0004;
                mask = mask & binaryArgTwo; // get msb of arg2
                mask = mask >> 2;   // shift to lsb spot
                binaryArgOne = binaryArgOne + mask; // add msb of arg2 to arg1 0101 = 1101 
                fprintf(outputFile, "%X", binaryArgOne); // print that hex
                    // 2nd hex print
                mask = 0xFFFB;  // get rid of msb in arg2        
                binaryArgTwo = binaryArgTwo & mask; // 0111 -> 0011            
                binaryArgTwo = binaryArgTwo << 2;  // shift over 2 spots so we can have 4 bits                 
                mask = 0x0030; 
                mask = mask & binaryArgThree; // arg3: - 000111 -> 000110
                mask = mask >> 4; // 000110 -> 000011
                if(immFlag == 0x0020){
                    immFlag = immFlag >> 4;
                    binaryArgTwo = binaryArgTwo + mask + immFlag; // 1100 + 000011 = 1111!
                } else {
                    binaryArgTwo = binaryArgTwo + mask; // 1100 + 000011 = 1111!
                }
                fprintf(outputFile,"%X", binaryArgTwo);
                immFlag = 0x000;
                // XOR OR NOT
                mask = 0x0020; 
                mask = mask & binaryArgThree; // we check 6th bit
                // XOR register
                if(mask == 0){
                    mask = 0x0007;
                    mask = mask & binaryArgThree;
                    fprintf(outputFile,"%X\n", mask);
                } else { // XOR OR NOT
                    mask = 0x000F;
                    mask = mask & binaryArgThree;
                    if(mask == 0x000F){
                        fprintf(outputFile,"F\n");    // XOR immediate
                    } else {
                        fprintf(outputFile,"%X\n", mask);   // NOT
                    }
                }      
            } else if(!strcmp(lOpcode,"jmp")){
               int mask = 0x0004;
               mask = mask & binaryArgOne;
               mask = mask >> 2;
               fprintf(outputFile, "%X", mask);
               mask = 0x0003;
               mask = mask & binaryArgOne;
               mask = mask << 2;
               fprintf(outputFile,"%X0\n", mask);
            } else if(!strcmp(lOpcode,"trap")){
                fprintf(outputFile,"0");
                int mask = 0x00FF;
                binaryArgOne = binaryArgOne & mask;
                fprintf(outputFile, "%X\n", binaryArgOne);
            } else if(!strcmp(lOpcode,"rti")){
                fprintf(outputFile,"000\n");
            } else if(!strcmp(lOpcode,"jsr")){
                int mask = 0x0800;
                mask = mask + binaryArgOne;
                fprintf(outputFile,"%03X\n",mask);         
            } else if(!strcmp(lOpcode,"jsrr")){
                binaryArgOne = binaryArgOne << 6;
                int mask = 0x01C0;
                mask = mask & binaryArgOne;
                fprintf(outputFile,"%03X\n",mask);
            }

        }
        PC += 2;
        
		}
	   } while( lRet != DONE ); // else continue to next line
	}








int parse(int argc, char* argv[]){
     char *prgName   = NULL;
     char *iFileName = NULL;
     char *oFileName = NULL;

     prgName   = argv[0];
     iFileName = argv[1];
     oFileName = argv[2];   

     printf("program name = '%s'\n", prgName);
     printf("input file name = '%s'\n", iFileName);
     printf("output file name = '%s'\n", oFileName);
}






    
int main(int argc, char* argv[]) {
	
     /* open the source file */
     infile = fopen("source.asm", "r");
     outfile = fopen("object.obj", "w");
    //  FILE* infile = fopen(argv[1], "r");
    //  FILE* outfile = fopen(argv[2], "w");
		 
     if (!infile) {
       printf("Error: Cannot open file %s\n", argv[1]);
       exit(4);
		 }
     if (!outfile) {
       printf("Error: Cannot open file %s\n", argv[2]);
       exit(4);
     }

    /* Do stuff with files */
    // Pass 1
    // open file
    // parse each line of the file
    // read the labels and assign the labels to the symbol table with addresses

    // 1st Pass
    func(infile,outfile); // call read and parse on file
    
    // 2nd pass setup 
    rewind(infile); // rewind file
    pass = 1;
    // FILE * pOutfile;
    // pOutfile = fopen( "data.out", "w" );

    // 2nd pass
    func(infile,outfile);
    


    // PASS 2
    // 2nd pass: translate ASM code to object code one line at a time

    

    
	
	
    // write to otuput file
	// fprintf( pOutfile, "0x%.4X\n", lInstr );	/* where lInstr is declared as an int */

     fclose(infile);
     fclose(outfile);

     // multiply function

}