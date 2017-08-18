#ifndef DLOG_H
#define DLOG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>




/// Maximum allowed length of an error message
#define LOG_MESSAGE_LENGTH 256




/**
 * @brief Initialize logging
 *
 * @param log_file If NULL, standard log file is used
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
 *
 * @param message Message to log.
 * A given string should not contain "newline" symbol at the end.
 * It will be printed to log file (or stream) unchanged, except for this symbol
 */
void d_log(const char* message);


/**
 * @brief Safely stop logging
 */
void d_log_close(void);




#endif // DLOG_H
