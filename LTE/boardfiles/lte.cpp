#include "lte.hpp"

LTE::LTE(UART_HandleTypeDef* huart):
	huart(huart){}

void LTE::transmit(const uint8_t* data, uint16_t size)
{
	HAL_UART_Transmit(huart, data, size, 100);
}

uint16_t LTE::receive(uint8_t* buffer, uint16_t bufferSize)
{
	HAL_UART_Receive(huart, buffer, bufferSize, 100);
}
