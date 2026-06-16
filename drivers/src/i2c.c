// including the required header files

/** standard C header files */
#include <stddef.h>

/** driver header file */
#include "i2c.h"
// ===================================================================================
// definitions

#define SCL_PIN 6
#define SDA_PIN 7

// I2C identifier as the 'alternate-function' user of the gpio
#define I2C_AF 6U

/** this value was taken from the CubeIDE repository */
// Note: this is packing value which will be parsed into different cfg parts
#define I2C_TIMING_400KHZ  0x0090273D
// ===================================================================================
// Exposed APIs

int i2c_init(void){
    /** enabling the clock for the I2C & GPIO peripheral */
    RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
    RCC->APBENR1 |= RCC_APBENR1_I2C1EN;

    /** set the gpio mode to AF (alternate function) for the clock pin */
    // (clear and set)
    GPIOB->MODER &= ~(3U << SCL_PIN * 2);
    GPIOB->MODER |= GPIO_MODE_AF << SCL_PIN * 2;

    /** set the gpio mode to AF (alternate function) for the data pin */
    // (clear and set)
    GPIOB->MODER &= ~(3U << SDA_PIN * 2);
    GPIOB->MODER |= GPIO_MODE_AF << SDA_PIN * 2;

    /** attaching the alternate function using the gpio pins */
    GPIOB->AFR[0] &= ~(0xFU << (SCL_PIN * 4));
    GPIOB->AFR[0] |=  (I2C_AF   << (SCL_PIN * 4));

    GPIOB->AFR[0] &= ~(0xFU << (SDA_PIN * 4));
    GPIOB->AFR[0] |=  (I2C_AF   << (SDA_PIN * 4));

    /** making the pin open drain */
    GPIOB->OTYPER |= (1U << SCL_PIN) | (1U << SDA_PIN);

    /** I2C requires pull-ups, we declare that for the pins */
    GPIOB->PUPDR &= ~(3U << (SCL_PIN * 2));
    GPIOB->PUPDR |=  (1U << (SCL_PIN * 2));

    GPIOB->PUPDR &= ~(3U << (SDA_PIN * 2));
    GPIOB->PUPDR |=  (1U << (SDA_PIN * 2));

    /** make the gpio pins high speed (better raise/fall times) */
    // optional
    GPIOB->OSPEEDR |= (3U << (SCL_PIN * 2));
    GPIOB->OSPEEDR |= (3U << (SDA_PIN * 2));

    /** disable I2C before configuration */
    I2C1->CR1 &= ~I2C_CR1_PE;

    /** configuring the i2c peripheral clock */
    I2C1->TIMINGR = I2C_TIMING_400KHZ; 

    /** enabling the I2C peripheral */
    I2C1->CR1 |= I2C_CR1_PE;

    return 0;
}

int i2c_write(const uint8_t *data, uint16_t data_len, uint8_t address){
    // total allowable response timeout
    uint32_t timeout = 365000;

    /* wait until bus is free */
    while (I2C1->ISR & I2C_ISR_BUSY){
        timeout --;

        if (timeout <= 0) return -5;
        if (I2C1->ISR & I2C_ISR_NACKF) return -8;
    };

    /** configure the control register for the transfer */
    I2C1->CR2 = (address << 1) | (data_len << 16) | I2C_CR2_START | I2C_CR2_AUTOEND; 

    /* transmission routine */
    timeout = 365000;
    for (int b=0; b<data_len; b++){
        /** wait until TX path is ready */
        while(!(I2C1->ISR & I2C_ISR_TXIS)){
            timeout --;

            if (timeout <= 0) return -6;
            if (I2C1->ISR & I2C_ISR_NACKF) return -8;
        };

        I2C1->TXDR = data[b];
    }

    /* wait until transfer is complete */
    timeout = 365000;
    while(!(I2C1->ISR & I2C_ISR_TC)){
        timeout --;

        if (timeout <= 0) return -7;
        if (I2C1->ISR & I2C_ISR_NACKF) return -8;
    }

    return 0;
}

int i2c_read(uint8_t *data, uint16_t data_len, uint8_t address){
    // total response timeout
    uint32_t timeout = 365000;

    if (!data) return -1;

    /* wait until bus is free */
    while (I2C1->ISR & I2C_ISR_BUSY){
        timeout --;

        if (timeout <= 0) return -8;
    }

    /* configure read transfer */
    I2C1->CR2 = ((uint32_t)address << 1) | ((uint32_t)data_len <<16) | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_AUTOEND;

    /* receive bytes */
    timeout = 365000;
    for (int b =0; b<data_len; b++){
        while(!(I2C1->ISR & I2C_ISR_RXNE)){
            timeout --;

            if (timeout <=0) return -6;
            if (I2C1->ISR & I2C_ISR_NACKF) return -8;
        }

        data[b] = (uint8_t)I2C1->RXDR;
    }

    timeout = 365000;
    while(!(I2C1->ISR & I2C_ISR_TC)){
        timeout --;

        if (timeout <= 0) return -6;
    }

    return 0;
}