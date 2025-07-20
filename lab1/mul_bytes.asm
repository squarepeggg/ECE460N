.ORIG x3000
LEA R0, X 
LEA R1, Y
LDB R2, R0, #0 ; gets X byte
LDB R3, R1, #0 ; gets Y byte

AND R4, R4, #0 ; result
AND R5, R5, #0 ; 0 or 1 checker
AND R6, R6, #0 ; 8 bits = 9 counter
ADD R6, R6, #8 
AND R7, R7, #0 ; bit counter


; need to check if we hit a 1 or a 0
multiLoop ; loop
ADD R6, R6, #-1
BRz storeResult
AND R5, R3, #1  ; get LSB
BRz skip
ADD R4, R4, R1 ; store 1 bit
skip
LSHF R2, R2, #1 ; go to next bit
RSHFA R3, R3, #1 ; 
ADD R6, R6, #-1
br multiLoop

storeResult
LEA R6, result
STB R4, R6, #0

AND R6, R6, #0 ; 8 bits = 9 counter
ADD R6, R6, #8 

ofLOOP
ADD R6, R6, #-1
BRz noOF
AND R5, R4, #1
BRz BAD
RSHFA R4, R4, #1
BRp ofLOOP

BAD 
LEA R5, overflow
AND R4, R4, #0
ADD R4, R4, #1
STB R4, R5, #0
BR EXIT


noOF
LEA R5, overflow
AND R4, R4, #0
STB R4, R5, #0

EXIT

X .fill x3100 
Y .fill x3101 
result .FILL x3102
overflow  .FILL x3103
.end