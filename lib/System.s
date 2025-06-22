@namespace System


@func @native("PixotronVM.so") currentTimeNano() : long @end

@func  @native("PixotronVM.so") printNanoDiff(long base,long now) : void @end

@func @native("PixotronVM.so") println(string text):void @end