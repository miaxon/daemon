#include "tlpi_hdr.h"
#include <libconfig.h>
#include <string.h>
#include "dmsd.h"
#include "logging.h"
#include "libcmd.h"
#define CF_PIPE_PORT_LOCAL "PIPE_PORT_LOCAL"
#define CF_PIPE_PORT_REMOTE "PIPE_PORT_REMOTE"
#define CF_PIPE_IPC_REMOTE "PIPE_IPC_REMOTE"
#define CF_PIPE_PORT_EVENT "PIPE_PORT_EVENT"
#define CF_PIPE_LOCK_SEM "PIPE_LOCK_SEM"
#define CF_PIPE_AUTH_CRED "PIPE_AUTH_CRED"
#define CF_PIPE_DEBUG_USER "PIPE_DEBUG_USER"
#define CF_PID_FILE "PID_FILE"
#define CF_TDIR "TDIR"
#define CF_TFILE "TFILE"
#define CF_TPATH "TPATH"

void
print_conf (struct conf_param *cfp)
{
  LOGD ("Config options:\n%s = '%s'\n%s = '%s'\n%s = '%s'\n%s = '%s'\n%s = '%s'\n%s = '%s'\n%s = '%s'\n%s = '%s'\n%s = '%s'\n%s = '%s'\n",
        CF_PIPE_PORT_LOCAL, cfp->pipePortLocal,
        CF_PIPE_PORT_REMOTE, cfp->pipePortRemote,
        CF_PIPE_IPC_REMOTE, cfp->pipeIpcRemote,
        CF_PIPE_PORT_EVENT, cfp->pipePortEvent,
        CF_PIPE_AUTH_CRED, cfp->pipeAuthCreds,
        CF_PIPE_LOCK_SEM, cfp->pipeLockSem,
        CF_PID_FILE, cfp->pidFile,
        CF_TDIR, cfp->tdir,
        CF_TFILE, cfp->tfile,
        CF_TPATH, cfp->tpath);
}

BOOL
load_conf (const char *confFile, struct conf_param *cfp, struct start_param *sp)
{
  config_t cfg;
  const char *str;
  config_init (&cfg);

  /* Read the file. If there is an error, report it and exit. */
  if (!config_read_file (&cfg, confFile))
    {
      char buf[1024] = {0};
      sprintf (buf, "%s:%d - %s\n", confFile, config_error_line (&cfg), config_error_text (&cfg));
      config_destroy (&cfg);
      logExit (buf, EXIT_FAILURE);
    }
  if (config_lookup_string (&cfg, CF_PIPE_PORT_LOCAL, &str))
    {
      sprintf (cfp->pipePortLocal, str, sp->userName, sp->hostName);
    }
  else
    logExit ("No 'PIPE_PORT_LOCAL' setting in configuration file.\n", EXIT_FAILURE);
  
  if (config_lookup_string (&cfg, CF_PID_FILE, &str))
    {
      sprintf (cfp->pidFile, str, sp->userName, sp->hostName);
    }
  else
    logExit ("No 'PID_FILE' setting in configuration file.\n", EXIT_FAILURE);
  
  if (config_lookup_string (&cfg, CF_PIPE_PORT_REMOTE, &str))
    sprintf (cfp->pipePortRemote, str, sp->hostName, sp->userName);
  else
    logExit ("No 'PIPE_PORT_REMOTE' setting in configuration file.\n", EXIT_FAILURE);
  
 if (config_lookup_string (&cfg, CF_PIPE_IPC_REMOTE, &str))
    sprintf (cfp->pipeIpcRemote, str, sp->hostName);
  else
    logExit ("No 'PIPE_IPC_REMOTE' setting in configuration file.\n", EXIT_FAILURE);
  
  if (config_lookup_string (&cfg, CF_PIPE_PORT_EVENT, &str))
    sprintf (cfp->pipePortEvent, str, sp->hostName, sp->userName);
  else
    logExit ("No 'PIPE_PORT_EVENT' setting in configuration file.\n", EXIT_FAILURE);

  if (config_lookup_string (&cfg, CF_PIPE_AUTH_CRED, &str))
    sprintf (cfp->pipeAuthCreds, str, sp->userName, sp->hostName);
  else
    logExit ("No 'PIPE_AUTH_CRED' setting in configuration file.\n", EXIT_FAILURE);

  if (config_lookup_string (&cfg, CF_PIPE_LOCK_SEM, &str))
    sprintf (cfp->pipeLockSem, str, sp->userName, sp->hostName);
  else
    logExit ("No 'PIPE_AUTH_CRED' setting in configuration file.\n", EXIT_FAILURE);

  if (config_lookup_string (&cfg, CF_TDIR, &str))
    sprintf (cfp->tdir, "%s", str);
  else
    logExit ("No 'TDIR' setting in configuration file.\n", EXIT_FAILURE);

  if (config_lookup_string (&cfg, CF_TFILE, &str))
    sprintf (cfp->tfile, "%s", str);
  else
    logExit ("No 'TFILE' setting in configuration file.\n", EXIT_FAILURE);

  if (config_lookup_string (&cfg, CF_TPATH, &str))
    sprintf (cfp->tpath, "%s", str);
  else
    logExit ("No 'TPATH' setting in configuration file.\n", EXIT_FAILURE);

  if (bDebug)
    {
      sprintf (cfp->pipePortLocal, "%sd", cfp->pipePortLocal);
      sprintf (cfp->pidFile, "%sd", cfp->pidFile);
    }
  config_destroy (&cfg);
  print_conf (cfp);
  cfp_mem = cfp;
  return TRUE;
}
