@namespace PVAudio

@file "pvaudio.c"
@import {      println     }    from    System
@library("PVAudio")

@func PVA_event(int event) : void
    %locals 1
    %stacks 1
    llocal 0
    call println
    ret
@end

@func @native PVA_game_loop(handle h) : void @end

@func @native(destroy="PVA_destroy",raw_str="on") PVA_init(string title,int w,int h,int flags) : handle @end

@func @native PVA_destroy(handle pva) : void @end


