@namespace TString

@import {   println  } from System


@constant "Hello,World."

@func main() : void
    call _println
    ret
@end


@func _println() : void
    %locals 0
    %stacks 1
    ldc.str 0
    call println
    ret
@end

