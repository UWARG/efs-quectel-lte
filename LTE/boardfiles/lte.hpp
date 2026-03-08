#pragma once

#include "stm32l5xx_hal.h"
//#include "stm32l5xx_uart_hal.h"

#include <cstdint>

#define BUFFER_SIZE 	4000
#define POWER_ON_WAIT_TIME	20000
#define SIM_WAIT_TIME		20000
#define CS_WAIT_TIME		90000
#define PS_WAIT_TIME		60000


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
		PDP,
		WAIT_PDP,
        DNS,
        WAIT_DNS,
        READY
    };

    State currentState;


};

