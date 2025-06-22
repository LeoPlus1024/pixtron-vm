@namespace TMath
@import {   currentTimeNano , printNanoDiff        } from System
@import {   max                                    } from Math

@func testMax(int a,int b) : int
    %locals 4
    %stack 3
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