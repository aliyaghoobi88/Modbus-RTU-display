#include "RtuHandler.h"
#include "mb.h"
#include "mbport.h"
#include "port.h"
#include "rtudisplay.h"

uint8_t SaveEE_flag = 0;
static USHORT usRegInputStart = REG_INPUT_START;
static USHORT usRegInputBuf[REG_INPUT_NREGS] = {'M', 'o', 'd', 'b', 'u', 's', 0, 0};


uint8_t ModbusRtuInit(uint8_t uid,uint32_t Baudrate)
{
    MT_PORT_SetTimerModule(&htim3);
    MT_PORT_SetUartModule(&huart2);
    eMBErrorCode eStatus;
    eStatus = eMBInit(MB_RTU, uid, 0, Baudrate, MB_PAR_NONE);
    eStatus = eMBEnable();
    return eStatus;
}






/*Read Input register handler*/
eMBErrorCode eMBRegInputCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNRegs)
{
  eMBErrorCode eStatus = MB_ENOERR;
  int iRegIndex;

  if ((usAddress >= REG_INPUT_START) &&
      (usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS))
  {
    iRegIndex = (int)(usAddress - usRegInputStart);

    while (usNRegs > 0)
    {
      *pucRegBuffer++ = (unsigned char)(usRegInputBuf[iRegIndex] >> 8);
      *pucRegBuffer++ = (unsigned char)(usRegInputBuf[iRegIndex] & 0xFF);

      iRegIndex++;
      usNRegs--;
    }
  }
  else
  {
    eStatus = MB_ENOREG;
  }

  return eStatus;
}

/*Read and Write Holding  register handler*/
eMBErrorCode eMBRegHoldingCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode)
{
  eMBErrorCode eStatus = MB_ENOERR;
  int iRegIndex;

  if ((usAddress >= HR_StartAdd) &&
      (usAddress + usNRegs <= HR_StartAdd + HR_REG_Size))
  {
    iRegIndex = (int)(usAddress - Holding_REG_Start);

    switch (eMode)
    {
    case MB_REG_WRITE:
      // Write operation
      SaveEE_flag = 1;
      while (usNRegs > 0)
      {
        // Extract two bytes from the buffer and update the holding register.
        HRBuff[iRegIndex] = (USHORT)(*pucRegBuffer++ << 8);
        HRBuff[iRegIndex] |= (USHORT)(*pucRegBuffer++);

        iRegIndex++;
        usNRegs--;
      }
      break;

    case MB_REG_READ:
      // Read operation
      while (usNRegs > 0)
      {
        // Copy the holding register data to the buffer.
        *pucRegBuffer++ = (unsigned char)(HRBuff[iRegIndex] >> 8);
        *pucRegBuffer++ = (unsigned char)(HRBuff[iRegIndex] & 0xFF);

        iRegIndex++;
        usNRegs--;
      }
      break;

    default:
      // Unsupported operation mode. Return an error.
      eStatus = MB_EINVAL;
      break;
    }
  }
  else
  {
    eStatus = MB_ENOREG;
  }
  return eStatus;
}

// Unesed Function Code Handler

eMBErrorCode eMBRegCoilsCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNCoils,
                           eMBRegisterMode eMode)
{
  return MB_ENOREG;
}
eMBErrorCode eMBRegDiscreteCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNDiscrete)
{
  return MB_ENOREG;
}
