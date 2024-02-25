#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "FreeRTOS.h"
#include <string.h>
#include "task.h"

#define UART_ID uart1
#define BAUD_RATE 9600
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_NONE

#define UART_TX_PIN 4
#define UART_RX_PIN 5
#define BUFFER_SIZE 64
const uint LED_PIN1 = 14;
const uint LED_PIN2 = 13;
volatile uint8_t rx_buffer[BUFFER_SIZE];
volatile uint16_t rx_buffer_head = 0;
volatile uint16_t rx_buffer_tail = 0;
uint32_t received_data;
// Flag to indicate data received
volatile bool data_received = false;

// RX interrupt handler
void on_uart_rx()
{
    while (uart_is_readable(UART_ID))
    {
        uint8_t data = uart_getc(UART_ID);
        uint16_t next_head = (rx_buffer_head + 1) % BUFFER_SIZE;
        if (next_head != rx_buffer_tail)
        {
            rx_buffer[rx_buffer_head] = data;
            rx_buffer_head = next_head;
            data_received = true;
        }
    }
}

void uart_task(void *pvParameters)
{
    (void)pvParameters;

    // Set up the UART with specified parameters
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);
    uart_set_irq_enables(UART_ID, true, false);
    
    while (1)
    {
        // Check if there is data in the buffer
        while (rx_buffer_tail != rx_buffer_head)
        {
            received_data = rx_buffer[rx_buffer_tail];
            rx_buffer_tail = (rx_buffer_tail + 1) % BUFFER_SIZE;

            
            printf("%d", received_data);
            
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
void sendUart(void *pvParameters)
{

    (void)pvParameters;
    int counter1 = 0;
    int counter2 = 0;
    int counter3 = 0;
    while (1)
    {
        // Format counters as a string
        // Increment the counters
        counter1++;
        counter2++;
        counter3++;
        if (counter1 > 10)
        {
            counter1 = 0;
        }

        if (counter2 > 10)
        {
            counter2 = 0;
        }
        if (counter3 > 10)
        {
            counter3 = 0;
        }
        char counters_str[20];
        sprintf(counters_str, "%d,%d,%d", counter1, counter2, counter3);
        if (uart_is_writable(UART_ID))
        {
            // Send counters string over UART
            uart_puts(UART_ID, counters_str);
        }

        vTaskDelay(pdMS_TO_TICKS(300));
    }
}
void led_task(void *pvParameters)
{
    (void)pvParameters;

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (1)
    {

        if (data_received)
        {
            gpio_put(LED_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(50));
            gpio_put(LED_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(50));
            data_received = false;
        }
        else
        {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

int main()
{
    stdio_init_all();

    gpio_init(LED_PIN1);
    gpio_init(LED_PIN2);
    gpio_set_dir(LED_PIN1, GPIO_OUT);
    gpio_set_dir(LED_PIN2, GPIO_OUT);
    xTaskCreate(uart_task, "UART", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    xTaskCreate(sendUart, "SendUART", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    xTaskCreate(led_task, "LED", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    vTaskStartScheduler();

    while (1)
    {
        ;
    }
}
