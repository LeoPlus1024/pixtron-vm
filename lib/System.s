@namespace System


@func @native("libc.6") currentTimeNano() : long @end

@func  @native("libc.6") printNanoDiff(long base,long now) : void @end

@func @native("libc.6") println(string text):void @end