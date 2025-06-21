@namespace Math

@func Max(int a,int b):int
    %locals 2
    %stack 2
    load.i $0
    load.i $1
    icmp
    ifeq equal
    load.i $0
    load.i $1
    icmp
    ifgt label0
    load.i $0
    load.i $1
    icmp
    iflt label1
label0:
    load.i $0
    ret
label1:
    load.i $1
    ret
equal:
   load.i $0
   ret
@end