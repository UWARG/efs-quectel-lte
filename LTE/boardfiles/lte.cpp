#include "lte.hpp"

LTE::LTE(UART_HandleTypeDef* huart):
	huart(huart){}

LTE::~LTE() = default;

void LTE::transmit(const uint8_t* data, uint16_t size)
{
	HAL_UART_Transmit(huart, data, size, 100);
}

uint16_t LTE::receive(uint8_t* buffer, uint16_t bufferSize)
{
	HAL_UART_Receive(huart, buffer, bufferSize, 100);
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
