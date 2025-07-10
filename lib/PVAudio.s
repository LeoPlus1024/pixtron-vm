@namespace PVAudio

@file "pvaudio.c"

@library("PVAudio")

@func @native PVA_game_loop(handle h) : void @end

@func @native(destroy="PVA_destroy",raw_str="on") PVA_init(string title,int w,int h,int flags) : handle @end

@func @native PVA_destroy(handle pva) : void @end


