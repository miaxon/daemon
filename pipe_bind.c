#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <pwd.h>
#include <locale.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <dlfcn.h>
#include <libsmbclient.h>
#include "dmsd.h"
#include "logging.h"
#include "libcmd.h"


char request[BUF_REQ_SIZE] = {0};
extern int
smbc_close (int fd);
extern int
smbc_open (const char *furl, int flags, mode_t mode);
extern ssize_t
smbc_write (int fd, const void *buf, size_t bufsize);
extern int
smbc_init (smbc_get_auth_data_fn fn, int debug);
extern SMBCCTX *
smbc_init_context (SMBCCTX *context);
extern SMBCCTX *
smbc_set_context (SMBCCTX *context);
extern int
smbc_getDebug (SMBCCTX *c);
extern void
smbc_setUser (SMBCCTX *c, char * user);
extern void
smbc_setWorkgroup (SMBCCTX *c, char * workgroup);
extern void
smbc_setOptionBrowseMaxLmbCount (SMBCCTX *c, int count);
extern void
smbc_setOptionOneSharePerServer (SMBCCTX *c, smbc_bool b);
extern int
smbc_free_context (SMBCCTX *c, int shutdown_ctx);
extern SMBCCTX *
smbc_new_context (void);

SMBCCTX *smbcCtx = NULL;
int ipcFd = 0;

cred_struct_t crd_mem = {
  {0},
  {0},
  {0}, 0
};

struct conf_param *
get_cfp ()
{
  return cfp_mem;
}

cred_struct_t *
get_crd ()
{
  LOGD ("crd_mem.init %d", crd_mem.init);
  if (crd_mem.init)
    return &crd_mem;
  conf_param_t *cfp = get_cfp ();
#define DCHAR '\n'
  uid_t cuid = geteuid ();
  seteuid (0);
  int fd;
  fd = open (cfp->pipeAuthCreds, O_RDONLY);
  if (fd == -1)
    return NULL;
  char buf[256] = {0};
  int ret = read (fd, buf, 256);
  if (ret <= 0)
    return NULL;
  char* tok;
  char *ptr = buf;

  tok = strchr (ptr, DCHAR);
  strncpy (crd_mem.workgroup, ptr, tok - ptr);
  ptr = tok + 1;

  tok = strchr (ptr, DCHAR);
  strncpy (crd_mem.username, ptr, tok - ptr);
  ptr = tok + 1;

  tok = strchr (ptr, DCHAR);
  strncpy (crd_mem.password, ptr, tok - ptr);

  close (fd);
  seteuid (cuid);
  crd_mem.init = 1;
  return &crd_mem;
}

static void
get_auth_data_fn (const char * pServer,
                  const char * pShare,
                  char * pWorkgroup,
                  int maxLenWorkgroup,
                  char * pUsername,
                  int maxLenUsername,
                  char * pPassword,
                  int maxLenPassword)
{
  cred_struct_t *cr = get_crd ();
  if (cr != NULL)
    {
      LOG ("get_credentials: server - %s, share - %s, %s %s", pServer, pShare, pUsername, pWorkgroup);
      //strncpy (pWorkgroup, cr->workgroup, maxLenWorkgroup - 1);
      //strncpy (pUsername, cr->username, maxLenUsername - 1);
      strncpy (pPassword, cr->password, maxLenPassword - 1);
    }
  else
    {
      LOG ("get_credentials failed!");
    }
}

int
flush_fifo (char *name)
{
  unbecome_user ();
  LOGD ("flushing fifo %s.", name);
  if (access (name, F_OK) == 0)
    remove (name);
  if (mkfifo (name, S_IRUSR | S_IWUSR) == -1 && errno != EEXIST)
    logExit ("open pipe local port failed", errno);
  chown (name, currentUid, 0);

  return 0;
}

void /* Free memory pointed to by 'arg' and unlock mutex */
cleanupHandler (void *arg)
{
  unbecome_user ();
  if (ipcFd > 0)
    smbc_close (ipcFd);
  if (smbcCtx != NULL)
    smbc_free_context (smbcCtx, 1);
  conf_param_t *cfp = (conf_param_t*) arg;
  LOGD ("fifo service cleanupHandler");
  int s;
  s = remove (cfp->pipePortLocal);
  //sem_t *sem = sem_open (cfp->pipeLockSem, 0);
  //if (sem != SEM_FAILED)
  //  sem_close (sem);
  //sem_unlink (cfp->pipeLockSem);
  remove (cfp->pidFile);
}

int
smbclient_init ()
{
  int ret = -1;
  ret = smbc_init (get_auth_data_fn, 0);
  if (ret < 0)
    return ret;
  smbcCtx = smbc_new_context ();
  smbc_setDebug (smbcCtx, 0);
  smbc_setFunctionAuthData (smbcCtx, get_auth_data_fn);
  smbc_setOptionBrowseMaxLmbCount (smbcCtx, 0);
  smbc_setOptionOneSharePerServer (smbcCtx, 1);
  cred_struct_t *cr = get_crd ();
  smbc_setUser (smbcCtx, cr->username);
  smbc_setWorkgroup (smbcCtx, cr->workgroup);
  if (smbc_init_context (smbcCtx))
    {
      smbc_set_context (smbcCtx);
      return 1;
    }
  else
    {
      smbc_free_context (smbcCtx, 1);
      smbcCtx = NULL;
      return -1;
    }
}

void
init_pipe_local (struct conf_param *cfp)
{
  //sem_t *sem = sem_open (cfp->pipeLockSem, O_RDWR|O_CREAT, 0766, 1);
  //if (sem == SEM_FAILED)
  //  {
  //    LOGD ("===== write_pipe sem_open failed. sem_name '%s', errno '%d'", cfp->pipeLockSem, errno);
  //    logExit ("sem_open", errno);
  //  }
  if (smbclient_init () < 0)
    logExit ("Initializing the smbclient library failed.", errno);
  LOGD ("Initializing the smbclient library OK.");
  flush_fifo (cfp->pipePortLocal);
  //sem_t *semc = sem_open (cfp->pipeLockSem, 0);
  //if (semc != SEM_FAILED)
 //   sem_post (semc);
  pthread_cleanup_push (cleanupHandler, (void*) cfp);
  LOGD ("fifo service: start listening.");
  //ipcFd = smbc_open(cfp->pipeIpcRemote, O_WRONLY | O_CREAT, 0700);
  //LOG("connect to remote %s: %d %s",cfp->pipeIpcRemote, ipcFd, strerror(errno));
  become_user (currentUser);
  for (;;)
    {
      //sleep (10);
      //break;
      int rlen = read_fifo (cfp->pipePortLocal);
      if (rlen < 0)
        {
          LOGD ("Stopping by stop_signal 0x13...");
          break; //stopping
        }
      if (rlen == 0)
        continue; //too small data
      char *response = jsonrpc_process (request, &rlen);
      if (rlen <= 0)
        {
          LOGD ("rlen <= 0");
          continue;
        }//too small data
      if (!write_pipe (cfp->pipePortRemote, cfp->pipeLockSem, response, rlen))
        LOGD ("write_pipe failed: %s, %s, %s, %d",
              cfp->pipePortRemote,
              cfp->pipeLockSem,
              strerror (errno),
              errno);
      free (response);
    }
  LOGD ("fifo servise: exit listening.");
  pthread_cleanup_pop (1);
}

BOOL
need_stop (char *buf)
{

  int i = 0;
  for (i = 0; i < 10; i++)
    {
      if (buf[i] != 0x13)
        break;
    }
  return i == 10;
}

int
read_fifo (const char* name)
{
  //LOG ("reading fifo %s.", name);
  int rlen = 0;
  int fifoFd = open (name, O_RDONLY);
  if (fifoFd != -1)
    {
      memset (request, 0, BUF_REQ_SIZE);
      char* bufPtr = request;
      while (1)
        {
          int cbRead = 0;
          cbRead = read (fifoFd, bufPtr, PIPE_BUFSZE);
          if (cbRead == 0 && errno != EAGAIN)
            {
              break;
            }
          bufPtr += cbRead;
          rlen += cbRead;
        }
      close (fifoFd);
    }
  LOG ("====================   READ IN %d bytes  =====================", rlen);
  //LOG ("bytes read: %d", rlen);
  if (need_stop (request))
    {
      LOG ("need_stop");
      return -1;
    }
  if (rlen < 32)
    {
      LOG ("request too small.");
      return 0;
    }
  return rlen;
}

BOOL
write_pipe (const char* name, const char* sem_name, char *data, int data_len)
{
  //LOG ("\t===== attempting to write pipe");
  //sem_t *sem = sem_open (sem_name, 0);
  //if (sem == SEM_FAILED)
  //  {
  //    LOGD ("===== write_pipe sem_open failed. sem_name '%s', errno '%d'", sem_name, errno);
  //    logExit ("\t===== write_pipe sem_open failed", errno);
  //  }
  int len = 0;
  //if (sem != SEM_FAILED && sem_wait (sem) != -1)
  //  {
      // LOG ("\t===== sem wait ok: %s", sem_name);
      int waitc = 1;
      int pipeFd = smbc_open (name, O_WRONLY, 0);
      if (pipeFd < 0)
        LOGD ("\t===== smbc_open: %d %d %s", pipeFd, errno, strerror (errno));
      while (waitc < 15 && pipeFd <= 0)
        {
          LOGD ("\t===== waiting pipe %d sec.\n", waitc);
          pipeFd = smbc_open (name, O_WRONLY, 0);
          sleep (1);
          waitc++;
        }
      if (!pipeFd)
        return FALSE;
      char* ptr = data;

      while (len < data_len)
        {
          int sz = data_len - len;
          int smbc_ret = smbc_write (pipeFd, ptr, sz > PIPE_BUFSZE ? PIPE_BUFSZE : sz);
          if (smbc_ret < 0)
            break;
          ptr += smbc_ret;
          len += smbc_ret;
        }
      //LOG ("writing responce (%d) %d\n", data_len, len);
      LOG ("====================   WRITE OUT %d bytes (%d)  =====================", len, data_len);
      smbc_close (pipeFd);
  //  }
 // sem_post (sem);
 // sem_close (sem);
  return len == data_len;
}