//
// Created by outlaw on 01.06.2022.
//
#ifndef SA_PICO_CAN_BOOTLOADER_PROTOS_BOOT_MSG_H
#define SA_PICO_CAN_BOOTLOADER_PROTOS_BOOT_MSG_H

#include "protos_msg.h"

namespace Protos
{
    class BootMsg
    {
    public:
        BootMsg() {
            Dlc = 8;
            ExtId.Bytes = 0;
            ExtId.bit.BootLoader = 1;
            ExtId.bit.Protocol = 1;
        }

        BootMsg(const Protos::Msg& msg)
        {
            ExtId.bit.BootLoader = 1;
            ExtId.bit.Protocol = 1;
            Dlc = msg.Dlc;
            ExtId.Bytes = msg.Id.Ext;
            for (int i = 0; i < Dlc; i++)
                Data[i] = msg.Data[i];
        }

        union
        {
            struct
            {
                unsigned long ServiceByte2  : 8;  ///< 24-bit Offset or UID
                unsigned long ServiceByte1  : 8;
                unsigned long ServiceByte0  : 8;
                unsigned long MsgType	    : 3;  ///< type of boot msg
                unsigned long BootLoader    : 1;  ///< BootLoader Id
                unsigned long Protocol	    : 1;  ///< Protos or RAW-protocol
            } bit;
            unsigned long Bytes;
            unsigned char Tab[4];
        } ExtId;

        inline unsigned char GetMSGType() const
        {
            return (ExtId.bit.MsgType);
        }

        inline uint16_t GetFlowCMDCode() const
        {
            return Data[3];
        }

        inline uint16_t GetCRC16() const {
            return (Data[0] + (Data[1] << 8));
        }

        inline uint16_t GetDataLen() const {
            return (Data[2] + (Data[3] << 8));
        }

        inline uint32_t GetAbsolutePacketNum() const {
            return (ExtId.Tab[0] + (ExtId.Tab[1] << 8) + (ExtId.Tab[2] << 16));
        }

        inline uint32_t GetMsgUID() const {
            return (ExtId.Tab[0] + (ExtId.Tab[1] << 8) + (ExtId.Tab[2] << 16));
        }

        inline uint16_t GetTotalBlocks() const {
            return (Data[4] + (Data[5] << 8));
        }
        inline unsigned char GetCRCMsgAddr() const
        {
            return Data[7];
        }
        inline unsigned char GetFlowMsgAddr() const
        {
            return Data[1];
        }

        enum { DATA_SIZE = 8 };
        unsigned char Dlc;
        unsigned char Data[DATA_SIZE];
    };
}

#endif //SA_PICO_CAN_BOOTLOADER_PROTOS_BOOT_MSG_H
