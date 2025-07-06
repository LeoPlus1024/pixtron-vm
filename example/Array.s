@namespace Array

@import {      println     }    from System


@func main() : void
    %stacks 10
    %locals 10
    li32 10
    newarray.i32
    slocal 0
    llocal 0
    li32 0
    getarray
    li32 1
    add
    llocal 0
    li32 0
    setarray
    llocal 0
    li32 0
    getarray
    call println
    srefdec 0
    ret
@end