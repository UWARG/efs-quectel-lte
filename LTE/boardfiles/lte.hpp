#pragma once

#include "stm32l5xx_hal.h"
//#include "stm32l5xx_uart_hal.h"

#include <cstdint>

#define BUFFER_SIZE 	4000
#define POWER_ON_WAIT_TIME	20000
#define SIM_WAIT_TIME		20000
#define CS_WAIT_TIME		90000
#define PS_WAIT_TIME		60000
#define PDP_ACT_WAIT_TIME 		150000
#define OPEN_CONNECT_WAIT_TIME	150000
#define CLOSE_CONNECT_WAIT_TIME 10000
#define DEACT_WAIT_TIME			40000

#define CONTEXT_ID 		"1"
#define CONNECT_ID		"0"
#define SERVICE_TYPE	"UDP"
#define IP_ADDR			""
#define PORT			""
#define ACCESS_MODE		"2"	//transparent access mode
#define CONTEXT_TYPE	"1"
#define APN				""	//need
#define USERNAME		""	//prob dont need
#define PASSWORD		""	//prob dont need
#define AUTH			"0"


class LTE {

public:

    LTE(UART_HandleTypeDef* huart);
    ~LTE();

    void transmit(const uint8_t* data, uint16_t size);
    void receive(uint8_t* buffer);

    // Getters
    UART_HandleTypeDef* getHuart() const;
    bool getNewData();

    // DMA callback
    void receiveCallback(uint16_t write_index);

    // Start DMA
    void startReceive();

    //state machine action????
    void update();
    void stopUpdate();

    void setNewData();

private:
    char parse(char* at_response, uint8_t param_n);

    UART_HandleTypeDef* huart;
    uint8_t rxBuffer[BUFFER_SIZE];
    volatile uint16_t readIndex;
    volatile uint16_t writeIndex;
    volatile bool newData; //ISR Fodder

    enum State {
        POWER_ON,
        POWER_ON_WAIT,
        POWER_OFF,
        SIM_CARD,
        WAIT_SIM_CARD,
		CS,
		WAIT_CS,
		PS,
		WAIT_PS,
		CONFIG_PDP,
		WAIT_CONFIG_PDP,
		ACT_PDP,
		WAIT_ACT_PDP,
		DEACT_PDP,
		WAIT_DEACT_PDP,
		OPEN_CONNECT,
		WAIT_OPEN_CONNECT,
		CLOSE_CONNECT,
		WAIT_CLOSE_CONNECT,
        READY
    };

    State currentState;


};

