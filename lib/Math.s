@namespace Math

@func Max(int a,int b):int
    %locals 2
    %stack 2
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