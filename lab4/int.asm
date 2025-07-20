.ORIG x1200
ADD R6, R6, #-2
STW R0, R6, #0
ADD R6, R6, #-2
STW R1, R6, #0
ADD R6, R6, #-2
STW R2, R6, #0

LEA R0, location 
LDW R1, R0, #0 ; r1: x4000
LDW R2, R1, #0 ; r3: M[x4000]
ADD R2, R2, #1
STW R2, R1, #0

LDW R2, R6, #0
ADD R6, R6, #2
LDW R1, R6, #0
ADD R6, R6, #2
LDW R0, R6, #0
ADD R6, R6, #2

RTI
location .fill x4000
.END