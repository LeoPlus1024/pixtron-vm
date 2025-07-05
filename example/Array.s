@namespace Array

@func main() : void
    %stacks 10
    %locals 10
    newarray.i32 10
    slocal 0
    llocal 0
    li32 0
    getarray
    li32 1
    add
    llocal 0
    li32 0
    setarray
    srefdec 0
    ret
@end