#include "dmsd.h"
#include "logging.h"
#include <signal.h>
#include <sys/resource.h>
#include "libcmd.h"
struct sigaction sigact;
static char* sig_names[] = {"SIGHUP", "SIGINT", "SIGQUIT", "SIGILL", "SIGTRAP", "SIGABRT", "SIGBUS", "SIGFPE", "SIGKILL", "SIGUSR1", "SIGSEGV", "SIGUSR2", "SIGPIPE", "SIGALRM", "SIGTERM"};

void
panic (char *msg)
{
  LOGD ("%s", msg);
  char email[1024 ] = {0};
  sprintf (email, "/etc/samba/dmspanic '%s'", msg);
  system (email);
}

static void
dumpstack (void)
{
  LOGD ("dumping stack ...");
  struct rlimit core_limits;
  core_limits.rlim_cur = core_limits.rlim_max = RLIM_INFINITY;
  setrlimit (RLIMIT_CORE, &core_limits);
  char cmd[160] = {0};
  sprintf (cmd, "/etc/samba/dmsdump %d", getpid ());
  system (cmd);
  return;
}

void
cleanup ()
{
  sigemptyset (&sigact.sa_mask);
  conf_param_t *cfp = get_cfp ();
  cleanupHandler (cfp);
}

void
CommonSigHandler (int sig)
{
  if (sig <= 15)
    LOGD ("CommonSigHandler: signal %s recieved. Exit.", sig_names[sig - 1]);
  else
    LOGD ("CommonSigHandler: signal %d recieved. Exit.", sig);
  pid_t pid = getpid ();
  char user[16] = {0};
  sprintf (user, "%s", userNameFromId (currentUid));
  if (sig == SIGHUP)
    {
      char msg[1024] = {0};
      sprintf (msg, "FATAL: Program hanged up pid %d, user %s\n", pid, user);
      panic (msg);
    }
  if (sig == SIGSEGV || sig == SIGBUS)
    {
      dumpstack ();
      char msg[1024] = {0};
      sprintf (msg, "FATAL: pid %d, user %s - %s Fault. Logged StackTrace in /var/log/dmsd/core/core.%d\n", pid, user, (sig == SIGSEGV) ? "Segmentation" : ((sig == SIGBUS) ? "Bus" : "Unknown"), pid);
      panic (msg);
    }
  //if (sig == SIGQUIT) panic ("QUIT signal ended program pid %d, user %s\n", pid, user);
  //if (sig == SIGKILL) panic ("KILL signal ended program pid %d, user %s\n", pid, user);
  //if (sig == SIGINT) panic ("INT signal ended program pid %d, user %s\n", pid, user);
  cleanup ();
  exit (sig);
}

BOOL
configureSignalHandlers ()
{  
  sigact.sa_handler = CommonSigHandler;
  sigemptyset (&sigact.sa_mask);
  sigact.sa_flags = 0;
  
  sigaddset (&sigact.sa_mask, SIGINT);
  sigaction (SIGINT, &sigact, (struct sigaction *) NULL);

  sigaddset (&sigact.sa_mask, SIGSEGV);
  sigaction (SIGSEGV, &sigact, (struct sigaction *) NULL);

  sigaddset (&sigact.sa_mask, SIGBUS);
  sigaction (SIGBUS, &sigact, (struct sigaction *) NULL);

  sigaddset (&sigact.sa_mask, SIGQUIT);
  sigaction (SIGQUIT, &sigact, (struct sigaction *) NULL);

  sigaddset (&sigact.sa_mask, SIGHUP);
  sigaction (SIGHUP, &sigact, (struct sigaction *) NULL);

  sigaddset (&sigact.sa_mask, SIGKILL);
  sigaction (SIGKILL, &sigact, (struct sigaction *) NULL);

  sigaddset (&sigact.sa_mask, SIGUSR1);
  sigaction (SIGUSR1, &sigact, (struct sigaction *) NULL);
  
  sigaddset (&sigact.sa_mask, SIGTERM);
  sigaction (SIGTERM, &sigact, (struct sigaction *) NULL);

}
