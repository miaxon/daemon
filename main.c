#define _GNU_SOURCE
#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <locale.h>
#include <libconfig.h>
#include <signal.h>
#include "logging.h"
#include "become_daemon.h"
#include "ugid_functions.h"
#include "tlpi_hdr.h"
#include "dms.h"
#include "dmsd.h"
#include "libcmd.h"

BOOL
become_user (const char* name)
{
  if (name == NULL)
    return seteuid (currentUid) == 0;
  currentUid = userIdFromName (name);
  LOGD ("become user %s, (%d)", currentUser, currentUid);
  return seteuid (currentUid) == 0;
}

BOOL
unbecome_user ()
{
  LOGD ("unbecome user %s, (%d)", currentUser, currentUid);
  return seteuid (0) == 0;
}

BOOL
save_pid (const char* pidFile)
{
  int fd, len;
  pid_t pid = currentPid;
  char pid_buf[16] = {0};
  sprintf (pid_buf, "%d", pid);
  if (OK (pidFile) == 0)
    unlink (pidFile);
  if ((fd = open (pidFile, O_CREAT | O_WRONLY, 0600)) < 0)
    {
      LOGD ("don't open lock file '%s'.", pidFile);
      return FALSE;
    }
  len = write (fd, pid_buf, strlen (pid_buf));
  close (fd);
  chown (pidFile, 0, 0);
  LOGD ("saving pid file '%s' [%s].", pidFile, pid_buf);
  return len > 0;
}

pid_t
get_saved_pid (const char* pidFile)
{
  int fd, len;
  pid_t pid;
  char pid_buf[16] = {0};
  if ((fd = open (pidFile, O_RDONLY)) < 0)
    {
      LOGD ("Lock file '%s' not found.", pidFile);
      return FALSE;
    }
  len = read (fd, pid_buf, 16);
  close (fd);
  pid_buf[len] = 0;
  pid = atoi (pid_buf);
  LOGD ("saved pid : %s", pid_buf);
  return pid;
}

BOOL
stop (conf_param_t *cfp)
{
  LOGD ("stopping by paramenter '-s' '%s'.", cfp->pipePortLocal);
  pid_t pid = get_saved_pid (cfp->pidFile);
  LOGD ("trying kill another instance '%s' [%d].", cfp->pipePortLocal, pid);
  int ret = 0;
  if (pid && kill (pid, 0) == 0)
    ret = kill (pid, SIGUSR1);
  else
    LOGD ("No process found with pid %d", pid);
  logExit ("Exit after killing", ret);
}

BOOL
check_instance (conf_param_t *cfp)
{
  LOGD ("check_instance '%s'.", cfp->pipePortLocal);
  struct stat sb;
  if (stat (cfp->pipePortLocal, &sb) == 0)
    {
      pid_t pid = get_saved_pid (cfp->pidFile);
      LOGD ("trying kill another instance '%s' [%d].", cfp->pipePortLocal, pid);
      if (pid)
        kill (pid, SIGUSR1);
    }
  return TRUE;
}

BOOL
parse_args (int argc, char *argv[], struct start_param *stp)
{
  bDebug = FALSE;
  strncpy (stp->confFile, CONFIG_FILE, 1024);
  memset (stp->hostName, 0, 32);
  memset (stp->userName, 0, 32);
  const char* short_options = "u:h:c:ds";
  const struct option long_options[] = {
    {"user", required_argument, NULL, 'u'},
    {"host", required_argument, NULL, 'h'},
    {"config", optional_argument, NULL, 'c'},
    {"debug", no_argument, NULL, 'd'},
    {"stop", no_argument, NULL, 's'},
    {NULL, 0, NULL, 0}
  };
  int opt;
  int option_index;
  opt = getopt_long (argc, argv, short_options, long_options, &option_index);
  while (opt != -1)
    {
      //printf ("opt: '%c' %d %s\n", opt, opt, optarg);
      switch (opt)
        {
        case 'u':
          strncpy (stp->userName, optarg, 32);
          break;
        case 'h':
          strncpy (stp->hostName, optarg, 32);
          break;
        case 'c':
          strncpy (stp->confFile, optarg, 1024);
        case 'd':
          bDebug = TRUE;
          break;
        case 's':
          bStop = TRUE;
          break;
        default:
          break;
        }
      opt = getopt_long (argc, argv, short_options, long_options, &option_index);
    }
  if (strlen (stp->userName) == 0 || strlen (stp->hostName) == 0)
    return FALSE;
  if (strstr (stp->userName, stp->hostName) != NULL)
    {
      return FALSE;
    }
  logOpen (stp->userName, stp->hostName);
  logStart (stp);
  return TRUE;
}

int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");
  start_param_t stp;
  if (!parse_args (argc, argv, &stp))
    {
      printf ("Invalid cmd params.\nUsage %s -u|--user <username> -h|--host <hostname> [-c|--config configFileName] [-d]\n", argv[0]);
      exit (EXIT_FAILURE);
    }

  currentUid = geteuid ();
  currentPid = getpid ();
  strncpy (currentUser, stp.userName, 32);
  conf_param_t cfp;
  load_conf (stp.confFile, &cfp, &stp);
  if (bStop)
    stop (&cfp);
  if (!check_instance (&cfp))
    logExit ("another instance still running!", EXIT_FAILURE);
  int ret = 0;
  if (!bDebug)
    ret = BecomeDaemon (BD_NO_CLOSE_FILES);
  if (ret < 0)
    logExit ("Failed BecomeDaemon procedure!", EXIT_FAILURE);
  configureSignalHandlers ();
  save_pid (cfp.pidFile);
  if (!become_user (stp.userName))
    logExit ("don't become user!", EXIT_FAILURE);
  init_pipe_local (&cfp);
  //waiting for exit ......
  logExit ("normally shutdown", EXIT_SUCCESS);
} 