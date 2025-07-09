@namespace Math

@func max(int a,int b):int
    %locals 2
    %stacks 2
    llocal 0
    llocal 1
    icmp
    iflt blarger
    llocal 0
    ret
blarger:
    llocal 1
    ret
@end

; Return the first argument raised to the power of the second argument
@func @native(destroy="xxxxxxx") pow(double a,double b) : double @end