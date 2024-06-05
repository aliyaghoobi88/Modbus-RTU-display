#include "Myutil.h"
#include <stdarg.h>
#include <stdio.h> 
#include <string.h>
char buffer[BUFFER_SIZE];
FloatBytes F2Byte;



/**
 * @brief Prints the binary representation of a uint32_t value.
 *
 * @param value The uint32_t value to print.
 * @param bitLength The number of bits to consider.
 */
void print_binary(uint32_t value, unsigned int bitLength)
{
  for (unsigned int i = bitLength - 1; i < bitLength; i--)
  {
    LogPrint("%d", (value >> i) & 1);
  }
}

/**
 * @brief Prints formatted string to UART.
 *
 * @param format The format string.
 * @param ... Additional arguments for the format.
 */
void LogPrint(const char *format, ...)
{
  memset(buffer, 0, sizeof(buffer));

  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  uint32_t len = strlen(buffer);
  HAL_UART_Transmit(&LOG_UART_HANDLE, (uint8_t *)buffer, len, HAL_MAX_DELAY);
}


float Mb_GetFloat(uint8_t Base_address, uint16_t *array)
{
    /* Ensure that the array indices are within bounds */
    if (Base_address < 1 || Base_address >= UINT8_MAX || array == NULL)
    {
        /* Handle the error appropriately, e.g., return an error code or take corrective action */
        return 0.0f;
    }

    /* Use explicit type casting to avoid implicit conversions */
    F2Byte.Byte[1] = (uint16_t)array[Base_address];
    F2Byte.Byte[0] = (uint16_t)array[Base_address + 1];

    return F2Byte.Float;
}


void Mb_SetFloat(uint8_t Base_address, float FValue, uint16_t *array)
{
    /* Ensure that the array indices are within bounds */
    if (Base_address < 1 || Base_address >= UINT8_MAX || array == NULL)
    {
        /* Handle the error appropriately, e.g., return an error code or take corrective action */
        return;
    }

    F2Byte.Float = FValue;
    /* Use explicit type casting to avoid implicit conversions */
    array[Base_address] = (uint16_t)F2Byte.Byte[1];
    array[Base_address + 1] = (uint16_t)F2Byte.Byte[0];
}


float LinInterp(float raw_value, float slope, float intercept)
{
    float result;

    /* Ensure that there are no mixed-mode operations */
    result = raw_value * slope;
    result += intercept;

    return result;
}


uint16_t DACmapSignal(float input, float inputMin, float inputMax, uint16_t outputMin, uint16_t outputMax)
{
    // Perform the mapping using linear interpolation
    float scaledValue = ((input - inputMin) / (inputMax - inputMin)) * (outputMax - outputMin) + outputMin;

    // Ensure the output is within the desired range
    if (scaledValue < outputMin)
    {
        scaledValue = outputMin;
    }
    else if (scaledValue > outputMax)
    {
        scaledValue = outputMax;
    }
    return (uint16_t)scaledValue;
}


// float MapSignalFloat(float input, float inputMin, float inputMax, float outputMin, float outputMax)
// {
//     float mappedValue = 0.0f;  // Initialize to a default value

//     // Check for invalid input range
//     if (inputMin >= inputMax)
//     {
//         // Handle error, return a default value, or throw an exception as needed.
//         // You may also want to add additional error checking.
//         return mappedValue;  // Default value, change this as needed.
//     }

//     // Ensure the input value is within the input range
//     if (input < inputMin)
//     {
//         input = inputMin;
//     }
//     else if (input > inputMax)
//     {
//         input = inputMax;
//     }

//     // Perform the mapping
//     float inputRange = inputMax - inputMin;
//     float outputRange = outputMax - outputMin;

//     // Ensure the denominators are not zero before performing the division
//     if (inputRange != 0.0f)
//     {
//         mappedValue = ((input - inputMin) / inputRange) * outputRange + outputMin;
//     }

//     return mappedValue;
// }


float MapSignalFloat(float input, float inputMin, float inputMax, float outputMin, float outputMax) {
    // Ensure that the input is within the specified range
    if (input < inputMin) {
        input = inputMin;
    } else if (input > inputMax) {
        input = inputMax;
    }

    // Map the input to the output range
    float output = outputMin + (input - inputMin) * (outputMax - outputMin) / (inputMax - inputMin);

    // Ensure that the output is within the specified range
    if (output < outputMin) {
        output = outputMin;
    } else if (output > outputMax) {
        output = outputMax;
    }

    return output;
}