
#include <stdio.h>
#include <stdlib.h>
#include <acl/libacl.h>
#include <sys/acl.h>
#include <time.h>
#include <pthread.h>
#include <locale.h>
#include <dlfcn.h>
#include "libcmd.h"
#include "dms.h"

void
_XAttrSetValue (json_t *request, json_t *response)
{
  LOG ("_XAttrSetValue():\n");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  const char *attr_name = json_string_value (json_array_get (params_object, 1));
  const char *attr_value = json_string_value (json_array_get (params_object, 2));
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    {
      ret = setxattr (path, attr_name, attr_value, strlen (attr_value), 0);
    }
  make_responce (response, ret, strerror (errno), o);

}

void
_XAttrGetValue (json_t *request, json_t *response)
{
  LOG ("_XAttrGetValue():\n");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  const char *attr_name = json_string_value (json_array_get (params_object, 1));
  char value[MAXLISTLEN] = {0};
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    {
      getxattr (path, attr_name, value, MAXLISTLEN);
      LOG ("%s %s %s", path, attr_name, value);
      json_object_set_new (o, RETSTR, json_string (value));
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_XAttrGetLog (json_t *request, json_t *response)
{
  LOG ("_XAttrGetLog():");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  char value[MAXLISTLEN] = {0};
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    {
      //if(getxattr (path, A_PRINTJOB, value, MAXLISTLEN) <= 0 && IsMSDoc(path))
      //  {
      //    get_xattr_from_cache(path);          
      //  }
      getxattr (path, A_LOG, value, MAXLISTLEN);
      json_error_t jerror;
      json_t *jlog;
      jlog = json_loads (value, 0, &jerror);
      json_object_set_new (o, RETSTR, jlog);
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_XAttrClearLog (json_t *request, json_t *response)
{
  LOG ("_XAttrClearLog():");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    {
      setxattr (path, A_LOG, "", 0, 0);
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_XAttrGetPrintJob (json_t *request, json_t *response)
{
  LOG ("_XAttrGetPrintJob():");

  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  char value[MAXLISTLEN] = {0};
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    {
      if(getxattr (path, A_PRINTJOB, value, MAXLISTLEN) <= 0 && IsLink (path) == 0)
        {
          char link[PATH_MAX] = {0};
          readlink(path, link, PATH_MAX);
          get_xattr_from_cache(link);          
        }
      getxattr (path, A_PRINTJOB, value, MAXLISTLEN);
      LOG ("printjob: %s", value);
      json_error_t jerror;
      json_t *job;
      job = json_loads (value, 0, &jerror);
      json_object_set_new (o, RETSTR, job);
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_XAttrSetPrintJob (json_t *request, json_t *response)
{
  LOG ("_XAttrSetPrintJob:");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  json_t *job = json_array_get (params_object, 1);
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    {
      char * s = json_dumps (job, JSON_COMPACT);
      ret = setxattr (path, A_PRINTJOB, s, strlen (s), 0);
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_XFlagsSet (json_t *request, json_t *response)
{
  LOG ("_XFlagsSet:\n");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  json_int_t val = json_integer_value (json_array_get (params_object, 1));
  LOG (path);
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    {
      struct xattr xa;
      get_xattr (path, &xa);
      xa.x_flags = (unsigned short) val;
      ret = set_xattr (path, &xa);
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_XListSet (json_t *request, json_t *response)
{
  LOG ("_XListSet:");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  json_int_t val = json_integer_value (json_array_get (params_object, 1));
  LOG (path);
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    {
      struct xattr xa;
      get_xattr (path, &xa);
      xa.x_list = (unsigned int) val;
      ret = set_xattr (path, &xa);
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_XProgressSet (json_t *request, json_t *response)
{
  LOG ("_XProgressSet:");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  json_int_t val = json_integer_value (json_array_get (params_object, 1));
  LOG (path);
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    {
      struct xattr xa;
      get_xattr (path, &xa);
      xa.x_progress = (unsigned int) val;
      ret = set_xattr (path, &xa);
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_XElapseSet (json_t *request, json_t *response)
{
  LOG ("_XElapseSet:");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  json_int_t val = json_integer_value (json_array_get (params_object, 1));
  LOG (path);
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    {
      struct xattr xa;
      get_xattr (path, &xa);
      xa.x_elapse = (unsigned int) val;
      ret = set_xattr (path, &xa);
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_XDateBeginSet (json_t *request, json_t *response)
{
  LOG ("_XDateBeginSet:");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  json_int_t val = json_integer_value (json_array_get (params_object, 1));
  LOG (path);
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    {
      struct xattr xa;
      get_xattr (path, &xa);
      xa.x_date_begin = (unsigned int) val;
      ret = set_xattr (path, &xa);
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_XDateEndSet (json_t *request, json_t *response)
{
  LOG ("_XDateEndSet:");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  json_int_t val = json_integer_value (json_array_get (params_object, 1));
  LOG ("%s\n", path);
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    {
      struct xattr xa;
      get_xattr (path, &xa);
      xa.x_date_end = (unsigned int) val;
      ret = set_xattr (path, &xa);
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_XCommentSet (json_t *request, json_t *response)
{
  LOG ("_XCommentSet:");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  const char *val = json_string_value (json_array_get (params_object, 1));
  LOG (path);
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    {
      struct xattr xa;
      get_xattr (path, &xa);
      memset (xa.x_comment, 0, A_COMMENT_MAX);
      strncpy (xa.x_comment, val, A_COMMENT_MAX);
      LOG("new comment: %s", xa.x_comment);
      ret = set_xattr (path, &xa);
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_XCommonSet (json_t *request, json_t *response)
{
  LOG ("_XCommonSet:");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  const char *val = json_string_value (json_array_get (params_object, 1));
  LOG (path);
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    {
      struct xattr xa;
      get_xattr (path, &xa);
      memset (xa.x_common, 0, A_COMMENT_MAX);
      strncpy (xa.x_common, val, A_COMMENT_MAX);
      ret = set_xattr (path, &xa);
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_XNameSet (json_t *request, json_t *response)
{
  LOG ("_XNameSet:");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *path = json_string_value (json_array_get (params_object, 0));
  const char *val = json_string_value (json_array_get (params_object, 1));
  LOG (path);
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    {
      ret = xname_set (path, val);
    }
  make_responce (response, ret, strerror (errno), o);
}