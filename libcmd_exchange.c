#define _GNU_SOURCE
#define _XOPEN_SOURCE 500 
#include <stdio.h>
#include <stdlib.h>
#include <acl/libacl.h>
#include <sys/acl.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include "libcmd.h"
#include "dms.h"
#include "ugid_functions.h"
#include "libcmd_sys.h"
#define USER_ARCHIVE_COPY_DIR "/monolit/dms/users/%s/docs/Архивные копии"
#define USER_ARCHIVE_COPY_PROJ "/monolit/dms/users/%s/docs/Архивные копии/%s"
#define USER_ARCHIVE_COPY_FILE "/monolit/dms/users/%s/docs/Архивные копии/%s/%s"
#define ARCHIVE_USAGE_LOG "/monolit.back/storage/log/archive_usage.log"
#define ARCHIVE_USAGE_LOG_SIZE 1024*1024

void
_PutArchiveCopyToUser (json_t *request, json_t *response)
{
  LOG ("_PutArchiveCopyToUser():");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  const char *userName = json_string_value (json_array_get (params_object, 1));
  const char *projName = json_string_value (json_array_get (params_object, 2));
  LOG ("%s -> %s", path, userName);
  json_t *o = get_ret_object ();
  int ret = OK (path);
  char *msg = NULL;
  if (ret == 0)
    {
      const char *cpName = path_name (path);
      char *cpPath;
      asprintf (&cpPath, USER_ARCHIVE_COPY_DIR, userName);
      if (OK (cpPath) != 0)
        {
          ret = mkdir (cpPath, 0700);
          if (ret != 0)
            {
              free (cpPath);
              goto exit;
            }
          ret = chownR (cpPath, userName, NULL);
          if (ret != 0)
            {
              free (cpPath);
              goto exit;
            }
        }
      asprintf (&cpPath, USER_ARCHIVE_COPY_PROJ, userName, projName);
      if (OK (cpPath) != 0)
        {
          ret = mkdir (cpPath, 0700);
          if (ret != 0)
            {
              free (cpPath);
              goto exit;
            }
          ret = chownR (cpPath, userName, NULL);
          if (ret != 0)
            {
              free (cpPath);
              goto exit;
            }
        }
      asprintf (&cpPath, USER_ARCHIVE_COPY_FILE, userName, projName, cpName);
      if (OK (cpPath) == 0)
        {
          ret = rmrf (cpPath);
          if (ret != 0)
            {
              free (cpPath);
              goto exit;
            }
        }
      ret = cpax (path, cpPath);
      if (ret != 0)
        {
          free (cpPath);
          goto exit;
        }
      ret = chownR (cpPath, userName, NULL);
      if (ret != 0)
        {
          free (cpPath);
          goto exit;
        }
      ret = chmodR (cpPath, 0700);
      free (cpPath);
      logmsg (path, PutArchiveCopyToUser, userName);
    }
exit:
  asprintf (&msg, "%s -> %s (%d)", path, userName, ret);
  log_echo (ARCHIVE_USAGE_LOG, msg, ARCHIVE_USAGE_LOG_SIZE);
  frees (msg);
  make_responce (response, ret, strerror (errno), o);
}
