@namespace Number
@import {       println     }   from System

@func main() : void
    %stacks 10
    %locals 10
    li8 #FF
    li32 1
    add
    slocal 0
    llocal 0
    li32 10
    ishl
    call println
    ret
@end