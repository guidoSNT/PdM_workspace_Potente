#ifndef API_INC_API_DELAY_H_
#define API_INC_API_DELAY_H_
#include <stdint.h>
#include <stdbool.h>

typedef uint32_t tick_t; //! The stdint has to be used, but it already is included
typedef bool bool_t; //! custom bool type

typedef struct {
	tick_t startTime; //! The time that the delay was initialized
	tick_t duration; //! The duration in ms of the delay
	bool_t running; //! Current state of the delay
} delay_t;

/**
 * @brief Sets the duration and default config of the delay.
 *
 * @param[in,out] Delay pointer to the delay object.
 * @param[in] Duration value of the time to sleep.
 *
 * @retval None.
 */
void delayInit(delay_t *delay, tick_t duration);

/**
 * @brief Reads the current state of the timer.
 *
 * Stars the timer if not running so it also works as a start condition.
 *
 * @param[in,out] Delay pointer to the delay object.
 *
 * @retval True if duration time has passed.
 */
bool_t delayRead(delay_t *delay);

/**
 * @brief Changes the delay time to the delay object.
 *
 * @param[in,out] Delay pointer to the delay object.
 * @param[in] Duration value of the time to sleep.
 *
 * @retval None.
 */
void delayWrite(delay_t *delay, tick_t duration);

/**
 * @brief Gives the state of the delay.
 *
 * @param[in,out] Delay pointer to the delay object.
 *
 * @retval The running state.
 */
bool_t delayIsRunning(delay_t *delay);

/**
 * @brief Stops the delay if running.
 *
 * @param[in,out] Delay pointer to the delay object.
 *
 * @retval None.
 */
void delayStop(delay_t *delay);
#endif /* API_INC_API_DELAY_H_ */
