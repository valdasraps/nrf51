/***************************************************************
 *
 * OpenBeacon.org - nRF51 ADC Support Routines
 *
 * Copyright 2014 Milosch Meriac <meriac@openbeacon.de>
 *
 ***************************************************************

 This file is part of the OpenBeacon.org active RFID firmware

 OpenBeacon is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 OpenBeacon is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Foobar.  If not, see <http://www.gnu.org/licenses/>.

 */
#include <config.h>
#include <adc.h>

static volatile uint8_t g_battery_voltage;
static volatile bool adc_wait = false;

void ADC_IRQHandler(void) {
	if (NRF_ADC->EVENTS_END) {

		/* acknowledge event */
		NRF_ADC->EVENTS_END = 0;

		/* read battery voltage */
		g_battery_voltage = (((uint16_t) (NRF_ADC->RESULT & 0xFF)) * 36) >> 8;

		/* disable ADC after sampling voltage */
		NRF_ADC->TASKS_STOP = 1;
		NRF_ADC->ENABLE = 0;

		adc_wait = false;
	}
}

void adc_start(void) {

	/* start ADC voltage conversion */
	NRF_ADC->ENABLE = 1;
	NRF_ADC->TASKS_START = 1;
	adc_wait = true;

}

uint8_t adc_bat(bool wait) {
	if (wait) {
		while (adc_wait) __WFE();
	}
	return g_battery_voltage;
}

void adc_init(void) {
	/* initialize batter voltage */
	g_battery_voltage = 0;

	/* setup ADC for capturing battery voltage */
	NRF_ADC->CONFIG =
			 ((ADC_CONFIG_REFSEL_VBG << ADC_CONFIG_REFSEL_Pos)
			| (ADC_CONFIG_INPSEL_SupplyOneThirdPrescaling << ADC_CONFIG_INPSEL_Pos)
			| (ADC_CONFIG_RES_8bit << ADC_CONFIG_RES_Pos));
	NRF_ADC->INTENSET = ((ADC_INTENSET_END_Enabled << ADC_INTENSET_END_Pos));

	NVIC_SetPriority(ADC_IRQn, IRQ_PRIORITY_ADC);
	NVIC_EnableIRQ(ADC_IRQn);
}
