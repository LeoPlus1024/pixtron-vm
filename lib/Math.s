@namespace Math

@func max(int a,int b):int
    %locals 2
    %stacks 2
    li32 0
    llocal
    li32 1
    llocal
    icmp
    iflt blarger
    li32 0
    llocal
    ret
blarger:
    li32 1
    llocal
    ret
@end

; Return the first argument raised to the power of the second argument
@func @native pow(double a,double b) : double @end