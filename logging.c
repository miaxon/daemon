#include <stdarg.h>
#include <error.h>
#include <sys/stat.h>
#include <time.h>
#include "tlpi_hdr.h"
#include "logging.h"
#define DEFULT_EXIT_REASON "UNKNOWN"

void
logExit (const char* message, int code)
{
  const char *msg = DEFULT_EXIT_REASON;
  if (message != NULL)
    msg = message;
  logDaemonMessage ("EXIT with reason: '%s' [%s] (code = %d)", msg, strerror (code), code);
  logClose ();
  if(code != 0)
    {
      char buf[1024] = {0};
      sprintf(buf,"Abnormal exit with reason: %s [%s] (code = %d), user %s, pid %d", msg, strerror (code), code, userNameFromId (currentUid), getpid() );
      panic(buf);
    }
  exit (code);
}

void
logStart (struct start_param *stp)
{
  logDaemonMessage ("Starting dmsd daemon: [user: %s, host: %s, debug: %d, conf-file: %s]",
              stp->userName,
              stp->hostName,
              bDebug,
              stp->confFile);
}

void
logDaemonMessage (const char *format, ...)
{
  va_list argList;
  time_t t = time (0);
  struct tm tstruct = *localtime (&t);
  char tstr[256] = "??Unknown time??: ";
  strftime (tstr, sizeof (tstr), "%Y-%m-%d %H:%M", &tstruct);

  fprintf (logfp, "%s [%d]: ", tstr, currentPid);
  va_start (argList, format);
  vfprintf (logfp, format, argList);
  fprintf (logfp, "\n");
  va_end (argList);
  if (bDebug)
    {
      printf ("%s [%d]: ", tstr, currentPid);
      va_start (argList, format);
      vprintf (format, argList);
      printf ("\n");
      va_end (argList);
    }
}

void
logMessage (const char *format, ...)
{
  va_list argList;
  time_t t = time (0);
  struct tm tstruct = *localtime (&t);
  char tstr[256] = "??Unknown time??: ";
  strftime (tstr, sizeof (tstr), "%Y-%m-%d %H:%M", &tstruct);

  printf ("%s [%d]: ", tstr, currentPid);
  va_start (argList, format);
  vprintf (format, argList);
  printf ("\n");
  va_end (argList);
}

/* Open the log file 'logFilename' */
void
rotateLog (const char *logFilename)
{
  struct stat sb;
  stat (logFilename, &sb);
  if (MAX_LOG_SIZE < sb.st_size)
    {
      char logFilenameOld[1024] = {0};
      sprintf (logFilenameOld, "%s.old", logFilename);
      rename (logFilename, logFilenameOld);
    }
}

void
logOpen (const char *name, const char *host)
{
  char logFile[1024] = {0};
  sprintf (logFile, LOG_FILE_TEMPLATE, name, host);
  if (access (logFile, F_OK) == 0)
    rotateLog (logFile);
  logfp = fopen (logFile, "a");
  if (logfp == NULL)
    exit (EXIT_FAILURE);
  setbuf (logfp, NULL);
  logDaemonMessage ("%s Opened log file.", LOG_DELIMITER);
}

/* Close the log file */

void
logClose (void)
{
  logDaemonMessage ("%s Closing log file.", LOG_DELIMITER);
  fclose (logfp);
}
