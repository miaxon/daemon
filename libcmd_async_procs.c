#define _GNU_SOURCE
#define _XOPEN_SOURCE 500 

#include <acl/libacl.h>
#include <sys/acl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <locale.h>
#include <dirent.h>
#include <unistd.h>
#include "libcmd.h"
#include "libcmd_sys.h"
#include "dms.h"

int
filter (struct proc_data *pd, char *path, char *name, struct xattr *xa)
{
  //LOG ("filter %s", path);
  struct stat sb;
  if (stat (path, &sb) != 0)
    {
      //LOG ("== return 0 == 1");
      return 0;
    }
  int f_uid = pd->s_uid < 0 ? 1 : sb.st_uid == pd->s_uid;
  int f_gid = pd->s_gid < 0 ? 1 : sb.st_gid == pd->s_gid;
  int f_name = *(pd->s_name) == 0 ? 1 : strstr (name, pd->s_name) != NULL;
  int f_comment = *(pd->s_comment) == 0 ? 1 : strstr (xa->x_comment, pd->s_comment) != NULL;
  int f_sign = pd->s_sign < 0 ? 1 : IS_SIGN (xa->x_flags) & pd->s_sign;
  int f_ready = pd->s_ready < 0 ? 1 : IS_READY (xa->x_flags) & pd->s_ready;
  return f_uid & f_gid & f_name & f_comment & f_sign & f_ready;

}

void
jsearch (struct proc_data *pd, json_t *jstats, json_t *jxattrs, json_t *jacls, char *iter_path)
{
  DIR *dirp;
  char *path = pd->s_path;
  if (iter_path)
    path = iter_path;
  struct dirent *dp;
  dirp = opendir (path);
  if (dirp != NULL)
    {
      json_t *jret;
      for (;;)
        {
          //errno = 0;
          dp = readdir (dirp);
          if (dp == NULL)
            break;
          if (str_equal (dp->d_name, ".") || str_equal (dp->d_name, ".."))
            continue;
          char *_path;
          asprintf (&_path, "%s/%s", path, dp->d_name);
          //LOG("curr path %s", _path);
          struct xattr xa;
          get_xattr (_path, &xa);
          if (IS_SCAN (xa.x_flags) || IS_BASKET (xa.x_flags))
            {
              frees (_path);
              continue;
            }
          if (IS_DTYPE (dp->d_type, pd->s_mask))
            {
              if (filter (pd, _path, dp->d_name, &xa))
                {
                  jret = get_jstat (_path, dp->d_name);
                  json_array_append (jstats, jret);
                  json_decref (jret);
                  jret = get_jacl (_path, ACL_TYPE_ACCESS);
                  json_array_append (jacls, jret);
                  json_decref (jret);
                  jret = get_jxattr (_path);
                  json_array_append (jxattrs, jret);
                  json_decref (jret);
                }
            }
          if (dp->d_type == DT_DIR)
            {
              if (IS_PROJECT (xa.x_flags) && pd->s_close >= 0)
                {
                  if (IS_CLOSE (xa.x_flags) == pd->s_close)
                    jsearch (pd, jstats, jxattrs, jacls, _path);
                }
              else
                {
                  jsearch (pd, jstats, jxattrs, jacls, _path);
                }
            }
          frees (_path);
        }
      closedir (dirp);
    }

}

void *
proc_search (void *parm)
{
  struct proc_data *pd = parm;
  LOG ("proc_search(%s)", pd->s_path);
  json_t *o = get_async_jobject (pd->s_id);
  json_t *data = json_object ();
  json_t *jstats = json_array ();
  json_t *jxattrs = json_array ();
  json_t *jacls = json_array ();
  setfsuid (0);
  jsearch (pd, jstats, jxattrs, jacls, NULL);
  setfsuid (currentUid);
  json_object_set_new (data, "stats", jstats);
  json_object_set_new (data, "acls", jacls);
  json_object_set_new (data, "xattrs", jxattrs);
  json_object_set_new (o, "data", data);
  send_event (o);
  free (pd);
  return 0;
}
