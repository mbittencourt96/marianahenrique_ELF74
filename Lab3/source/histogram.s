  PUBLIC  EightBitHistogram
       
  SECTION .text : CODE (2)
  THUMB
    
EightBitHistogram:
            MUL         R0,R0,R1      //Multiplies width (R0) by height (R1) and stores in R0
            PUSH        {R4,R5,R6}
            MOV         R4, #65536
            CMP         R0,R4
            BGT         fim
            MOV         R5, #255
            MOV         R6, #0
populate:   STR         R6, [R3], #4
            SUB         R5,R5,#1
            CMP         R5, #0
            BEQ         fim2
            B           populate
    
fim:        POP         {R4,R5,R6} 
            MOV         R0,#0      //Returns zero 
            BX          LR
fim2:       POP         {R4,R5,R6}
            BX          LR


    END