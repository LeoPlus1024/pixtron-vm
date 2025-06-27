@namespace TBaseline


@func fib(int n) : int
    %stacks 2
    %locals 1
    load.i32 $0
    load.i32 1
    icmp
    ifeq ret1
    load.i32 $0
    load.i32 0
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
    load.i32 0
    ret
ret1:
    load.i32 1
    ret
@end


@func main() : void
    %stacks 1
    %locals 0
    load.i32 1
    call fib
    ret
@end