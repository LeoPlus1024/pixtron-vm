@namespace TBaseline

@import {   currentTimeNano , printNanoDiff             } from System


@func fib(int n) : int
    %stacks 3
    %locals 1
    load.i32 $0
    load.i32 1
    icmp
    ifle ret0
    load.i32 $0
    load.i32 1
    sub
    call fib
    load.i32 $0
    load.i32 2
    sub
    call fib
    add
    ret
ret0:
    load.i32 $0
    ret
@end


@func main() : void
    %stacks 3
    %locals 1
    call currentTimeNano
    store.i64 $0
    load.i32 40
    call fib
    load.i64 $0
    call currentTimeNano
    call printNanoDiff
    ret
@end