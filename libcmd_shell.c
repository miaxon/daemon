#define _GNU_SOURCE
#define _XOPEN_SOURCE 500 
#include <stdio.h>
#include <stdlib.h>
#include <ftw.h>
#include <time.h>
#include <acl/libacl.h>
#include <sys/acl.h>
#include "libcmd.h"
#include "libcmd_sys.h"
int
setup_tmp_fd (void)
{
  int fd;
  char path[16] = "/tmp/vfs.XXXXXX";
  fd = mkstemp (path);
  unlink (path);
  return fd;
}

int
shell_internal (const char *command, int *outfd)
{
  if (outfd && ((*outfd = setup_tmp_fd ()) == -1))
    return -5;

  int status;
  pid_t childPid;
  switch (childPid = fork ())
    {
    case -1: /* Error */
      return -2;
    case 0: /* Child */
      close (1);
      if (dup2 (*outfd, 1) != 1)
        {
          close (*outfd);
          exit (80);
        }
      execl ("/bin/sh", "sh", "-c", command, (char *) NULL);
      _exit (127); /* Failed exec */
    default: /* Parent */
      if (waitpid (childPid, &status, 0) == -1)
        return -3;
      else
        return status;
    }
}

char*
read_tmp_fd (int outFd)
{
  off_t len = lseek (outFd, 0, SEEK_END);
  if (len <= 0)
    return NULL;
  char *buf = (char*) malloc (len + 1);
  if (buf == NULL)
    return NULL;
  lseek (outFd, 0, SEEK_SET);
  memset (buf, 0, len + 1);
  read (outFd, buf, len);
  return buf;
}

void
_Shell (json_t *request, json_t *response)
{
 LOG("_Shell():\n");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *cmd = json_string_value (json_array_get (params_object, 0));
  LOG(cmd);
  int ret, outFd;
  ret = shell_internal (cmd, &outFd);
  json_t *o = get_ret_object ();
  if (ret < 0 || ret == 127 || ret == 80)
    goto exit;
  char *buf = read_tmp_fd (outFd);
  close (outFd);
  LOG("%s (%d)\n", strerror (errno), errno);
  json_t *out_str;
  if (buf != NULL)
    {
      out_str = json_string (buf);
      free (buf);
    }
  else
    {
      out_str = json_string (DEFSTR);
    }
  json_t *out_int = json_integer (ret);
  json_object_set_new (o, RETSTR, out_str);
  json_object_set_new (o, RETINT, out_int);
exit:
  make_responce (response, ret, strerror (errno), o);
}

void
shell_cmd (json_t *response, char *cmd)
{
  LOG("shell_cmd():");
  LOG(cmd);
  int ret, outFd;
  json_t *o = get_ret_object ();
  if (cmd == NULL)
    {
      ret = -1;
      errno = EINVAL;
      goto exit;
    }
  ret = shell_internal (cmd, &outFd);
  if (ret < 0 || ret == 127 || ret == 80)
    goto exit;
  char *buf = read_tmp_fd (outFd);
  close (outFd);
  LOG("%s (%d)", strerror (errno), errno);
  json_t *out_str;
  if (buf != NULL)
    {
      out_str = json_string (buf);
      free (buf);
    }
  else
    {
      out_str = json_string (DEFSTR);
    }
  json_t *out_int = json_integer (ret);
  json_object_set_new (o, RETSTR, out_str);
  json_object_set_new (o, RETINT, out_int);
exit:
  make_responce (response, ret, strerror (errno), o);
}

void
_UserAdd (json_t *request, json_t *response)
{
  LOG("_UserAdd():");
  char cmd[256] = "/monolit/dms/backend/sh/adduser.sh";
  json_t *params_object = json_object_get (request, PARAMS);
  const char *name = json_string_value (json_array_get (params_object, 0));
  const char *pwd = json_string_value (json_array_get (params_object, 1));
  char *buf;
  asprintf (&buf, "%s %s %s", cmd, name, pwd);
  LOG(buf);
  shell_cmd (response, buf);
  free (buf);
}
void
_FindByInum (json_t *request, json_t *response)
{
  LOG("_UtilFindByInum():");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  const char *inum = json_string_value (json_array_get (params_object, 1));
  char *cmd;
  asprintf (&cmd, "find '%s' -path '%s/.*' -prune -o -inum %s -print", path, path, inum);
  LOG(cmd);
  shell_cmd(response, cmd);
  free(cmd);
}
void
_GetInUseInfo (json_t *request, json_t *response)
{
  LOG("_GetInUseInfo:");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *name = json_string_value (json_array_get (params_object, 0));
  char *cmd;
  asprintf (&cmd, "/monolit/dms/backend/pl/status.pl '%s'", name);
  LOG(cmd);
  shell_cmd(response, cmd);
  free(cmd);
}