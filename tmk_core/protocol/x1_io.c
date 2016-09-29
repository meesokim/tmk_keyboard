#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include <config.h>

/* Check port settings for data line */
#if !(defined(X1_DATA_PORT) && \
      defined(X1_DATA_PIN) && \
      defined(X1_DATA_DDR) && \
      defined(X1_DATA_BIT))
#   error "X1 data port setting is required in config.h"
#endif


/*
 * Clock
 */
void clock_init(void)
{
}

void clock_lo(void)
{
}

void clock_hi(void)
{
}

bool clock_in(void)
{
    return 0;
}

/*
 * Data
 */
void data_init(void)
{
}

void data_lo(void)
{
    X1_DATA_PORT &= ~(1<<X1_DATA_BIT);
    X1_DATA_DDR  |=  (1<<X1_DATA_BIT);
}

void data_hi(void)
{
    /* input with pull up */
    X1_DATA_DDR  &= ~(1<<X1_DATA_BIT);
    X1_DATA_PORT |=  (1<<X1_DATA_BIT);
}

bool data_in(void)
{
    X1_DATA_DDR  &= ~(1<<X1_DATA_BIT);
    X1_DATA_PORT |=  (1<<X1_DATA_BIT);
    _delay_us(1);
    return X1_DATA_PIN&(1<<X1_DATA_BIT);
}
