@namespace HelloWorld
; count
@var int count 10
; main
@func main(int i,byte b)
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
  ret
@end