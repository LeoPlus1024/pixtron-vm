@namespace Fib

@import {   currentTimeNano , printNanoDiff , println            } from System

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
    %locals 2
    call currentTimeNano
    slocal 0
    li32 5
    call fib
    slocal 1
    call currentTimeNano
    llocal 1
    call println
    llocal 0
    sub
    call println
    ret
@end