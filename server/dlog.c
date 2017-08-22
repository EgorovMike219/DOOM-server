#include "dlog.h"




/// Normal message type
#define MES_LOG_NORMAL 1

/// "Stop" message type. Send once to stop 'logger' thread
#define MES_LOG_STOP 592


/// Message structure
struct msgbuf_log {
	long type;
	char text[LENGTH_LOG_MESSAGE];
};


/// Message queue ID (private ownership)
int msqid_log;

/// Log file pointer
FILE* log_file_ptr;

/// Is log initialized
int is_active = 0;

/// Is 'log' an `stderr` stream
int is_stderr = 0;




/// Thread identifier for 'logger' thread
pthread_t logger_id;




/**
 * @brief Thread function. Writes logs
 */
void* logger(void* dummy) {
	// Dummy-check
	if (dummy != NULL) {
		return NULL;
	}
	
	struct msgbuf_log message;
	
	struct timeval raw_time;
	struct tm parsed_time;
	
	fprintf(log_file_ptr, "\nLOGGING START\n\n");
	
	while (1) {
		// Receive message. If an error occurs, do nothing
		if (msgrcv(msqid_log,
				   &message, LENGTH_LOG_MESSAGE, 0, MSG_NOERROR) < 0) {
			errno = 0;
			continue;
		}
		
		// Thread stop
		if (message.type == MES_LOG_STOP) {
			break;
		}
		
		// Print time
		gettimeofday(&raw_time, NULL);
		localtime_r(&raw_time.tv_sec, &parsed_time);
		raw_time.tv_usec /= 1000;
		fprintf(log_file_ptr, "[%02d:%02d:%02d.%03d] ",
				parsed_time.tm_hour, parsed_time.tm_min, parsed_time.tm_sec,
				(int)raw_time.tv_usec);
		
		// Print message and format
		fprintf(log_file_ptr, "%s", message.text);
		fprintf(log_file_ptr, "\n");
	}
	
	fprintf(log_file_ptr, "\nLOGGING END\n");
	
	return NULL;
}




int d_log_initialize(const char* log_file, char mode) {
	// Checks
	if (is_active == 1) {
		return 0;
	}
	if (log_file == NULL) {
		return -1;
	}
	
	// Open log file
	if (mode == 'n') {
		log_file_ptr = fopen(log_file, "w");
	}
	else if (mode == 'a') {
		log_file_ptr = fopen(log_file, "a");
	}
	else if (mode == 't') {
		log_file_ptr = stderr;
		is_stderr = 1;
	}
	else {
		return -1;
	}
	
	// Check log file
	if (log_file_ptr == NULL) {
		errno = 0;
		return -1;
	}
	
	// Open message queue
	msqid_log = msgget(IPC_PRIVATE, 0666);
	if (msqid_log < 0) {
		if (is_stderr == 0) {
			fclose(log_file_ptr);
		}
		return -1;
	}
	
	// Launch logger thread
	if (pthread_create(&logger_id, NULL, logger, NULL) < 0) {
		if (is_stderr == 0) {
			fclose(log_file_ptr);
		}
		return -1;
	}
	
	is_active = 1;
	return 0;
}




void d_log(const char* text) {
	struct msgbuf_log message;
	
	if ((text == NULL) || (is_active == 0)) {
		return;
	}
	
	strcpy(message.text, text);
	message.type = MES_LOG_NORMAL;
	msgsnd(msqid_log, &message, LENGTH_LOG_MESSAGE, 0);
}




void d_log_close(void) {
	struct msgbuf_log message;
	
	// Checks
	if (is_active == 0) {
		return;
	}
	
	// Stop logger
	message.type = MES_LOG_STOP;
	msgsnd(msqid_log, &message, LENGTH_LOG_MESSAGE, 0);
	pthread_join(logger_id, NULL);
	
	if (is_stderr == 0) {
		fclose(log_file_ptr);
	}
	
	is_active = 0;
}



