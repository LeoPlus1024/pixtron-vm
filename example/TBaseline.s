@namespace TBaseline

@import {   currentTimeNano , printNanoDiff             } from System

; 斐波那契数朴素递归实现
@func fib(int n) : int
    %stacks 3
    %locals 1
    li32 0
    llocal
    li32 1
    icmp
    ifle ret0
    li32 0
    llocal
    li32 1
    sub
    call fib
    li32 0
    llocal
    li32 2
    sub
    call fib
    add
    ret
ret0:
    li32 0
    llocal
    ret
@end


@func main() : void
    %stacks 3
    %locals 1
    call currentTimeNano
    li32 0
    slocal
    li32 40
    call fib
    li32 0
    llocal
    call currentTimeNano
    call printNanoDiff
    ret
@end