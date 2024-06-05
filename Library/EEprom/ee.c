#include "ee.h"
#include <string.h>

#define PAGE 0
#define SECTOR 1
#define PAGE_NUM 2

#define _EE_SIZE 2048
#define _EE_ADDR_INUSE (((uint32_t)0x08000000) | (_EE_SIZE * _EE_USE_FLASH_PAGE_OR_SECTOR))
#define _EE_PAGE_OR_SECTOR PAGE_NUM

#if (_EE_USE_RAM_BYTE > 0)
  uint8_t ee_ram[_EE_USE_RAM_BYTE];
#endif

/**
 * @brief initail the EE for the stm32
 *
 * @return true  in success Mode
 * @return false  in fail Mode
 */
bool ee_init(void)
{
#if (_EE_USE_RAM_BYTE > 0)
  return ee_read(0, _EE_USE_RAM_BYTE, NULL);
#else
  return true;
#endif
}
/**
 * @brief Formats the EEPROM.
 *
 * This function erases the EEPROM memory, effectively formatting it.
 * Depending on the configuration, it can erase a page or sector of the EEPROM.
 * Optionally, it can also reset the RAM buffer used for caching EEPROM data.
 *
 * @param keepRamData If true, the RAM buffer will not be cleared after formatting.
 *                    If false, the RAM buffer will be filled with 0xFF after formatting.
 *
 * @return Returns true on successful formatting, false on failure.
 *         If formatting is successful, the function checks for an error code, which should be 0xFFFFFFFF.
 *         If the error code is not 0xFFFFFFFF, it means an error occurred during formatting.
 */

bool ee_format(bool keepRamData)
{
  uint32_t error;
  HAL_FLASH_Unlock();
  FLASH_EraseInitTypeDef flashErase;
#if _EE_PAGE_OR_SECTOR == PAGE
  flashErase.NbPages = 1;
  flashErase.PageAddress = _EE_ADDR_INUSE;
  flashErase.TypeErase = FLASH_TYPEERASE_PAGES;
#elif _EE_PAGE_OR_SECTOR == SECTOR
  flashErase.NbSectors = 1;
  flashErase.Sector = _EE_USE_FLASH_PAGE_OR_SECTOR;
  flashErase.TypeErase = FLASH_TYPEERASE_SECTORS;
#elif _EE_PAGE_OR_SECTOR == PAGE_NUM
  flashErase.NbPages = 1;
  flashErase.PageAddress = _EE_ADDR_INUSE;
  flashErase.TypeErase = FLASH_TYPEERASE_PAGES;
#endif
#ifdef _EE_FLASH_BANK
  flashErase.Banks = _EE_FLASH_BANK;
#endif
#ifdef _EE_VOLTAGE_RANGE
  flashErase.VoltageRange = _EE_VOLTAGE_RANGE;
#endif
  if (HAL_FLASHEx_Erase(&flashErase, &error) == HAL_OK)
  {
    HAL_FLASH_Lock();
    if (error != 0xFFFFFFFF)
      return false;
    else
    {
#if (_EE_USE_RAM_BYTE > 0)
      if (keepRamData == false)
        memset(ee_ram, 0xFF, _EE_USE_RAM_BYTE);
#endif
      return true;
    }
  }
  HAL_FLASH_Lock();
  return false;
}
/**
 * @brief Reads data from EEPROM.
 *
 * This function reads data from the EEPROM starting from the given virtual address.
 * If a valid data pointer is provided, it stores the data in the given buffer.
 * Optionally, it also updates a separate RAM buffer if configured to do so.
 *
 * @param startVirtualAddress The starting virtual address to read data from.
 * @param len The length of the data to read in bytes.
 * @param data Pointer to the buffer where the read data will be stored.
 *             If this parameter is NULL, only the RAM buffer will be updated.
 *
 * @return Returns true on success, false on failure.
 *         The function will fail if the read operation goes beyond the EEPROM size
 *         or if the data pointer is NULL.
 */
bool ee_read(uint32_t startVirtualAddress, uint32_t len, uint8_t *data)
{
  if ((startVirtualAddress + len) > _EE_SIZE)
    return false;
  for (uint32_t i = startVirtualAddress; i < len + startVirtualAddress; i++)
  {
    if (data != NULL)
    {
      *data = (*(__IO uint8_t *)(i + _EE_ADDR_INUSE));
      data++;
    }
#if (_EE_USE_RAM_BYTE > 0)
    if (i < _EE_USE_RAM_BYTE)
      ee_ram[i] = (*(__IO uint8_t *)(i + _EE_ADDR_INUSE));
#endif
  }
  return true;
}
/**
 * @brief Writes data to EEPROM.
 *
 * This function writes data to the EEPROM starting from the given virtual address.
 * It writes data in chunks of 8 bytes, assuming the EEPROM can be written in double words (64 bits).
 *
 * @param startVirtualAddress The starting virtual address to write data to.
 * @param len The length of the data to write in bytes.
 * @param data Pointer to the data to be written.
 *
 * @return Returns 0 on success, or a non-zero error code on failure.
 *         Error codes: 1 - Out of EEPROM address range
 *                      2 - Null pointer provided for data
 *                      3 - Error during EEPROM programming
 */
int ee_write(uint32_t startVirtualAddress, uint32_t len, uint8_t *data)
{
  if ((startVirtualAddress + len) > _EE_SIZE)
    return 1;
  if (data == NULL)
    return 2;
  HAL_FLASH_Unlock();

  for (uint32_t i = 0; i < len; i += 8)
  {
    uint64_t data64 = data[i];
    data64 += data[i + 1] * 0x100;
    data64 += data[i + 2] * 0x10000;
    data64 += data[i + 3] * 0x1000000;
    data64 += data[i + 4] * 0x100000000;
    data64 += data[i + 5] * 0x10000000000;
    data64 += data[i + 6] * 0x1000000000000;
    data64 += data[i + 7] * 0x100000000000000;
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, ((i + startVirtualAddress)) + _EE_ADDR_INUSE, data64) != HAL_OK)
    {
      HAL_FLASH_Lock();
      return 3;
    }
  }
  HAL_FLASH_Lock();
  return 0;
}
// ##########################################################################################################
bool ee_writeToRam(uint32_t startVirtualAddress, uint32_t len, uint8_t *data)
{
#if (_EE_USE_RAM_BYTE > 0)
  if ((startVirtualAddress + len) > _EE_USE_RAM_BYTE)
    return false;
  if (data == NULL)
    return false;
  memcpy(&ee_ram[startVirtualAddress], data, len);
  return true;
#else
  return false;
#endif
}
// ##########################################################################################################
bool ee_commit(void)
{
#if (_EE_USE_RAM_BYTE > 0)
  if (ee_format(true) == false)
    return false;
  return ee_write(0, _EE_USE_RAM_BYTE, ee_ram);
#else
  return false;
#endif
}
// ##########################################################################################################
uint32_t ee_maxVirtualAddress(void)
{
  return (_EE_SIZE);
}
// ##########################################################################################################
