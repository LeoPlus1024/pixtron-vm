@namespace TMath
@import {   currentTimeNano , printNanoDiff             } from System
@import {   max             , pow                       } from Math

@func testMax(int a,int b) : int
    %locals 4
    %stacks 3
    call currentTimeNano
    store.i64 $2
    load.i32 $0
    load.i32 $1
    call max
    store.i32 $3
    ; base
    load.i64 $2
    ; now
    call currentTimeNano
    call printNanoDiff
    load.i32 $3
    ret
@end


@func testPow(double a,double b) : double
    %locals 2
    %stacks 2
    load.f64 $0
    load.f64 $1
    call pow
    ret
@end