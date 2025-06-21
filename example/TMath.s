@namespace TMath
@import {   VM_CurrentTimeNano , VM_PrintNanoDiff  } from System
@import {   Max                                    } from Math


@func testMax(int a,int b) : int
    %locals 4
    %stack 3
    call VM_CurrentTimeNano
    store.l $2
    load.i $0
    load.i $1
    call Max
    store.i $3
    ; base
    load.l $2
    ; now
    call VM_CurrentTimeNano
    call VM_PrintNanoDiff
    load.i $3
    ret
@end