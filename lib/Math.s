@namespace Math

@func Max(int a,int b):int
    %locals 2
    %stack 2
    load.i $0
    load.i $1
    icmp
    iflt blarger
    load.i $0
    ret
blarger:
    load.i $1
    ret
@end