@namespace Klass
@import         {       println        }     from System

@field string text

@func <cinit>() :void
    %stacks 1
    ldc "Init constructor"
    sfield 0
    lfield 0
    call println
    ret
@end


@func main() : void
    ret
@end