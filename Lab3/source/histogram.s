  PUBLIC  EightBitHistogram
       
  SECTION .text : CODE (2)
  THUMB
    
EightBitHistogram:
            MUL         R0,R0,R1      //Multiplies width (R0) by height (R1) and stores in R0
            PUSH        {R4,R5,R6,R7,R8,R9}
            MOV         R7, R0        //Stores image size in R7
            MOV         R4, #65536
            CMP         R0,R4
            BGT         fim
            MOV         R5, #255
            MOV         R6, #0
            MOV         R8, R3         //store initial address of histogram in R8
populate:   STR         R6, [R3], #4   //populate array "histograma" with zeros
            SUB         R5,R5,#1
            CMP         R5, #0
            BEQ         calculate
            B           populate
            
calculate:  MOV         R3, R8          //return to base address of "histograma" saved previously in R8
            LDRB        R4, [R2], #1   //gets values from image
            LDR         R6, [R3,R4]       
            ADD         R6, R6, #1     //increment the counter for this pixel
            STR         R6, [R3,R4]   //store in the right position in "histograma"
            SUB         R0, R0, #1
            CMP         R0, #0
            BNE         calculate
            B           fim2
fim:        POP         {R4,R5,R6,R7,R8,R9} 
            MOV         R0,#0      //Returns zero 
            BX          LR
fim2:       MOV         R0, R7    //Returns image size
            POP         {R4,R5,R6,R7,R8,R9}
            BX          LR


    END