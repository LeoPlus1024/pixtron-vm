@namespace Bool
@import {       println     } from System

@constant "iftrue exec....."
@constant "iffalse exec......"

@func main() : void
    %locals 1
    %stacks 2
    ltrue
    call println
    lfalse
    call println
    ltrue
    iftrue tlabel
flabel:
    ldc 1
    call println
    ret
tlabel:
    ldc 0
    call println
    lfalse
    iffalse flabel
    ret
@end