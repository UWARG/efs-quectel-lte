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
	uint16_t bytes_copied = 0;
	if(readIndex <= writeIndex)
	{
		memcpy(buffer, rxBuffer + readIndex, writeIndex - readIndex);
		bytes_copied = writeIndex - readIndex;
	}
	else
	{

		uint16_t tail_chunk = BUFFER_SIZE - readIndex;
		memcpy(buffer, rxBuffer + readIndex, tail_chunk);
		memcpy(buffer + tail_chunk, rxBuffer, writeIndex);
		bytes_copied = tail_chunk + writeIndex;
	}
	buffer[bytes_copied] = '\0';
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

//char LTE::parse(char* at_response, uint8_t param_n)
//{
//	for(uint8_t i = 0; i < strlen(at_response); i++)
//	{
//		if( (at_response[i] == ',') || (at_response[i] == ':') ){
//			param_n--;
//			if(param_n == 0){
//				if(at_response[i] == ':'){
//					return at_response[i+2];
//				}
//				else
//				{
//					return at_response[i+1]; //assuming parameter is just one character
//				}
//			}
//		}
//	}
//}

void LTE::update() {
	static uint32_t wait_start = 0;
	static char buffer[128] = {0};

	switch(currentState){
		case POWER_ON:{
			strcpy(buffer,"ATI\r");
			transmit((uint8_t*)buffer, strlen(buffer));

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
			strcpy(buffer, "AT+CPIN?\r");
			transmit((uint8_t*)buffer, strlen(buffer));

			wait_start = HAL_GetTick();
			currentState = WAIT_SIM_CARD;
			break;
		}
		case WAIT_SIM_CARD:{
    	   	if (newData)
			{
       			receive((uint8_t*)buffer);
       			if(strstr(buffer, "READY")){
       				currentState = CS;
       			}
       			else
       			{
       				//if has password
       				strcpy(buffer, "AT+CPIN=\r"); //password
					transmit((uint8_t*)buffer, strlen(buffer));
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
			strcpy(buffer, "AT+CREG?\r");
			transmit((uint8_t*)buffer, strlen(buffer));

			wait_start = HAL_GetTick();
			currentState = WAIT_CS;
			break;
		}
		case WAIT_CS:{
			if (newData)
			{
				receive((uint8_t*)buffer);
				if( strstr(buffer, ",1") || strstr(buffer, ",5") )
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
			strcpy(buffer, "AT+CEREG?\r");
			transmit((uint8_t*)buffer, strlen(buffer));

			wait_start = HAL_GetTick();
			currentState = WAIT_PS;
			break;
		}
		case WAIT_PS:{
			if (newData)
			{
				receive((uint8_t*)buffer);
				if( strstr(buffer, ",1") || strstr(buffer, ",5") || (HAL_GetTick() - wait_start > CS_WAIT_TIME) )
				{
					currentState = CONFIG_PDP;
				}
			}
			break;
		}
		case CONFIG_PDP:{
			strcpy(buffer, "AT+QICSGP=");
			strcat(buffer, CONTEXT_ID);
			strcat(buffer, ",");
			strcat(buffer, CONTEXT_TYPE);
			strcat(buffer, ",");
			strcat(buffer, APN);
			strcat(buffer, ",");
			strcat(buffer, USERNAME);
			strcat(buffer, ",");
			strcat(buffer, PASSWORD);
			strcat(buffer, ",");
			strcat(buffer, AUTH);
			strcat(buffer, "\r");
			transmit((uint8_t*)buffer, strlen(buffer));

			wait_start = HAL_GetTick();
			currentState = WAIT_CONFIG_PDP;
			break;
		}
		case WAIT_CONFIG_PDP:{
			if (newData)
			{
				if(strstr(buffer, "OK")){
					currentState = ACT_PDP;
				}
			}
			break;
		}
		case ACT_PDP:{
			strcpy(buffer, "AT+QIACT=");
			strcat(buffer, CONTEXT_ID);
			strcat(buffer,  "\r");
			transmit((uint8_t*)buffer, strlen(buffer));

			strcpy(buffer, "AT+QIACT?\r");
			transmit((uint8_t*)buffer, strlen(buffer));

			wait_start = HAL_GetTick();
			currentState = WAIT_ACT_PDP;
			break;
		}
		case WAIT_ACT_PDP:{
			if (newData)
			{
				if(strstr(buffer, "OK")){
					currentState = OPEN_CONNECT;
				}
			}
			if (HAL_GetTick() - wait_start > PDP_ACT_WAIT_TIME)
			{
				//reboot the module
			}
			break;
		}
		case DEACT_PDP:{
			strcpy(buffer, "AT+QIDEACT=");
			strcat(buffer, CONTEXT_ID);
			strcat(buffer, "\r");
			transmit((uint8_t*)buffer, strlen(buffer));

			wait_start = HAL_GetTick();
			currentState = WAIT_DEACT_PDP;
			break;
		}
		case WAIT_DEACT_PDP:{
			if (newData)
			{
				if(strstr(buffer, "OK")){
					currentState = SIM_CARD;
				}
			}
			if (HAL_GetTick() - wait_start > DEACT_WAIT_TIME)
			{
				//reboot the module
			}
			break;
		}
		case OPEN_CONNECT:{
			//open connection
			strcpy(buffer, "AT+QIOPEN=");
			strcat(buffer, CONTEXT_ID);
			strcat(buffer, ",");
			strcat(buffer, CONNECT_ID);
			strcat(buffer, ",");
			strcat(buffer, SERVICE_TYPE);
			strcat(buffer, ",");
			strcat(buffer, IP_ADDR);
			strcat(buffer, ",");
			strcat(buffer, PORT);
			strcat(buffer, ",");
			strcat(buffer, ACCESS_MODE);
			strcat(buffer, "\r");
			transmit((uint8_t*)buffer, strlen(buffer));

			wait_start = HAL_GetTick();
			currentState = WAIT_OPEN_CONNECT;
			break;
		}
		case WAIT_OPEN_CONNECT:{
			if (newData)
			{
				if(strstr(buffer, "OK")){
					currentState = READY;
				}
			}
			if (HAL_GetTick() - wait_start > OPEN_CONNECT_WAIT_TIME)
			{
				currentState = CLOSE_CONNECT;
			}
			break;
		}
       	case CLOSE_CONNECT:{
       		strcpy(buffer, "AT+QICLOSE="),
			strcat(buffer, CONNECT_ID);
       		strcat(buffer, "\r");
       		transmit((uint8_t*)buffer, strlen(buffer));

       		wait_start = HAL_GetTick();
			currentState = WAIT_CLOSE_CONNECT;
			break;
		}
       	case WAIT_CLOSE_CONNECT:{
       		if (newData)
			{
       			if(strstr(buffer, "READY")){
					currentState = OPEN_CONNECT;
				}
			}
       		if (HAL_GetTick() - wait_start > CLOSE_CONNECT_WAIT_TIME)
			{
				//reboot
			}
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
	char buffer[128] = "AT+QPOWD";
	transmit((uint8_t*)buffer, strlen(buffer));
}
