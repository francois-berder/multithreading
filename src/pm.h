
#ifndef PM_H
#define PM_H

/**
 * @brief Enter low power mode
 *
 * By default, it executes WFI instruction.
 * The function can be redefined.
 */
void pm_enter(void);

#endif
