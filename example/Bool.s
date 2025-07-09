@namespace Bool
@import {       println     } from System

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
    ldc "iffalse exec......"
    call println
    ret
tlabel:
    ldc "iftrue exec....."
    call println
    lfalse
    iffalse flabel
    ret
@end