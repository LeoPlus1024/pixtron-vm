@namespace TString

@import {   VM_Println  } from System


@constant "Hello,World."


@func printHelloWorld():void
    %locals 0
    %stack 1
    ldc.str 0
    call VM_Println
    ret
@end

