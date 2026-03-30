#include "API_delay.h"
#include <stdio.h> // for the NULL definition
#include "stm32f4xx_hal.h" // for the delay

void delayInit(delay_t *delay, tick_t duration) {
	if (delay == NULL || duration == 0)
		return;

	delay->duration = duration;
	delay->running = false;
}

bool_t delayRead(delay_t *delay) {
	if (delay == NULL)
		return false;

	if (delay->duration == 0)
		return false;

	// Start the delay if it's not running
	if (!delay->running) {
		delay->startTime = HAL_GetTick();
		delay->running = true;
		return false;
	}

	// Checks if the timer has finished
	if ((uint32_t) (HAL_GetTick() - delay->startTime) > delay->duration) {
		delay->running = false;
		return true;
	}

	// Reached if the timer is running but not finished
	return false;
}

void delayWrite(delay_t *delay, tick_t duration) {
	if (delay == NULL || duration == 0)
		return;

	delay->duration = duration;
}

bool_t delayIsRunning(delay_t *delay) {
	if (delay == NULL)
		return;

	return delay->running;
}
