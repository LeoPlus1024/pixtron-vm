@namespace TLogic

@constant "Logic left shift fail."
@constant "Unsign logic left shift fail"

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

@func _ushlTest(int a,int b,int except) : void
    %locals 3
    %stacks 2
    load.i32 $0
    load.i32 $1
    iushr
    load.i32 $2
    icmp
    ifne fail
    ret
fail:
    assert 2
    ret
@end

@func main() : void
    %locals 0
    %stacks 3
    call _shlTest
    load.i32 #FFFFFFFF
    load.i32 1
    load.i32 #7FFFFFFF
    call _ushlTest
    ret
@end