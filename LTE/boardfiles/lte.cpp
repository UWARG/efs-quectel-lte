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

char LTE::parse(char* at_response, uint8_t param_n)
{
	for(uint8_t i = 0; i < strlen(at_response); i++)
	{
		if( (at_response[i] == ',') | (at_response[i] == ':') ){
			param_n--;
			if(param_n == 0){
				if(at_response[i] == ':'){
					return at_response[i+2];
				}
				else
				{
					return at_response[i+1]; //assuming parameter is just one character
				}
			}
		}
	}
}

void LTE::update() {
	static uint32_t wait_start = 0;
	static char b_receive[100] = {0};

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
    	   	char tx_buffer[128] = "AT+CPIN?\r";
			transmit((uint8_t*)tx_buffer, strlen(tx_buffer));
			wait_start = HAL_GetTick();
			currentState = POWER_ON_WAIT;
			break;
		}
		case WAIT_SIM_CARD:{
    	   	if (newData)
			{
       			receive((uint8_t*)b_receive);
       			if(strcmp(b_receive, "+CPIN: READY\r") == 0){
       				currentState = DNS;
       			}
       			else
       			{
       				//if has password
       				char tx_buffer[128] = "AT+CPIN=\r"; //password
					transmit((uint8_t*)tx_buffer, strlen(tx_buffer));
					wait_start = HAL_GetTick();
       			}
			}
			if (HAL_GetTick() - wait_start > SIM_WAIT_TIME)
			{
				currentState = POWER_ON;
			}
			break;
		}
		case CS:{
			char tx_buffer[128] = "AT+CREG?\\r";
			transmit((uint8_t*)tx_buffer, strlen(tx_buffer));
			wait_start = HAL_GetTick();
			currentState = POWER_ON_WAIT;
			break;
		}
		case WAIT_CS:{
			if (newData)
			{
				receive((uint8_t*)b_receive);
				if( (parse(b_receive, 1) == 1) | (parse(b_receive, 1) == 5) )
				{
					currentState = PS;
				}
			}
			if (HAL_GetTick() - wait_start > CS_WAIT_TIME)
			{
				currentState = POWER_ON;
			}
			break;
		}
		case PS:{
			char tx_buffer[128] = "AT+CEREG?\r";
			transmit((uint8_t*)tx_buffer, strlen(tx_buffer));
			wait_start = HAL_GetTick();
			currentState = WAIT_PS;
			break;
		}
		case WAIT_PS:{
			if (newData)
			{
				receive((uint8_t*)b_receive);
				if( (parse(b_receive, 1) == 1) | (parse(b_receive, 1) == 5) | (HAL_GetTick() - wait_start > CS_WAIT_TIME) )
				{
					currentState = PDP;
				}
			}
			break;
		}
		case PDP:{
			char tx_buffer[128] = "AT+QICSGP=<1>\r";
			transmit((uint8_t*)tx_buffer, strlen(tx_buffer));
			wait_start = HAL_GetTick();
			currentState = WAIT_PS;
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
