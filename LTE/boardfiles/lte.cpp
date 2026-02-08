#include "lte.hpp"

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
	}
}

uint16_t LTE::receive(uint8_t* buffer, uint16_t bufferSize)
{
	for (int i = 0; i < bufferSize; i++)
	{
		if(readIndex == writeIndex)
		{
			return i;
		}
		buffer[i] = rxBuffer[readIndex];
		readIndex++;
		if (readIndex >= BUFFER_SIZE)
		{
			writeIndex = 0;
		}
	}
	newData = false;
	return bufferSize;
}

void LTE::setNewData()
{
	newData = true;
}


void LTE::receiveCallback(uint16_t size)
{
	writeIndex = size;
}

UART_HandleTypeDef* LTE::getHuart() const
{
	return huart;
}

bool LTE::getNewData()
{
	return newData;
}

void sendATCommand(const char* command, const char* arguments) {
  // Command should be in the form "AT+..." or "AT&..."
  char tx_buffer[128];

  //sprintf(tx_buffer, "%s\r", command);
  
  transmit(tx_buffer, strlen((char *)tx_buffer));
}

void update() {
	switch(currentState){
		case POWER_ON:
			break;
        case POWER_OFF:
			break;
        case SIM_CARD:
			break;
        case WAIT_SIM_CARD:
			break;
        case DNS:
			break;
        case WAIT_DNS:
			break;
        case READY:
			break;
		default:
			break;
	}
}
