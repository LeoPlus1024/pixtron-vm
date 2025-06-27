@namespace TMath
@import {   currentTimeNano , printNanoDiff             } from System
@import {   max             , pow                       } from Math

@constant "Max exec fail."
@constant "Integer div fail."
@constant "Double div fail."

; Compare the maximum value between a and b.
@func _testMax(int a,int b,int expect) : void
    %locals 4
    %stacks 3
    load.i32 $0
    load.i32 $1
    call max
    load.i32 $2
    icmp
    ifne max_fail
    ret
 max_fail:
    assert 0
    ret
@end


@func _testPow(double a,double b) : void
    %locals 3
    %stacks 3
    load.f64 $0
    load.f64 $1
    call pow
    ret
@end

@func _testIntDiv(int a,int b,int except) : void
    %locals 2
    %stacks 2
    load.i32 $0
    load.i32 $1
    div
    load.i32 $2
    icmp
    ifne fail
    ret
fail:
    assert 1
    ret
@end

@func _testDoubleDiv(double a,double b,double except) : void
    %locals 2
    %stacks 2
    load.f64 $0
    load.f64 $1
    div
    load.f64 $2
    dcmp
    ifne div_fail
    ret
div_fail:
    assert 2
    ret
@end


@func main() : void
    %locals 0
    %stacks 3
    load.i32 10
    load.i32 11
    load.i32 11
    call _testMax
    load.f64 2.0
    load.f64 2.0
    call _testPow
    load.i32 10
    load.i32 2
    load.i32 5
    call _testIntDiv
    load.f64 100.0
    load.f64 5
    load.f64 20
    call _testDoubleDiv
    ret
@end