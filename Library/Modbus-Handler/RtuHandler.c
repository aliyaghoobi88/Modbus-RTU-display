#include "RtuHandler.h"
#include "mb.h"
#include "mbport.h"
#include "port.h"
#include "rtudisplay.h"

uint8_t HR_Write_Flag = 0;




uint8_t ModbusRtuInit(uint8_t uid,uint32_t Baudrate,TIM_HandleTypeDef* timer,UART_HandleTypeDef* uart)
{
    MT_PORT_SetTimerModule(timer);
    MT_PORT_SetUartModule(uart);
    eMBErrorCode eStatus;
    eStatus = eMBInit(MB_RTU, uid, 0, Baudrate, MB_PAR_NONE);
    eStatus = eMBEnable();
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
      HR_Write_Flag = 1;
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

 