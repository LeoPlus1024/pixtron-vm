@namespace Throw
@file "Throw.pvm"
@constant "Test throw exception"

@func a() : void
    call b
    ret
@end


@func b() : void
      %stacks 1
      ldc 0
      throw
      ret
@end

@func main() : void
    %locals 1
    %stacks 1
    call a
@end