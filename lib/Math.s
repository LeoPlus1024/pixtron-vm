@namespace Math

@func max(int a,int b):int
    %locals 2
    %stacks 2
    load.i32 $0
    load.i32 $1
    icmp
    iflt blarger
    load.i32 $0
    ret
blarger:
    load.i32 $1
    ret
@end

; Return the first argument raised to the power of the second argument
@func @native pow(double a,double b) : double @end