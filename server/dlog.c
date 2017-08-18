#include "dlog.h"




/// Normal message type
#define MES_LOG_NORMAL 1

/// "Stop" message type. Send once to stop 'logger' thread
#define MES_LOG_STOP 592


/// Message structure
struct msgbuf_log {
	long type;
	char text[LOG_MESSAGE_LENGTH];
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
	struct msgbuf_log message;
	
	while (1) {
		msgrcv(msqid_log, &message, LOG_MESSAGE_LENGTH, 0, MSG_NOERROR);
		
		if (message.type == MES_LOG_STOP) {
			break;
		}
		
		fprintf(log_file_ptr, "%s", message.text);
		fprintf(log_file_ptr, "\n");
	}
	
	return NULL;  // Equivalent to pthread_exit(NULL)
}




int d_log_initialize(const char* log_file, char mode) {
	if (is_active == 1) {
		return 0;
	}
	if (log_file == NULL) {
		log_file = "./game.log";
	}
	
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
	
	msqid_log = msgget(IPC_PRIVATE, 0);
	if (msqid_log < 0) {
		if (is_stderr == 0) {
			fclose(log_file_ptr);
		}
		return -1;
	}
	
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
	
	msgsnd(msqid_log, &message, LOG_MESSAGE_LENGTH, 0);
}




void d_log_close(void) {
	struct msgbuf_log message;
	message.type = MES_LOG_STOP;
	msgsnd(msqid_log, &message, LOG_MESSAGE_LENGTH, 0);
	
	pthread_join(logger_id, NULL);
	
	if (is_stderr == 0) {
		fclose(log_file_ptr);
	}
	
	is_active = 0;
}



