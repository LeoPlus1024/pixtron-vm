@namespace System


@func @native("libc.6") VM_CurrentTimeNano() : long @end

@func  @native("libc.6") VM_PrintNanoDiff(long base,long now) : void @end

@func @native("libc.6") VM_Println(string text):void @end