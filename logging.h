/* 
 * File:   global.h
 * Author: alexcon
 *
 * Created on 28 Март 2013 г., 11:56
 */

#ifndef LOGGING_H
#define	LOGGING_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "dmsd.h"
FILE *logfp; 
void logDaemonMessage(const char *format, ...);
void logMessage(const char *format, ...);
void logOpen(const char *name, const char* host);
void logClose(void);
void logStart (struct start_param *stp);
void logExit (const char *message, int code);
int bDebug;
int bStop;
#define LOG_FILE_TEMPLATE "/var/log/dmsd/%s.%s"
#define MAX_LOG_SIZE 500*1024 //100 kB
#define LOG_DELIMITER "========================================================="
#ifdef	__cplusplus
}
#endif

#endif	/* LOGGING_H */

