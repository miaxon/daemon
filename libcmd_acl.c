#define _GNU_SOURCE
#define _XOPEN_SOURCE 500 
#include <stdio.h>
#include <stdlib.h>
#include <acl/libacl.h>
#include <sys/acl.h>
#include <time.h>
#include <pthread.h>
#include <locale.h>
#include <dlfcn.h>
#include "libcmd.h"
#include "libcmd_sys.h"
#include "ugid_functions.h"
#include "dms.h"
json_t *JSON_PARAM;

void
_AclChown (json_t *request, json_t *response)
{
  LOG ("_AclChown()");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  json_int_t uid = json_integer_value (json_array_get (params_object, 1));
  json_int_t gid = json_integer_value (json_array_get (params_object, 2));
  json_int_t rec = json_integer_value (json_array_get (params_object, 3));
  json_t *o = get_ret_object ();
  char *cmd;
  int ret = OK (path);
  if (ret == 0)
    {
      if (IsDirectory (path) == 0 && rec == 1)
        {
          ret = chownR (path, userNameFromId (uid), groupNameFromId (gid));
          if (gid > 0)
            {
              asprintf (&cmd, "chmod -R g+s '%s'", path);
              ret = system (cmd);
            }
        }
      struct stat sb;
      stat (path, &sb);
      ret = chown (path, uid, gid);
      if (ret == 0)
        {
          if (gid < 0)
            {
              asprintf (&cmd, "%d", sb.st_uid);
              logmsg (path, Chowned, cmd);
            }
          if (uid < 0)
            {
              asprintf (&cmd, "%d", sb.st_gid);
              logmsg (path, Chgrped, cmd);
              chmod (path, sb.st_mode | S_ISGID);
            }
        }
      frees (cmd);
    }
  make_responce (response, ret, strerror (errno), o);

}

void
_AclChmod (json_t *request, json_t *response)
{
  LOG ("_AclChmod()");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  json_int_t mode = json_integer_value (json_array_get (params_object, 1));
  json_int_t rec = json_integer_value (json_array_get (params_object, 2));
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    {
      if (IsDirectory (path) == 0 && rec == 1)
        {
          ret = chmodR (path, mode);
        }
      ret = chmod (path, mode);
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_AclClear (json_t *request, json_t *response)
{
  LOG ("_AclClear():");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  json_int_t acl_type = json_integer_value (json_array_get (params_object, 1));
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    {
      ret = clearAcl (path, (int) acl_type);
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_AclGet (json_t *request, json_t *response)
{
  LOG ("_AclGet():");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  json_int_t acl_type = json_integer_value (json_array_get (params_object, 1));
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    {
      LOG (path);
      json_t *jacls = get_jacl (path, (int) acl_type);
      //LOG (json_dumps (jacls, 0));
      json_object_set_new (o, "acl", jacls);
    }
  make_responce (response, 0, strerror (errno), o);
}

int /* Function called by nftw() */
ftw_setAcl (const char *pathname, const struct stat *sbuf, int type, struct FTW *ftwb)
{
  clearAcl (pathname, ACL_TYPE_ACCESS);
  set_jacl (JSON_PARAM, pathname, ACL_TYPE_ACCESS);
  return 0;
}

void
_AclSet (json_t *request, json_t * response)
{
  LOG ("_AclSet():");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  json_t *ent_array = json_array_get (params_object, 1);
  json_int_t rec = json_integer_value (json_array_get (params_object, 2));
  json_int_t acl_type = json_integer_value (json_array_get (params_object, 3));
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    {
      LOG (path);
      if (IsDirectory (path) == 0)
        {
          if (rec == 1)
            {
              if (json_array_size (ent_array) > 0)
                {
                  JSON_PARAM = ent_array;
                  ret = nftw (path, ftw_setAcl, 10, 0);
                  json_decref (JSON_PARAM);
                }
              else
                {
                  ret = setacl_from_file (path, path);
                }
            }
          else
            {
              ret = clearAcl (path, (int) acl_type);
              if (ret == 0)
                ret = set_jacl (ent_array, path, (int) acl_type);
            }
        }
      else
        {
          ret = clearAcl (path, (int) acl_type);
          if (ret == 0)
            ret = set_jacl (ent_array, path, (int) acl_type);
        }
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_AclAddEntry (json_t *request, json_t * response)
{
  LOG ("_AclAddEntry():");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  json_t *jent = json_array_get (params_object, 1);
  json_int_t acl_type = json_integer_value (json_array_get (params_object, 2));
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    {
      LOG(path);
      json_int_t t = json_integer_value (json_object_get (jent, ACL_TAG));
      json_int_t q = json_integer_value (json_object_get (jent, ACL_QUALIFIER));
      json_int_t p = json_integer_value (json_object_get (jent, ACL_PERMSET));
      ret = acl_add_entry (path, t, q, p, (int) acl_type);
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_AclRemoveEntry (json_t *request, json_t * response)
{
  LOG ("_AclRemoveEntry():");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  json_t *jent = json_array_get (params_object, 1);
  json_int_t acl_type = json_integer_value (json_array_get (params_object, 2));
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    {
      LOG ("%s, %x", path, (int) acl_type);
      json_int_t t = json_integer_value (json_object_get (jent, ACL_TAG));
      json_int_t q = json_integer_value (json_object_get (jent, ACL_QUALIFIER));
      ret = acl_remove_entry (path, t, q, (int) acl_type);
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_AclModifyEntry (json_t *request, json_t * response)
{
  LOG ("_AclModifyEntry():");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  json_t *jent = json_array_get (params_object, 1);
  json_int_t acl_type = json_integer_value (json_array_get (params_object, 2));
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    {
      LOG(path);
      json_int_t t = json_integer_value (json_object_get (jent, ACL_TAG));
      json_int_t q = json_integer_value (json_object_get (jent, ACL_QUALIFIER));
      json_int_t p = json_integer_value (json_object_get (jent, ACL_PERMSET));
      ret = acl_modify_entry (path, t, q, p, (int) acl_type);
    }
  make_responce (response, ret, strerror (errno), o);
}
