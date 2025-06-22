@namespace TMath
@import {   VM_CurrentTimeNano , VM_PrintNanoDiff  } from System
@import {   Max                                    } from Math
@constant "startTest"

@func testMax(int a,int b) : int
    %locals 4
    %stack 3
    call VM_CurrentTimeNano
    store.i64 $2
    load.i32 $0
    load.i32 $1
    call Max
    store.i32 $3
    ; base
    load.i64 $2
    ; now
    call VM_CurrentTimeNano
    call VM_PrintNanoDiff
    load.i32 $3
    ret
@end