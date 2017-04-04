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
#include "dms.h"
#include "libcmd_sys.h"
#include <ftw.h>
#include <unistd.h>

void
_UtilMakeTmplate (json_t *request, json_t *response)
{
  LOG ("_UtilMakeTmplate():");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  const char *templatepath = json_string_value (json_array_get (params_object, 1));
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    {
      const char *name = path_name (path);
      LOG ("%s, %s\n", path, templatepath);
      int i = 1;
      char *newpath;
      asprintf (&newpath, "%s/%s", templatepath, name);
      while (OK (newpath) == 0)
        {
          frees (newpath);
          asprintf (&newpath, "%s/%s %d", templatepath, name, i);
          i++;
        }
      ret = cpax (path, newpath);
      if (ret == 0)
        {
          ret = clear_dir (newpath);
          struct xattr xa;
          get_xattr (newpath, &xa);
          xflag_unset (&xa, A_FLAG_CLOSE);
          xflag_set (&xa, A_FLAG_PROJECT);
          memset (xa.x_name, 0, A_NAME_MAX);
          strncpy (xa.x_name, path_name (newpath), A_NAME_MAX);
          set_xattr (newpath, &xa);
        }
      frees (newpath);
    }
  make_responce (response, ret, strerror (errno), o);

}

void
_UtilDirEmpty (json_t *request, json_t *response)
{
  LOG ("_UtilDirEmpty():");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    {
      ret = IsDirectoryEmpty (path);
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_UtilDuplicateFSO (json_t *request, json_t *response)
{
  LOG ("_UtilDuplicateFSO():");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  const char *newpath = json_string_value (json_array_get (params_object, 1));
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    {
      LOG ("%s, %s\n", path, newpath);
      ret = cpax (path, newpath);
      xname_set (newpath, NULL);
      logmsg (newpath, Copy, "");
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_UtilClose (json_t *request, json_t *response)
{
  LOG ("_UtilClose():");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    {
      struct xattr xa;
      get_xattr (path, &xa);
      xflag_set (&xa, A_FLAG_CLOSE);
      set_xattr (path, &xa);
      if (IsRegularFile (path) == 0)
        {
          char *cmd;
          asprintf (&cmd, "chmod 0551 '%s'", path);
          ret = system (cmd);
          free (cmd);
          asprintf (&cmd, "setfacl -m g:archive_users:r-x '%s'", path);
          ret = system (cmd);
          free (cmd);
        }
      if (IsDirectory (path) == 0)
        {
          ret = close_dir (path);
        }
      if (ret == 0)
        logmsg (path, Closed, "");
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_UtilUnClose (json_t *request, json_t *response)
{
  LOG ("_UtilUnClose():");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    {
      struct xattr xa;
      get_xattr (path, &xa);
      xflag_unset (&xa, A_FLAG_CLOSE);
      set_xattr (path, &xa);
      set_xattr (path, &xa);
      if (IsRegularFile (path) == 0)
        {
          char *cmd;
          asprintf (&cmd, "chmod 0755 '%s'", path);
          ret = system (cmd);
          free (cmd);
        }
      if (IsDirectory (path) == 0)
        {
          ret = unclose_dir (path);
        }
      if (ret == 0)
        logmsg (path, UnClosed, "");
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_UtilCacheXattr (json_t *request, json_t *response)
{
  LOG ("_UtilCacheXattr():");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    {
      //logmsg (path, Opened, "");
      cache_xattr (path);
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_UtilGetThumb (json_t *request, json_t *response)
{
  LOG ("_UtilGetThumb():");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  json_t *o = get_ret_object ();
  LOG ("%s", path);
  int ret = OK (path);
  if (ret == 0)
    {      
      char *path_thumb, *path_get;
      asprintf(&path_thumb, "%s.jpg", path);
      char *cmd;
      asprintf (&cmd, "convert -density 300 -resize 640 '%s[0]' '%s'", path, path_thumb);
      LOG(cmd);
      ret = system (cmd);
      free (cmd);
      if(ret == 0)
        {
          path_get = get_fso (FSOP_Read, path_thumb);
          remove(path_thumb);
        }
      else
        path_get = get_fso (FSOP_Read, path);
      json_object_set_new (o, RETSTR, json_string (path_get));
      frees (path_get);
      frees(path_thumb);
    }
  make_responce (response, ret, strerror (errno), o);
}