#ifndef DLOG_H
#define DLOG_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>




/// Maximum allowed length of an error message
#define LENGTH_LOG_MESSAGE 256




/**
 * @brief Initialize logging
 *
 * @param log_file
 * @param mode = 'n': Rewrite log file
 * @param mode = 'a': Add after existing log file
 * @param mode = 't': Output log to `stderr`
 *
 * @return 0 if successful
 * @return -1 for errors
 */
int d_log_initialize(const char* log_file, char mode);


/**
 * @brief Write to log file or stream
 * @note Safe to call at any moment
 *
 * @param message Message to log.
 * A "newline" symbol will be added automatically
 */
void d_log(const char* message);


/**
 * @brief Safely stop logging
 * @note Safe to call at any moment
 */
void d_log_close(void);




#endif // DLOG_H
