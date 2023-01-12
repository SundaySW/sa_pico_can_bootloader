
#include "flash_driver.h"

void CpuInit()
{
  CpuIrqDisable();
  CpuWatchogConf();
}

bool interrupts_enabled;
void CpuIrqEnable(){
//    if (interrupts_enabled)
        __enable_irq();
}
void CpuIrqDisable(){
//     interrupts_enabled = (__get_PRIMASK() == 0);
     __disable_irq();
}

void CpuWatchogConf(){
    //TODO look about watchdog
}
void CpuWatchDogUpdate(){
    //TODO UpdateWatchDog
}

inline void softReset(){
    SysTick->CTRL = 0;
    for(int i = 0; i < 8; i++){
        NVIC->ICER[i] = 0xFFFFFFFF;
//        NVIC->ICPR[i] = 0xFFFFFFFF;
    }
    SCB->VTOR = CPU_USER_PROGRAM_VECTABLE_OFFSET & (unsigned int)0x1FFFFF80;
    HAL_RCC_DeInit();
    HAL_DeInit();
    auto* ptr = (uint32_t*)(CPU_USER_PROGRAM_VECTABLE_OFFSET & (unsigned int)0x1FFFFF80);
    __set_MSP (*(++ptr));
    __set_BASEPRI (0);
    __set_CONTROL (0);
}

void CpuStartUserProgram()
{
    __disable_irq();
    void (*start)();
    if (!FlashVerifyChecksum())
      return;
    softReset();
    start = (void(*)())(*((uint32_t *) CPU_USER_PROGRAM_STARTADDR_PTR));
    __enable_irq();
    start();
}

void CpuMemSet(uint32_t dest, uint8_t value, uint16_t len)
{
    auto *to = (uint8_t *)dest;
    while (len-- > 0)
    {
        *to++ = value;
        CpuWatchDogUpdate();
    }
}

void CpuMemCopy(uint32_t dest, uint32_t src, uint16_t len)
{
    auto *from = (uint8_t *)src;
    auto *to = (uint8_t *)dest;
    while (len-- > 0)
    {
        *to++ = *from++;
        CpuWatchDogUpdate();
    }
}

