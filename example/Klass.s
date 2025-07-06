@namespace Klass
@import         {       println        }     from System

@constant "Init constructor"

@field string text

@func <cinit>() :void
    %stacks 1
    ldc 0
    sfield 0
    lfield 0
    call println
    ret
@end


@func main() : void
    ret
@end