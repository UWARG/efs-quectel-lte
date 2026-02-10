#include "lte.hpp"
#include <cstring>

LTE::LTE(UART_HandleTypeDef* huart):
	huart(huart){}

LTE::~LTE() = default;

void LTE::transmit(const uint8_t* data, uint16_t size)
{
	if(huart)
	{
		HAL_UART_Transmit_DMA(huart, data, size);
	}
}

void LTE::startReceive()
{
	if(huart)
	{
		HAL_UARTEx_ReceiveToIdle_DMA(huart, rxBuffer, BUFFER_SIZE);
		__HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT);
	}
}

void LTE::receive(uint8_t* buffer)
{
	if(readIndex <= writeIndex)
	{
		memcpy(buffer, rxBuffer + readIndex, writeIndex - readIndex);
	}
	else
	{
		memcpy(buffer, rxBuffer + readIndex, BUFFER_SIZE - readIndex);
		memcpy(buffer, rxBuffer, readIndex - writeIndex);
	}
	readIndex = writeIndex;
//	for (int i = 0; i < bufferSize; i++)
//	{
//		if(readIndex == writeIndex)
//		{
//			newData = false;
//			return i;
//		}
//		buffer[i] = rxBuffer[readIndex];
//		readIndex++;
//		if (readIndex >= BUFFER_SIZE)
//		{
//			readIndex = 0;
//		}
//	}
//	newData = false;
//	return bufferSize;
}

void LTE::setNewData()
{
	newData = true;
}


void LTE::receiveCallback(uint16_t write_index)
{
	writeIndex = write_index % BUFFER_SIZE;
}

UART_HandleTypeDef* LTE::getHuart() const
{
	return huart;
}

bool LTE::getNewData()
{
	return newData;
}
//
//void LTE::update() {
//	uint32_t wait_start;
//
//	switch(currentState){
//		case POWER_ON:
//			char tx_buffer[128] = "ATI";
//			transmit((uint8_t*)tx_buffer, strlen((uint8_t*)tx_buffer));
//			wait_start = HAL_GetTick();
//			currentState = POWER_ON_WAIT;
//			break;
//		case POWER_ON_WAIT:
//			if (newData)
//			{
//				nextState = SIM_CARD;
//			}
//			if (HAL_GetTick() - wait_start > POWER_ON_WAIT)
//			{
//				currentState = POWER_ON;
//			}
//       case SIM_CARD:
//       	tx_buffer = "AT+CPIN?";
//			transmit((uint8_t*)tx_buffer, strlen((uint8_t*)tx_buffer));
//			wait_start = HAL_GetTick();
//			currentState = POWER_ON_WAIT;
//			break;
//       case WAIT_SIM_CARD:
//       	if (newData)
//			{
//				nextState = DNS;
//			}
//			if (HAL_GetTick() - wait_start > POWER_ON_WAIT)
//			{
//				currentState = POWER_ON;
//			}
//			break;
//       case CS:
//       	tx_buffer = "AT+CREG?";
//			transmit((uint8_t*)tx_buffer, strlen((uint8_t*)tx_buffer));
//			wait_start = HAL_GetTick();
//			currentState = POWER_ON_WAIT;
//			break;
//		case WAIT_CS:
//			if (newData)
//			{
//				nextState = DNS;
//			}
//			if (HAL_GetTick() - wait_start > POWER_ON_WAIT)
//			{
//				currentState = POWER_ON;
//			}
//			break;
//		case PS:
//			break;
//		case WAIT_PS:
//			break;
//		case PDP:
//			break;
//		case WAIT_PS:
//			break;
//       case DNS:
//			break;
//       case WAIT_DNS:
//			break;
//       case READY:
//			break;
//		default:
//			break;
//	}
//}
//
//void LTE::stopUpdate()
//{
//	char tx_buffer[128] = "AT+QPOWD";
//	transmit((uint8_t*)tx_buffer, strlen((uint8_t*)tx_buffer));
//}
