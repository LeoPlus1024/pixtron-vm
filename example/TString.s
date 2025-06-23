@namespace TString

@import {   println  } from System


@constant "Hello,World."


@func printHelloWorld():void
    %locals 0
    %stacks 1
    ldc.str 0
    call println
    ret
@end

