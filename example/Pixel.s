@namespace Pixel

@import {    PVA_init , PVA_window_show } from PVAudio

@field handle pva
@func main() : void
    %locals 1
    %stacks 4
    ldc "Demo"
    li32 640
    li32 480
    li32 0
    call PVA_init
    sfield 0
    lfield 0
    call PVA_window_show
    ret
@end