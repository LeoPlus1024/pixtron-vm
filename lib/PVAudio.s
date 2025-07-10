@namespace PVAudio

@file "pvaudio.c"

@library("PVAudio")

@func @native PVA_window_show(handle h) : bool @end

@func @native(destroy="PVA_destroy",raw_str="on") PVA_init(string title,int w,int h,int flags) : handle @end

@func @native PVA_destroy(handle pva) : void @end


