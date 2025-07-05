@namespace Fib

@import {   currentTimeNano , printNanoDiff             } from System

; 斐波那契数朴素递归实现
@func fib(int n) : int
    %stacks 3
    %locals 1
    llocal 0
    li32 1
    icmp
    ifle ret0
    llocal 0
    li32 1
    sub
    call fib
    llocal 0
    li32 2
    sub
    call fib
    add
    ret
ret0:
    llocal 0
    ret
@end


@func main() : void
    %stacks 3
    %locals 1
    call currentTimeNano
    slocal 0
    li32 40
    call fib
    llocal 0
    call currentTimeNano
    call printNanoDiff
    ret
@end