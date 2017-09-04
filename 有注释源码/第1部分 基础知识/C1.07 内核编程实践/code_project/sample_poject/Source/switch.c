
__asm void PendSV_Handler ()
{
    IMPORT  blockPtr
    
    // 加载寄存器存储地址
    LDR     R0, =blockPtr
    LDR     R0, [R0]
    LDR     R0, [R0]

    // 保存寄存器
    STMDB   R0!, {R4-R11}
    
    // 将最后的地址写入到blockPtr中
    LDR     R1, =blockPtr
    LDR     R1, [R1]
    STR     R0, [R1]
    
    // 修改部分寄存器，用于测试
    ADD R4, R4, #1
    ADD R5, R5, #1
    
    // 恢复寄存器
    LDMIA   R0!, {R4-R11}
    
    // 异常返回
    BX      LR
}  
