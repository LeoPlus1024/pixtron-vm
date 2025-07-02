@namespace TArray

@func main() : void
    %locals 10
    %stacks 10
    load.i32 10
    newarray.i32
    store.arr $0
    load.i32 0
    store.i32 $1
loop:
    load.i32 $1
    load.i32 10
    icmp
    ifge end
    load.arr $0
    load.i32 $1
    getarray
    store.i32 $2
    iinc $1,1
    goto loop
end:
    ret
@end