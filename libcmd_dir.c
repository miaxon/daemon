#define _GNU_SOURCE
#define _XOPEN_SOURCE 500 
#include <stdio.h>
#include <stdlib.h>
#include <ftw.h>
#include <acl/libacl.h>
#include <sys/acl.h>
#include <time.h>
#include <pthread.h>
#include <locale.h>
#include <fcntl.h>
#include <dirent.h>
#include "libcmd.h"
#include "libcmd_sys.h"
#include "dms.h"
#include "ugid_functions.h"

void
_DirMake (json_t *request, json_t *response)
{
  LOG ("_DirMake():");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  const char *vtype = json_string_value (json_array_get (params_object, 1));
  const char *tmpl = json_string_value (json_array_get (params_object, 2));
  LOG (path);
  json_t *o = get_ret_object ();
  char parent[1024] = {0};
  const char *name = path_parent (path, parent);
  int ret = OK (parent);
  if (ret == 0)
    {
      int isBook = strcmp (vtype, A_TYPE_BOOK) == 0;
      int isProj = strcmp (vtype, A_TYPE_PROJECT) == 0;
      struct xattr xa;
      if (isBook == 1)
        {
          ret = mkdir (path, 0755);
          if (ret == 0)
            {
              get_xattr (path, &xa);
              xflag_set (&xa, A_FLAG_BOOK);
              memset (xa.x_name, 0, A_NAME_MAX);
              strncpy (xa.x_name, name, A_NAME_MAX);
              set_xattr (path, &xa);
              chown (path, 0, groupIdFromName ("dmsadmins"));
              char *rpath;
              asprintf (&rpath, "%s/%s", path, ".recycle");
              ret = mkdir (rpath, 0755);
              if (ret == 0)
                {
                  get_xattr (rpath, &xa);
                  xflag_set (&xa, A_FLAG_BASKET);
                  memset (xa.x_name, 0, A_NAME_MAX);
                  strncpy (xa.x_name, "Корзина раздела", A_NAME_MAX);
                  set_xattr (rpath, &xa);
                  chown (rpath, 0, groupIdFromName ("dmsadmins"));
                }
              free (rpath);
            }
        }
      if (isProj == 1 && strlen (tmpl) > 0)
        {
          int i = 1;
          char *newpath;
          asprintf (&newpath, "%s/%s", parent, name);
          while (OK (newpath) == 0)
            {
              frees (newpath);
              asprintf (&newpath, "%s/%s %d", parent, name, i);
              i++;
            }
          name = path_name (newpath);
          if (strlen (tmpl) > 0 && OK (tmpl) == 0)
            {
              ret = cpax (tmpl, newpath);
              if (ret == 0)
                {
                  chown (newpath, currentUid, -1);
                }
              xname_set (newpath, name);
            }
          else
            {
              ret = mkdir (newpath, 0755);
              if (ret == 0)
                {
                  get_xattr (newpath, &xa);
                  xflag_set (&xa, A_FLAG_PROJECT);
                  memset (xa.x_name, 0, A_NAME_MAX);
                  strncpy (xa.x_name, name, A_NAME_MAX);
                  set_xattr (path, &xa);
                  chown (newpath, currentUid, -1);
                  char *rpath;
                  asprintf (&rpath, "%s/%s", newpath, ".recycle");
                  ret = mkdir (rpath, 0755);
                  if (ret == 0)
                    {
                      get_xattr (rpath, &xa);
                      xflag_set (&xa, A_FLAG_BASKET);
                      memset (xa.x_name, 0, A_NAME_MAX);
                      strncpy (xa.x_name, "Корзина проекта", A_NAME_MAX);
                      set_xattr (rpath, &xa);
                      chown (rpath, 0, groupIdFromName ("dmsadmins"));
                    }
                  free (rpath);
                }
            }
          frees (newpath);
        }
      if (isProj != 1 && isBook != 1)
        {
          ret = mkdir (path, 0755);
          if (ret == 0)
            {
              get_xattr (path, &xa);
              xa.x_flags = 0;
              memset (xa.x_name, 0, A_NAME_MAX);
              strncpy (xa.x_name, name, A_NAME_MAX);
              set_xattr (path, &xa);
              inherit_perms (parent, path);
              logmsg (path, Added, "");
            }
        }
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_DirRead (json_t *request, json_t *response)
{
  LOG ("_DirRead():");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  json_int_t mask = json_integer_value (json_array_get (params_object, 1));
  json_t *filter = json_array_get (params_object, 2);
  json_int_t flags = json_integer_value (json_object_get (filter, "flags"));
  LOG (path);
  int ret = OK (path);
  if (ret == 0)
    {
      json_t *o = get_jchild (path, mask, (int) flags);
      make_responce (response, ret, strerror (errno), o);
    }
  else
    {
      json_t *o = get_ret_object ();
      make_responce (response, ret, strerror (errno), o);
    }
}

void
_PutFSOTemp (json_t *request, json_t *response)
{
  LOG ("_PutFSOTemp():");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *tpath = json_string_value (json_array_get (params_object, 0));
  const char *path = json_string_value (json_array_get (params_object, 1));
  json_int_t op = json_integer_value (json_array_get (params_object, 2));
  json_t *o = get_ret_object ();
  int ret = 0;
  LOG ("%s %s %d", tpath, path, (int) op);
  char parent[1024] = {0};
  const char *name = path_parent (path, parent);
  ret = OK (parent);
  if (ret == 0)
    {
      if (name)
        {
          if (OK (path))
            rmrf (path);
          switch ((int) op)
            {
            case FSOP_Link:
              ret = rename (tpath, path);
              break;
            case FSOP_Copy:
              ret = cpax (tpath, path);
              break;
            case FSOP_Move:
              ret = rename (tpath, path);

              break;
            }
          /*if (OK (path) == 0)
            {
              inherit_perms (parent, path);
            }*/
          if (OK (path) == 0 && (IsRegularFile (path) == 0 || (int) op == FSOP_Link))
            {
              inherit_perms (parent, path);
            }
        }
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_MoveFSO (json_t *request, json_t *response)
{
  LOG ("_MoveFSO():");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  const char *newpath = json_string_value (json_array_get (params_object, 1));
  json_t *o = get_ret_object ();
  LOG ("%s, %s\n", path, newpath);
  int ret = OK (path);
  if (ret == 0)
    ret = rename (path, newpath);
  make_responce (response, ret, strerror (errno), o);
}

void
_GetFSOTemp (json_t *request, json_t *response)
{
  LOG ("_GetFSOTemp():");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  json_int_t op = json_integer_value (json_array_get (params_object, 1));
  json_t *o = get_ret_object ();
  LOG ("%s, %d", path, (int) op);
  int ret = OK (path);
  if (ret == 0)
    {
      char *path_get = get_fso (op, path);
      LOG ("%s, %d", path_get, (int) op);
      json_object_set_new (o, RETSTR, json_string (path_get));
      frees (path_get);
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_GetFSO (json_t *request, json_t *response)
{
  LOG ("_GetFSO():");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  LOG (path);
  const char *name = path_name (path);
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    {
      json_t *jret;
      jret = get_jstat (path, name);
      json_object_set_new (o, "stat", jret);
      jret = get_jacl (path, ACL_TYPE_ACCESS);
      json_object_set_new (o, "acl", jret);
      jret = get_jxattr (path);
      json_object_set_new (o, "xattr", jret);
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_ExistsFSO (json_t *request, json_t *response)
{
  LOG ("_ExistsFSO():");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  LOG ("%s\n", path);
  json_t *o = get_ret_object ();
  int ret = OK (path);
  make_responce (response, ret, strerror (errno), o);
}

void
_RemoveFSO (json_t *request, json_t *response)
{
  LOG ("_RemoveFSO():\n");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  const char *rpath = json_string_value (json_array_get (params_object, 1));
  json_t *o = get_ret_object ();
  int ret;
  ret = OK (rpath) | OK (path);
  if (ret == 0)
    {
      xname_set (path, NULL);
      char *inum;
      asprintf (&inum, "%s/%ld", rpath, GetInode (path));
      ret = rename (path, inum);
      if (ret == 0)
        {
          ret = setxattr (inum, A_RPATH, path, strlen (path), 0);
          logmsg (inum, Deleted, "");
        }
      free (inum);
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_RecoverFSO (json_t *request, json_t *response)
{
  LOG ("_RecoverFSO():");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  const char *recpath = json_string_value (json_array_get (params_object, 1));
  char rpath[MAXLISTLEN] = {0};
  getxattr (path, A_RPATH, rpath, MAXLISTLEN);
  LOG ("%s %s %s", path, rpath, recpath);
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    {
      if (strlen (recpath) == 0)
        {
          ret = rename (path, rpath);
          if (ret == 0)
            {
              ret = removexattr (rpath, A_RPATH);
              logmsg (rpath, Recovered, "");
            }
        }
      else
        {
          struct xattr xa;
          get_xattr (path, &xa);
          char *cpath;
          asprintf (&cpath, "%s/%s", recpath, xa.x_name);
          LOG ("%s %s %s", recpath, cpath, xa.x_name);
          ret = OK (cpath);
          if (ret != 0)
            ret = rename (path, cpath);
          if (ret == 0)
            {
              ret = removexattr (cpath, A_RPATH);
              logmsg (cpath, Recovered, "");
            }
          frees (cpath);
        }
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_RenameFSO (json_t *request, json_t *response)
{
  LOG ("_RenameFSO():");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  const char *newpath = json_string_value (json_array_get (params_object, 1));
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    ret = rename (path, newpath);
  if (ret == 0)
    {
      logmsg (newpath, Renamed, path_name (path));
      xname_set (newpath, NULL);
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_DeleteFSO (json_t *request, json_t *response)
{
  LOG ("_DeleteFSO():");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  json_int_t removeSelf = json_integer_value (json_array_get (params_object, 1));
  LOG (path);
  json_t *o = get_ret_object ();
  int ret;
  ret = OK (path);
  if (ret == 0)
    {
      if (IsRegularFile (path) == 0)
        {
          ret = remove (path);
        }
      if (IsDirectory (path) == 0)
        {
          if (removeSelf == 1)
            rmrf (path);
          else
            {
              DIR *dirp;
              char cwd[256] = {0};
              getcwd (cwd, 256);
              chdir (path);
              struct dirent *dp;
              struct stat sb;
              dirp = opendir (path);
              if (dirp != NULL)
                {
                  for (;;)
                    {
                      errno = 0;
                      dp = readdir (dirp);
                      if (dp == NULL)
                        break;
                      if (strcmp (dp->d_name, ".") == 0 || strcmp (dp->d_name, "..") == 0)
                        continue;
                      if (stat (dp->d_name, &sb) != -1)
                        {
                          if (S_ISDIR (sb.st_mode))
                            {
                              rmrf (dp->d_name);
                            }
                          if (S_ISREG (sb.st_mode))
                            {
                              remove (dp->d_name);
                            }
                        }
                    }
                }
              closedir (dirp);
              chdir (cwd);
            }
        }
    }
  make_responce (response, ret, strerror (errno), o);
}
