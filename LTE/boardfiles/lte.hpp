#pragma once

#include "stm32l5xx_hal.h"
#include "stm32l5xx_uart_hal.h"
#include <cstdint>

#define BUFFER_SIZE 8096

class LTE {

public:

    LTE(UART_HandleTypeDef* huart);
    ~LTE();

    void transmit(const uint8_t* data, uint16_t size);
    uint16_t receive(uint8_t* buffer, uint16_t bufferSize);

    // Getters
    UART_HandleTypeDef* getHuart() const;

    // DMA callback
    void receiveCallback(uint16_t size);

    // Start DMA
    void startReceive();

    //state machine action????
    void update();

private:
    UART_HandleTypeDef* huart;
    uint8_t rxBuffer[BUFFER_SIZE];
    uint16_t readIndex;
    uint16_t writeIndex;
    bool newData; //ISR Fodder

    enum state {
        POWER_ON,
        POWER_OFF,
        SIM_CARD,
        WAIT_SIM_CARD,
        DNS,
        WAIT_DNS,
        READY
    };


};

