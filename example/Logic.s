@namespace Logic
@file "Logic.pvm"

@import {       println     }   from System

@constant "AND result exception."

@func and(int a,int b,int except) : int
        %locals 3
        %stacks 4
        llocal 0
        llocal 1
        iand
        llocal 2
        icmp
        ifeq and_pass
        ldc 0
        throw
and_pass:
        ret
@end


@func main() : void
    %locals 5
    %stacks 5
    li32 1
    li32 0
    li32 0
    call and
    call println
    li32 #7FFFFFFF
    li32 #70000000
    li32 #70000000
    call and
    call println
    ret
@end