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
		uint16_t tail_chunk = BUFFER_SIZE - readIndex;
		memcpy(buffer, rxBuffer + readIndex, tail_chunk);
		memcpy(buffer + tail_chunk, rxBuffer, writeIndex);
	}
	newData = false;
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
	writeIndex = write_index;
}

UART_HandleTypeDef* LTE::getHuart() const
{
	return huart;
}

bool LTE::getNewData()
{
	return newData;
}

void LTE::update() {
	uint32_t wait_start = 0;

	switch(currentState){
		case POWER_ON:{
			char tx_buffer[128] = "ATI\r";
			transmit((uint8_t*)tx_buffer, strlen(tx_buffer));
			wait_start = HAL_GetTick();
			currentState = POWER_ON_WAIT;
			break;
		}
		case POWER_ON_WAIT:{
			if (newData)
			{
				currentState = SIM_CARD;
			}
			if (HAL_GetTick() - wait_start > POWER_ON_WAIT_TIME)
			{
				currentState = POWER_ON;
			}
			break;
		}
       case SIM_CARD:{
    	   	char tx_buffer[128] = "AT+CPIN?";
			transmit((uint8_t*)tx_buffer, strlen(tx_buffer));
			wait_start = HAL_GetTick();
			currentState = POWER_ON_WAIT;
			break;
       }
       case WAIT_SIM_CARD:{
       	if (newData)
			{
       			currentState = DNS;
			}
			if (HAL_GetTick() - wait_start > SIM_WAIT_TIME)
			{
				currentState = POWER_ON;
			}
			break;
       }
       case CS:{
    	   	char tx_buffer[128] = "AT+CREG?";
			transmit((uint8_t*)tx_buffer, strlen(tx_buffer));
			wait_start = HAL_GetTick();
			currentState = POWER_ON_WAIT;
			break;
       }
       case WAIT_CS:{
			if (newData)
			{
				currentState = DNS;
			}
			if (HAL_GetTick() - wait_start > POWER_ON_WAIT)
			{
				currentState = POWER_ON;
			}
			break;
       }
		case PS:{
			break;
		}
		case WAIT_PS:{
			break;
		}
		case PDP:{
			break;
		}
		case WAIT_PDP:{
			break;
		}
       	case DNS:{
			break;
		}
       	case WAIT_DNS:{
			break;
		}
       	case READY:{
			break;
		}
		default:
			break;
	}
}

void LTE::stopUpdate()
{
	char tx_buffer[128] = "AT+QPOWD";
	transmit((uint8_t*)tx_buffer, strlen(tx_buffer));
}
