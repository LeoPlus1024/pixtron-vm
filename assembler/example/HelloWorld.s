@namespace HelloWorld
; count
@field int count 10
@func @native("libc") VM_CurrentTimeNano() : long @end
; main
@func main(int i,byte b) : float
  call VM_CurrentTimeNano
  %locals 10
  %stack 10
  load.i #10
  goto loop
  ; load first param
  load.i $0
  ; load global variable
  gload.i $10
  ; add
  add
loop:
  ; store first variable slot
  store.i $0
  load.i #10
  i2f
  load.f 1.5
  add
  ret
@end