@namespace PVAudio

@file "pvaudio.c"
@import {      println     }    from    System
@library("PVAudio")

@func PVA_event(handle event) : void
    %locals 1
    %stacks 1
    llocal 0
    call PVA_event_tpye
    ;call println
    srefdec 0
    ret
@end


; Get event type enum
@func @native PVA_event_tpye(handle h) : long @end

; Startup game loop
@func @native PVA_game_loop(handle h) : void @end

; Create PVA instance
@func @native(destroy="PVA_destroy",raw_str="on") PVA_init(string title,int w,int h,int flags) : handle @end

; Destroy PVA instance
@func @native PVA_destroy(handle pva) : void @end


