//
// Created by outlaw on 01.06.2022.
//

#ifndef SA_PICO_CAN_BOOTLOADER_BOOTLOADER_HPP
#define SA_PICO_CAN_BOOTLOADER_BOOTLOADER_HPP

#include "tim.h"
#include "base_device.hpp"
#include "main.h"
#include "flash_driver.h"
#include "eeprom.hpp"
#include <cstring>
#include <eeprom_24aa02uid.hpp>
#include "i2c.hpp"


static inline void delay(uint32_t delayms){
    __HAL_TIM_DISABLE_IT(&htim3, TIM_IT_UPDATE);
    HAL_TIM_Base_Start(&htim6);
    TIM6->CNT = 0;
    while(TIM6->CNT < delayms){}
    HAL_TIM_Base_Stop(&htim6);
    __HAL_TIM_ENABLE_IT(&htim3, TIM_IT_UPDATE);
}

using namespace Protos;

#define BLOCK_SIZE_FLASH        (FLASH_WRITE_BLOCK_SIZE)
#define BYTES_IN_PACKET         (8)
#define PACKETS_IN_BLOCK        (BLOCK_SIZE_FLASH/BYTES_IN_PACKET)
#define RESEND_PACKETS_N_TIMES  (3)
#define EEPROM_I2C_ADDR 0x50

class BootLoader : public BaseDevice{
public:
    BootLoader(DeviceUID::TYPE uidType, uint8_t family, uint8_t addr, FDCAN_HandleTypeDef* can)
        : BaseDevice(uidType, family, addr, can)
    {
        savedFamily = family;
        savedAddress = addr;
    }

    inline void readExtEEPROM(I2C_HandleTypeDef *_i2c){
        auto I2CMaster = I2C(I2C(_i2c));
        auto eeprom = Eeprom24AAUID(&I2CMaster, EEPROM_I2C_ADDR);
        uint8_t r_data[6] = {0};
        eeprom.readUID(r_data);
        memcpy(&Uid.Data.I4, r_data, sizeof(uint32_t));
        UID = Uid.Data.I1[3] + (Uid.Data.I1[2]<<8) + (Uid.Data.I1[1]<<16);
    }

    void init(I2C_HandleTypeDef *_i2c){
        readInternalEEPROM();
        readExtEEPROM(_i2c);
        initDataStructures();
        startStayInBootTimer();
    }

    void OnTimer(int ms) override {}
    inline void OnPoll() override {
        if(jumpToProg) jumpToMainProg();
    }

    inline void jumpToMainProg(){
        CpuStartUserProgram();
    }

    inline void requestDataPackets(){
        if(nOKPackets > 0 && nOKPackets < expectedNOfPackets) {
            unValidateBlock();
            uint8_t firstMissed = 0, lastMissed = 0;
            for (int packetNum = 0; packetNum < PACKETS_IN_BLOCK; packetNum++) {
                if (correctPacketNums[packetNum] == 0xFFFF) {
                    if (!firstMissed) firstMissed = packetNum;
                    lastMissed = packetNum;
                }
            }
            if(resendNTimes++ >= RESEND_PACKETS_N_TIMES) stopResendPacketsTimer();
            else reStartResendPacketsTimer();
            sendFCMessage(BOOT_FC_RESEND_PACKETS, firstMissed, lastMissed-firstMissed+1);
        }
    }

    inline void ProcessBootMessage(const Protos::BootMsg& bootMsg) override {
        switch (bootMsg.GetMSGType()) {
            case MSGTYPE_BOOT_DATA:
                if(blockValidated)
                    putToBuffer(bootMsg);
                break;
            case MSGTYPE_BOOT_ADDR_CRC:
                if(savedAddress == bootMsg.GetCRCMsgAddr()){
                    validateBlock(bootMsg);
                } else unValidateBlock();
                break;
            case MSGTYPE_BOOT_FLOW:
                if(UID == bootMsg.GetMsgUID()){
                    if (bootMsg.GetFlowCMDCode() == BOOT_FC_FINISH_FLASH)
                        finishFlash();
                    if (bootMsg.GetFlowCMDCode() == BOOT_FC_STAY_IN_BOOT) {
                        savedAddress = bootMsg.GetFlowMsgAddr();
                        totalBlocks = bootMsg.GetTotalBlocks();
                        savedFWVer = bootMsg.GetSWVer();
                        stopStayInBootTimer();
                    }
                }
                break;
            case MSGTYPE_BOOT_BOOTREQ:
                if(UID == bootMsg.GetMsgUID())
                    sendHelloFromBoot();
                break;
            default:
                break;
        }
    }

    void ProcessMessage(const Protos::Msg& msg) override{
    }

    inline void stopStayInBootTimer(){
        __HAL_TIM_CLEAR_IT(&htim1, TIM_IT_UPDATE);
        HAL_TIM_Base_Stop_IT(&htim1);
    }

    inline void stayInBootTimHandler(){
        HAL_GPIO_TogglePin(LED_1_GPIO_Port, LED_1_Pin);
        if(stayInBootTimer == (WAIT_TIME_IN_BOOT_SEC - 1))
            sendHelloFromBoot();
        else if(stayInBootTimer <= 0) {
            stopStayInBootTimer();
            jumpToProg = true;
        }
        stayInBootTimer--;
    };

protected:
    inline void validateBlock(const Protos::BootMsg& bootMsg){
        uint16_t incomeBlockNum = bootMsg.GetAbsolutePacketNum() / BLOCK_SIZE_FLASH;
        if(currentBlockNum == incomeBlockNum){
            if(!nOKPackets){
                currentBlockCRC = bootMsg.GetCRC16();
                currentBlockDataLen = bootMsg.GetDataLen();
                expectedNOfPackets =
                        currentBlockDataLen / BYTES_IN_PACKET + (currentBlockDataLen % BYTES_IN_PACKET ? 1 : 0);
            }
            blockValidated = true;
        }else
            sendFCMessage(BOOT_FC_BLOCK_UNVALIDATED);
    }

    constexpr inline void unValidateBlock(){
        blockValidated = false;
    }

    inline void putToBuffer(const Protos::BootMsg& bootMsg){
        uint32_t absPacketNum = bootMsg.GetAbsolutePacketNum();
        uint16_t receivedBlock = absPacketNum / BLOCK_SIZE_FLASH;
        if(receivedBlock == currentBlockNum){
            reStartResendPacketsTimer();
            uint16_t bufferOffset = absPacketNum % BLOCK_SIZE_FLASH;
            uint16_t packetNumInBlock = bufferOffset / BYTES_IN_PACKET;
            if (isNotDuplicate(packetNumInBlock)) {
                for (int i = 0; i < BYTES_IN_PACKET; i++)
                    blockBuffer[i + bufferOffset] = bootMsg.Data[i];
                correctPacketNums[packetNumInBlock] = packetNumInBlock;
                if (++nOKPackets == expectedNOfPackets){
                    unValidateBlock();
                    finishBlockFlashEraseEachBlock();
                    return;
                }
            }
        }
    }

    inline bool isNotDuplicate(uint16_t packetNum){
        return correctPacketNums[packetNum] != packetNum;
    }

    inline constexpr bool cmpCRC(uint8_t packetNum, uint16_t incomeCRC){
        uint16_t currentCRC = 0;
        uint8_t bufferOffset = packetNum * BYTES_IN_PACKET;
        for(int i = bufferOffset; i < bufferOffset+BYTES_IN_PACKET; i++)
            currentCRC += blockBuffer[i];
        currentCRC = (((~currentCRC) + 1) & 0xffff);
        return incomeCRC == currentCRC;
    }

    inline void sendFCMessage(uint8_t flowCode, uint8_t resendPacketsFrom = 0x0, uint8_t resendPacketsLen = 0){
        char data[8] = {0};
        data[0] = Uid.Type;
        data[1] = savedAddress;
        data[2] = BOOT_FC_FLAG_FC;
        data[3] = flowCode;
        data[4] = resendPacketsFrom;
        data[5] = resendPacketsLen;
        data[6] = (currentBlockNum & 0x00ff);
        data[7] = (currentBlockNum & 0xff00);
        SendBootMsg (MSGTYPE_BOOT_FLOW, data, 8);
    }

    void sendHelloFromBoot(){
        char data[8] = {0};
        data[0] = Uid.Type;
        data[1] = savedAddress;
        data[2] = savedFamily;
        data[3] = savedHWVer;
        data[4] = savedFWVer;
        data[5] = 0xFF;
        data[6] = 0xFF;
        data[7] = 0xFF;
        SendBootMsg (MSGTYPE_BOOT_ACK, data, 8);
//        SendRawMsg (data, 8);
    }

    inline bool checkReceivedBlockCRC() const{
        uint16_t currentBufferBlockCRC = 0;
        for(int i=0; i < currentBlockDataLen; i++)
            currentBufferBlockCRC += blockBuffer[i];
        currentBufferBlockCRC = ((~currentBufferBlockCRC + 1) & 0xffff);
        return currentBufferBlockCRC == currentBlockCRC;
    }

    inline bool checkWrittenBlockCRC(uint32_t addr) const{
        if(addr == getUserProgBaseAddress() + FLASH_WRITE_FIRST_BLOCK_SIZE) return true;
        uint16_t inFlashBlockCRC = 0;
        auto *from = (uint8_t *)addr;
        uint16_t len = currentBlockDataLen;
        while (len-- > 0)
            inFlashBlockCRC += *from++;
        inFlashBlockCRC = (((~inFlashBlockCRC) + 1) & 0xffff);
        return currentBlockCRC == inFlashBlockCRC;
    }

    inline void finishBlockFlashEraseEachBlock(){
        stopResendPacketsTimer();
        uint32_t addrCalc = getUserProgBaseAddress() + (currentBlockNum * BLOCK_SIZE_FLASH);
        uint32_t bytesInPage = addrCalc % FLASH_ERASE_PAGE_SIZE;
        uint32_t writeLen = currentBlockDataLen + currentBlockDataLen % (sizeof(uint64_t)); //flashWrite manage doubleWord so if we need to be sure all data will be in bounds
        if(!bytesInPage) FlashErase(addrCalc);
        else if(writeLen > (FLASH_ERASE_PAGE_SIZE - bytesInPage))
            FlashErase(addrCalc + (FLASH_ERASE_PAGE_SIZE - bytesInPage));
        uint8_t *dataPtr = blockBuffer;
        if(!currentBlockNum){
            FlashCopyToVectorBlock((uint32_t)(blockBuffer));
            writeLen -= FLASH_WRITE_FIRST_BLOCK_SIZE;
            addrCalc += FLASH_WRITE_FIRST_BLOCK_SIZE;
            if(FLASH_WRITE_BLOCK_SIZE > FLASH_WRITE_FIRST_BLOCK_SIZE){
                dataPtr = &blockBuffer[FLASH_WRITE_FIRST_BLOCK_SIZE];
            }
        }
        if(checkReceivedBlockCRC()){
            if(FlashWrite(addrCalc, writeLen, dataPtr)){
                currentBlockNum++;
                sendFCMessage(BOOT_FC_BLOCK_OK);
//                if(currentBlockNum >= totalBlocks)
//                    finishFlash();
            }else
                sendFCMessage(BOOT_FC_FLASH_BLOCK_WRITE_FAIL);
        }else
            sendFCMessage(BOOT_FC_BLOCK_CRC_FAIL);
        initDataStructures();
    }

    inline void finishBlockFlashEraseOnce(){
        stopResendPacketsTimer();
        uint32_t addrCalc = getUserProgBaseAddress() + (currentBlockNum * BLOCK_SIZE_FLASH);
        uint32_t writeLen = currentBlockDataLen + currentBlockDataLen % (sizeof(uint64_t)); //flashWrite manage doubleWord so if we need to be sure all data will be in bounds
        uint8_t *dataPtr = blockBuffer;
        if(!currentBlockNum){
            FlashErase(addrCalc, (BLOCK_SIZE_FLASH * totalBlocks));
            FlashCopyToVectorBlock((uint32_t)(blockBuffer));
            writeLen -= FLASH_WRITE_FIRST_BLOCK_SIZE;
            addrCalc += FLASH_WRITE_FIRST_BLOCK_SIZE;
            if(FLASH_WRITE_BLOCK_SIZE > FLASH_WRITE_FIRST_BLOCK_SIZE){
                dataPtr = &blockBuffer[FLASH_WRITE_FIRST_BLOCK_SIZE];
            }
        }
        if(FlashWrite(addrCalc, writeLen, dataPtr)){
            currentBlockNum++;
            sendFCMessage(BOOT_FC_BLOCK_OK);
//            if(currentBlockNum >= totalBlocks)
//                finishFlash();
        }else
            sendFCMessage(BOOT_FC_FLASH_BLOCK_WRITE_FAIL);
        initDataStructures();
    }

    inline void finishFlash(){
        FlashFinishWriteChecksum();
        saveBoardDataToEEPROM();
        sendFCMessage(BOOT_FC_FLASH_READY);
        if(FlashVerifyChecksum()){
            jumpToProg = true;
            return;
        }
        //if ret from  FlashVerifyChecksum() -> error
        currentBlockNum = 0;
        nOKPackets = 0;
        currentBlockCRC = 0;
        sendFCMessage(BOOT_FC_FLASH_NOT_READY); //TODO add error ERROR_ON_JUMP
    }
private:
    uint16_t currentBlockNum = 0,
            nOKPackets = 0,
            currentBlockCRC = 0,
            currentBlockDataLen = 0;
    uint8_t blockBuffer[BLOCK_SIZE_FLASH] = {0xFF,};
    uint16_t correctPacketNums[PACKETS_IN_BLOCK] = {0xFFFF,};
    uint16_t expectedNOfPackets = 0;
    bool blockValidated = false;
    bool jumpToProg = false;

    uint32_t UID = 0;
    uint8_t savedAddress = 0,
        savedFamily = 0,
        savedHWVer = 0,
        savedFWVer = 0,
        totalBlocks = 0,
        resendNTimes = 0,
        stayInBootTimer = WAIT_TIME_IN_BOOT_SEC;

    BOARD_ERROR currentError = NO_ERROR;
    [[noreturn]] static void errorHandler(BOARD_ERROR error){
        switch (error){
            case CAN_ERROR:
                break;
            case EEPROM_ERROR:
                break;
            default:
                break;
        }
        //TODO mb remove
        Error_Handler();
        while (true){}
    }

    void initDataStructures(){
        nOKPackets = 0;
        CpuMemSet((uint32_t)correctPacketNums, 0xFF, PACKETS_IN_BLOCK*(sizeof(correctPacketNums[0])/sizeof(uint8_t)));
        CpuMemSet((uint32_t)blockBuffer, 0xFF, BLOCK_SIZE_FLASH*(sizeof(blockBuffer[0])/sizeof(uint8_t)));
    }

    inline void stopResendPacketsTimer(){
        __HAL_TIM_CLEAR_IT(&htim3, TIM_IT_UPDATE);
        TIM3->CNT = 0;
        HAL_TIM_Base_Stop_IT(&htim3);
        resendNTimes = 0;
    }

    inline void startStayInBootTimer(){
        HAL_TIM_Base_Start_IT(&htim1);
    }

    inline void reStartResendPacketsTimer() {
        __HAL_TIM_CLEAR_IT(&htim3, TIM_IT_UPDATE);
        TIM3->CNT = 0;
        HAL_TIM_Base_Start_IT(&htim3);
    }

    inline void readInternalEEPROM(){
        char buffer[EEPROM_BOARD_DATA_SIZE];
        int Offset = EEPROM_BOARD_DATA_ADDR;
        int bufferOffset = 0;
        eeprom_read_block(Offset, buffer, sizeof(buffer));
//        memcpy(&Uid.Data.I4, buffer, sizeof(Uid.Data.I4));
        bufferOffset += sizeof(Uid.Data.I4);
        savedAddress = buffer[bufferOffset];
        bufferOffset += sizeof(savedAddress);
        savedHWVer = buffer[bufferOffset];
        bufferOffset += sizeof(savedHWVer);
        savedFWVer = buffer[bufferOffset];
//        UID = Uid.Data.I1[3] + (Uid.Data.I1[2]<<8) + (Uid.Data.I1[1]<<16);
        Address = savedAddress;
    }

    inline void saveBoardDataToEEPROM(){
        char buffer[EEPROM_BOARD_DATA_SIZE];
        int Offset = EEPROM_BOARD_DATA_ADDR;
        int bufferOffset=0;
        memcpy(buffer+bufferOffset, &Uid.Data.I4, sizeof(uint32_t));
        bufferOffset += sizeof(uint32_t);
        memcpy(buffer+bufferOffset, &Address, sizeof(uint8_t));
        bufferOffset += sizeof(uint8_t);
        memcpy(buffer+bufferOffset, &savedHWVer, sizeof(uint8_t));
        bufferOffset += sizeof(uint8_t);
        memcpy(buffer+bufferOffset, &savedFWVer, sizeof(uint8_t));
        eeprom_write_block(Offset, buffer, sizeof(buffer));
    }
};

#endif //SA_PICO_CAN_BOOTLOADER_BOOTLOADER_HPP