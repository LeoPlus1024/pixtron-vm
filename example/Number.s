@namespace Number
@import {       println     }   from System

@func main() : void
    %stacks 10
    %locals 10
    li8 #FE
    li8 1
    add
    slocal 0
    llocal 0
    b2i
    li32 #100
    add
    li32 #FFFFFFFF
    add
    call println
    ret
@end