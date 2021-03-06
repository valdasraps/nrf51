#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "radio.h"
#include "uart.h"
#include "radio.h"
#include "aes.h"
#include "crc32.h"

static Packet p;
static uint8_t enc[sizeof(p)];

int main(void) {

    /* Start 16 MHz crystal oscillator */
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART    = 1;

    /* Wait for the external oscillator to start up */
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);

	NRF_RNG->CONFIG = 0;
	NRF_RNG->TASKS_START = 1;
	while (NRF_RNG->EVENTS_VALRDY == 0);

    nrf_gpio_cfg_output(LED_RGB_RED);
    nrf_gpio_pin_set(LED_RGB_RED);

    uart_init(TX_PIN_NUMBER, RX_PIN_NUMBER);
	radio_init();

	uint32_t oid = crc32(&NRF_FICR->DEVICEID, sizeof(NRF_FICR->DEVICEID));
	aes_init(oid);

    // Set payload pointer
    NRF_RADIO->PACKETPTR = (uint32_t) &enc;

    p.seq = 1;
    p.oid = NRF_RNG->VALUE;
    NRF_RNG->TASKS_START = 1;

	while (true) {

		//nrf_gpio_pin_set(LED_RGB_BLUE);

		if (aes_encr(&p, &enc, sizeof(enc), SIGNATURE_SIZE) == 0) {

			radio_send();

		}

		//uart_putstring((const uint8_t *) "Sent ");
		//uart_put(packet[0]);
		//uart_putstring("\n\r");

		//nrf_gpio_pin_clear(LED_RGB_BLUE);
		//nrf_delay_us(1000000);

		//nrf_gpio_pin_set(LED_RGB_RED);

		//if (radio_receive()) {
		//	uart_put(packet[0]);
		//	uart_putstring("\n\r");
		//} else {
		//	uart_putstring((const uint8_t *) "Bad CRC");
		//}

		//nrf_gpio_pin_clear(LED_RGB_RED);
		nrf_delay_us(100000);

		p.seq += 1;
		p.oid = NRF_RNG->VALUE;

		NRF_RNG->TASKS_START = 1;

		nrf_gpio_pin_clear(LED_RGB_RED);
		nrf_delay_ms(5);
		nrf_gpio_pin_set(LED_RGB_RED);

	}
}
