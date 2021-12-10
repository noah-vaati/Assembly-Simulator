        MOVE R1,0
        MOVE R2,1
        MOVE R3,2

        MOVE [R2],0
        MOVE [R3],3
        MOVE [R1],11

        MOVE [R2],24
        MOVE [R3],4
        MOVE [R1],10

 loop:  MOVE R0,[R1]
        BGT  R1,loop
        