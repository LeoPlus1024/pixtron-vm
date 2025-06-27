@namespace TLogic

@constant "Logic left shift fail."

@func _shlTest() : void
    %locals 0
    %stacks 2
    ; exec logic left shift
    load.i32 1
    load.i32 2
    ishl
    load.i32 4
    icmp
    ifne ishl_fail
    ret
ishl_fail:
    assert 0
@end

@func main() : void
    %locals 0
    %stacks 0
    call _shlTest
    ret
@end