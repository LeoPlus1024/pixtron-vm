@namespace Logic
@file "Logic.pvm"

@import {       println     }   from System


@func iandTest(int a,int b,int expect) : void
        %locals 3
        %stacks 4
        llocal 0
        llocal 1
        iand
        llocal 2
        icmp
        ifeq pass
        ldc "AND result exception."
        throw
pass:
        ret
@end


@func landTest(long a,long b,long expect) : void
        %locals 3
        %stacks 4
        llocal 0
        llocal 1
        land
        llocal 2
        lcmp
        ifeq pass
        ldc "AND result exception."
        throw
pass:
        ret
@end


@func main() : void
    %locals 5
    %stacks 5
    li32 1
    li32 0
    li32 0
    call iandTest

    li32 #7FFFFFFF
    li32 #70000000
    li32 #70000000
    call iandTest

    ; 全1相与
    li32 #FFFFFFFF
    li32 #FFFFFFFF
    li32 #FFFFFFFF
    call iandTest

    ; 任意数与0相与
    li32 #12345678
    li32 0
    li32 0
    call iandTest

    ; 符号位相与（最高位保留）
    li32 #80000000
    li32 #80000000
    li32 #80000000
    call iandTest


    ; 混合符号位相与（结果清零）
    li32 #80000000
    li32 #7FFFFFFF
    li32 0
    call iandTest

    ; 高位部分重叠相与
    li32 #C0000000
    li32 #A0000000
    li32 #80000000
    call iandTest

    ; 低位精确位掩码
    li32 #0000FFFF
    li32 #00FF00FF
    li32 #000000FF
    call iandTest

    ; 全0基础用例
    li32 0
    li32 0
    li32 0
    call iandTest


    ; 最低有效位相与
    li32 1
    li32 3
    li32 1
    call iandTest

    ; 无重叠位相与
    li32 #55555555  ; 0101...
    li32 #AAAAAAAA  ; 1010...
    li32 0
    call iandTest

    ; 复杂掩码冲突
    li32 #FF00FF00
    li32 #F0F0F0F0
    li32 #F000F000
    call iandTest


    li64 #FFFFFFFFFFFFFFFF
    li64 #7FFFFFFFFFFFFFFF
    li64 #7FFFFFFFFFFFFFFF
    call landTest


    ; 测试案例 1 - 全1掩码保留特定位
    li64 #FFFFFFFFFFFFFFFF   ; 操作数A
    li64 #7FFFFFFF7FFFFFFF   ; 操作数B（保留中间32位）
    li64 #7FFFFFFF7FFFFFFF   ; 预期结果
    call landTest

    ; 测试案例 2 - 高低位交替掩码
    li64 #AAAAAAAABBBBBBBB   ; 1010... + 1011...
    li64 #55555555DDDDDDDD   ; 0101... + 1101...
    li64 #0000000099999999   ; 结果：A&B低位计算
    call landTest             ; (B的D=1101 & B的B=1011 => 1001=9)

    ; 测试案例 3 - 符号位传播测试
    li64 #8000000000000000   ; 最小负数
    li64 #FFFFFFFFFFFFFFFF   ; 全1掩码
    li64 #8000000000000000   ; 符号位保留
    call landTest

    ; 测试案例 4 - 字节粒度掩码
    li64 #FF00FF00FF00FF00   ; 交替字节掩码
    li64 #F0F0F0F0F0F0F0F0   ; 半字节掩码
    li64 #F000F000F000F000   ; 预期结果
    call landTest

    ; 测试案例 5 - 无符号最大值的位截断
    li64 #00000000FFFFFFFF   ; 低32位全1
    li64 #FFFF0000FFFF0000   ; 高16位掩码
    li64 #00000000FFFF0000   ; 结果：低32位的高16位
    call landTest

    ; 测试案例 6 - 浮点数位模式测试
    li64 #3FF0000000000000   ; double 1.0
    li64 #7FFFFFFFFFFFFFFF   ; 去除符号位掩码
    li64 #3FF0000000000000   ; 保持非符号位
    call landTest

    ; 测试案例 7 - 时间戳掩码（保留41位时间戳）
    li64 #000003FFFFFFFFFF   ; 低42位掩码
    li64 #7CA89BDE4F2A6B1D   ; 示例时间戳
    li64 #000003DE4F2A6B1D   ; 预期结果
    call landTest

    ; 测试案例 8 - IPv6地址段操作
    li64 #FFFF00000000FFFF   ; 首尾16位掩码
    li64 #20010DB812345678   ; IPv6片段
    li64 #2001000000005678   ; 保留首尾段
    call landTest

    ; 测试案例 9 - 内存页对齐检测
    li64 #FFFFFFFFFFFFF000   ; 4KB页对齐掩码
    li64 #7FEDCBA987654321   ; 任意地址
    li64 #7FEDCBA987654000   ; 页对齐结果
    call landTest


    ret
@end