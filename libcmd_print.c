#define _GNU_SOURCE
#define _XOPEN_SOURCE 500 
#include <acl/libacl.h>
#include <sys/acl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <locale.h>
#include <dlfcn.h>
#include "libcmd.h"
#include "libcmd_sys.h"
#include "dms.h"
#include <time.h>
#define PRINTFILE "/monolit.back/storage/print/%ld"
#define PRINTFILE_S "/monolit.back/storage/print/%s"

#define PRINTADMINS "lpadmins"
#define DEFAULT_PRINTATTR "{\"printer\":1,\"format\":\"A4\",\"pages\":\"\",\"copies\":1,\"uid\":\"%d\",\"state\":0,\"urgent\":0,\"foul\":0,\"date\":\"%s\",\"comment\":\"\"}"
void print_bak(const char *path)
{
  char *cmd;
  asprintf (&cmd, "/monolit/dms/backend/pl/print_bak.pl %s '%s'", userNameFromId (currentUid), path);
  LOG("print_bak(%s)", cmd);
  system (cmd);
  free (cmd);
}
void
_PrintPutFSO (json_t *request, json_t * response)
{
  LOG("_PrintPutFSO():");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  json_t *o = get_ret_object ();
  int ret;
  LOG(path);
  ret = OK (path);
  if (ret == 0)
    {
      char *ppath;
      asprintf (&ppath, PRINTFILE, GetInode (path));
      //cpax (path, ppath);
      //ret = chmod (ppath, get_mode ("0555"));
      if (IsLink (ppath) != 0)
        {
          unlink (ppath);
          LOG ("unlink: %s, %s", ppath, strerror (errno));
          ret = symlink(path, ppath);
        }      
      xname_set(ppath, path_name(path));
      free (ppath);
      print_bak(path);
      logmsg(path, Printed, "");
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_PrintPut (json_t *request, json_t * response)
{
  LOG("_PrintPut():");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *tpath = json_string_value (json_array_get (params_object, 0));
  json_t *o = get_ret_object ();
  int ret;
  LOG(tpath);
  ret = OK (tpath);
  if (ret == 0)
    {
      char *ppath;
      unsigned long int id = GetInode (tpath);
      asprintf (&ppath, PRINTFILE, id + time (0));
      while(OK(ppath) == 0)
        {
          free(ppath);
          id = id + time (0);
          asprintf (&ppath, PRINTFILE, id);
        }
      
      cpax (tpath, ppath);
      ret = chmod (ppath, 0555);
      xname_set(ppath, path_name(tpath));
      time_t t = time (0);
      struct tm tstruct = *localtime (&t);
      char tstr[256 ] = {0};
      strftime (tstr, sizeof (tstr), "%d.%m.%Y %H:%M", &tstruct);
      char *pattr;
      asprintf (&pattr, DEFAULT_PRINTATTR, currentUid, tstr);
      ret = setxattr (ppath, A_PRINTJOB, pattr, strlen (pattr), 0);
      free (ppath);
      print_bak(tpath);
    }
  make_responce (response, ret, strerror (errno), o);
}
