#define _GNU_SOURCE
#define _XOPEN_SOURCE 500 
#include <stdio.h>
#include <stdlib.h>
#include <acl/libacl.h>
#include <sys/acl.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <openssl/md5.h>
#include "libcmd.h"
#include "dms.h"
#include "ugid_functions.h"
#include "libcmd_sys.h"
#include <ftw.h>
struct xattr XATTR_DEF = {
  .x_flags = 0,
  .x_name =
  {0},
  .x_comment =
  {0},
  .x_common =
  {0},
  .x_date_begin = 0,
  .x_date_end = 0,
  .x_list = 0,
  .x_progress = 0,
  .x_elapse = 0

};

int
inherit_perms (const char *src, const char *dst)
{
  LOG ("inherit perms: %s -> %s", src, dst);
  int ret;
  struct stat sb;
  ret = stat (src, &sb);
  if (ret == 0)
    ret = chownR (dst, userNameFromId (currentUid), groupNameFromId (sb.st_gid));
  if (ret == 0)
    ret = chmodR (dst, sb.st_mode);
  if (ret == 0)
    ret = setacl_from_file (src, dst);
  return ret;
}

int
setacl_from_file (const char *src, const char *dst)
{
  char *cmd;
  asprintf (&cmd, "getfacl '%s' | setfacl -R --set-file=- '%s'", src, dst);
  int ret = system (cmd);
  free (cmd);
  return ret;
}

const char *
path_name (const char *path)
{
  const char *base = path;
  while (*path)
    {
      if (*path == '/')
        base = path + 1;
      ++path;
    }
  return base;
}

const char *
path_parent (const char *path, char *buf)
{
  const char *p = path_name (path);
  if (buf && p)
    strncpy (buf, path, p - path - 1);
  return p;
}

int
mvf (const char* src, const char* dst)
{
  char *cmd;
  asprintf (&cmd, "mv -f '%s' '%s'", src, dst);
  int ret = system (cmd);
  free (cmd);
  return ret;
}

int
chmodR (const char *path, mode_t mode)
{
  char *cmd;
  asprintf (&cmd, "chmod -R %o '%s'", mode & 0xFFF, path);
  int ret = system (cmd);
  free (cmd);
  return ret;
}

int
chownR (const char *path, const char *user, const char *group)
{
  char *cmd;
  if (user == NULL && group == NULL)
    return -1;
  if (user != NULL && group == NULL)
    {
      asprintf (&cmd, "chown -R %s '%s'", user, path);
    }
  if (user == NULL && group != NULL)
    {
      asprintf (&cmd, "chown -R :%s '%s'", group, path);
    }
  if (user != NULL && group != NULL)
    {
      asprintf (&cmd, "chown -R %s:%s '%s'", user, group, path);
    }
  int ret = system (cmd);
  free (cmd);
  return ret;
}

int
rmrf (const char* path)
{
  char *cmd;
  asprintf (&cmd, "rm -rf '%s'", path);
  int ret = system (cmd);
  free (cmd);
  return ret;
}

int
cpax (const char* src, const char* dst)
{
  char *cmd;
  asprintf (&cmd, "cp -ax '%s' '%s'", src, dst);
  int ret = system (cmd);
  free (cmd);
  return ret;
}

int
log_echo (const char *path, char *str, int max_size)
{
  struct stat sb;
  if (stat (path, &sb) != 0 && sb.st_size > max_size)
    {
      int i = 0;
      char *ppath;
      asprintf (&ppath, "%s.%d", path, i);
      while (OK (ppath) == 0)
        {
          free (ppath);
          i++;
          asprintf (&ppath, "%s.%d", path, i);
        }
      rename (path, ppath);
      free (ppath);
    }
  time_t t = time (0);
  struct tm tstruct = *localtime (&t);
  char tstr[256] = "??Unknown time??: ";
  strftime (tstr, sizeof (tstr), "%Y-%m-%d %H:%M", &tstruct);
  char *cmd;
  asprintf (&cmd, "echo '%s[%s]: %s' >> '%s'", tstr, userNameFromId (currentUid), str, path);
  int ret = system (cmd);
  free (cmd);
  return ret;
}

int
close_dir (const char* path)
{
  char *cmd;
  asprintf (&cmd, "/monolit/dms/backend/sh/close_d '%s'", path);
  int ret = system (cmd);
  free (cmd);
  return ret;
}

int
unclose_dir (const char* path)
{
  char *cmd;
  asprintf (&cmd, "/monolit/dms/backend/sh/unclose_d '%s'", path);
  int ret = system (cmd);
  free (cmd);
  return ret;
}

int
clear_dir (const char* path)
{
  char *cmd;
  asprintf (&cmd, "find '%s' -type f -delete", path);
  int ret = system (cmd);
  free (cmd);
  return ret;
}

int
copyFile (const char *path1, const char *path2)
{
  int inputFd, outputFd, openFlags;
  mode_t filePerms;
  ssize_t numRead;
  char buf[1024];
  inputFd = open (path1, O_RDONLY);
  if (inputFd == -1)
    return -1;

  openFlags = O_CREAT | O_WRONLY | O_TRUNC;
  filePerms = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH; /* 755 */
  outputFd = open (path2, openFlags, filePerms);
  if (outputFd == -1)
    return -1;
  while ((numRead = read (inputFd, buf, 1024)) > 0)
    if (write (outputFd, buf, numRead) != numRead)
      return -1;
  if (numRead == -1)
    return -1;
  close (inputFd);
  close (outputFd);
  return 0;
}

int
clearAcl (const char* path, int acl_type)
{
  char *cmd;
  if (acl_type == ACL_TYPE_ACCESS)
    asprintf (&cmd, "setfacl -b '%s'", path);
  if (acl_type == ACL_TYPE_DEFAULT)
    asprintf (&cmd, "setfacl -k '%s'", path);
  return system (cmd);
}

unsigned long int
GetInode (const char *path)
{
  struct stat sb;
  if (stat (path, &sb) != 0)
    return -1;
  return sb.st_ino;
}

int
IsRegularFile (const char *path)
{
  struct stat sb;
  if (stat (path, &sb) == -1)
    return -1;
  if (S_ISREG (sb.st_mode))
    return 0;
  return -1;
}

int
IsDirectory (const char *path)
{
  struct stat sb;
  if (stat (path, &sb) == -1)
    return -1;
  if (S_ISDIR (sb.st_mode))
    return 0;
  return -1;
}

int
IsLink (const char *path)
{
  struct stat lsb, sb;
  if (lstat (path, &lsb) == -1)
    return -1;
  if (S_ISLNK (lsb.st_mode) && stat (path, &sb) == 0)
    return 0;
  if (S_ISLNK (lsb.st_mode) && stat (path, &sb) != 0)
    return 1;
  return -1;
}

int
IsDirectoryEmpty (const char *path)
{
  DIR *dirp;
  struct dirent *dp;
  dirp = opendir (path);
  int count = 0;
  if (dirp != NULL)
    {
      for (;;)
        {
          dp = readdir (dirp);
          if (dp == NULL)
            break;
          if (strcmp (dp->d_name, ".") == 0 || strcmp (dp->d_name, "..") == 0)
            continue;
          count++;
          break;
        }
      closedir (dirp);
    }
  return count;
}

int
IsDirHasSubDirs (const char *path)
{
  DIR *dirp;
  struct dirent *dp;
  dirp = opendir (path);
  int count = 0;
  if (dirp != NULL)
    {
      for (;;)
        {
          dp = readdir (dirp);
          if (dp == NULL)
            break;
          if (strcmp (dp->d_name, ".") == 0 || strcmp (dp->d_name, "..") == 0)
            continue;
          if (dp->d_type == DT_DIR)
            {
              count++;
              break;
            }
        }
      closedir (dirp);
    }
  return count;
}

int
unlink_cb (const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
  int rv = remove (fpath);
  return rv;
}

int
acl_count (const char *path, const char* name)
{
  char *_path;
  asprintf (&_path, "%s/%s", path, name);
  int ret = 0;
  acl_t acl = acl_get_file (_path, ACL_TYPE_ACCESS);
  if (acl != NULL)
    {
      int entryId;
      for (entryId = ACL_FIRST_ENTRY;; entryId = ACL_NEXT_ENTRY)
        {
          acl_entry_t entry;
          if (acl_get_entry (acl, entryId, &entry) != 1)
            break;
          acl_tag_t tag;
          if (acl_get_tag_type (entry, &tag) != -1)
            {
              if (tag == ACL_GROUP || tag == ACL_USER)
                {
                  ret++;
                }
            }
        }
    }
  free (_path);
  return ret;
}

json_t
*
get_jacl (const char* path, int acl_type)
{
  //#ifndef VFS_FIFO
  //printf ("get_jacl(%s, 0x%X)\n", path, acl_type );
  //#endif
  const char *name = "";
  if (acl_type == ACL_TYPE_ACCESS)
    name = ACL_EA_ACCESS;
  if (acl_type == ACL_TYPE_DEFAULT)
    name = ACL_EA_DEFAULT;

  json_t *jacls = json_array ();
  int attr_size = getxattr (path, name, NULL, 0);
  if (attr_size <= 0)
    return jacls;
  int *buf, bufs[ACL_EA_SIZE] = {0};
  if (attr_size > ACL_EA_SIZE)
    {
      int sz = (attr_size) / sizeof (int) + 1;
      buf = (int*) calloc (sz, sizeof (int));
    }
  else
    {
      buf = bufs;
    }
  getxattr (path, name, buf, attr_size);
  int i = 1;
  while (*(buf + i) != 0)
    {
      json_t *jacl = json_object ();
      int a = *(buf + i);
      int t = LOWORD (a);
      int p = HIWORD (a);
      i++;
      a = *(buf + i);
      int q = a == ACL_UNDEFINED_ID ? 0 : a;
      json_object_set_new (jacl, ACL_TAG, json_integer (t));
      json_object_set_new (jacl, ACL_QUALIFIER, json_integer (q));
      json_object_set_new (jacl, ACL_PERMSET, json_integer (p));
      //printf("acl entry: %s\n", json_dumps(jacl, JSON_COMPACT));
      json_array_append (jacls, jacl);
      json_decref (jacl);
      i++;
    }
  if (buf != bufs)
    free (buf);
  return jacls;
}

int
set_jacl (json_t *ent_array, const char *path, int acl_type)
{
#ifndef VFS_FIFO
  printf ("set_jacl() %s \n", path);
#endif
  int ret = 0;
  if (json_is_array (ent_array))
    {
      int len = json_array_size (ent_array);
      if (len > 0)
        {
          acl_t acl = acl_get_file (path, acl_type);
          if (acl != NULL)
            {
              int i = 0;
              while (i < len)
                {
                  json_t *jent = json_array_get (ent_array, i);
                  acl_entry_t entry;
                  if (jent != NULL && acl_create_entry (&acl, &entry) != -1)
                    {
                      json_int_t t = json_integer_value (json_object_get (jent, ACL_TAG));
                      json_int_t q = json_integer_value (json_object_get (jent, ACL_QUALIFIER));
                      json_int_t m = json_integer_value (json_object_get (jent, ACL_PERMSET));
                      acl_set_tag_type (entry, t);
                      acl_set_qualifier (entry, &q);
                      acl_permset_t permset;
                      if (acl_get_permset (entry, &permset) != -1)
                        ((mode_t*) permset)[0] = m;
                    }
                  else
                    return errno;
                  i++;
                }
              acl_calc_mask (&acl);
              if (acl_valid (acl) == 0)
                ret = acl_set_file (path, acl_type, acl);
              acl_free (acl);
            }
          else
            ret = errno;
        }
    }
  else
    ret = EINVAL;
  return ret;
}

json_t
*
get_jxattr (const char* path)
{
  //printf ("get_jxattr %s\n", path);
  struct xattr xa;
  get_xattr (path, &xa);
  json_t *jxattr = json_object ();
  json_object_set_new (jxattr, A_XFLAGS, json_integer (xa.x_flags));
  json_object_set_new (jxattr, A_XDATE1, json_integer (xa.x_date_begin));
  json_object_set_new (jxattr, A_XDATE2, json_integer (xa.x_date_end));
  json_object_set_new (jxattr, A_XLIST, json_integer (xa.x_list));
  json_object_set_new (jxattr, A_XELAPSE, json_integer (xa.x_elapse));
  json_object_set_new (jxattr, A_XPROGRESS, json_integer (xa.x_progress));
  json_object_set_new (jxattr, A_XNAME, json_string (xa.x_name));
  json_object_set_new (jxattr, A_XCOMMENT, json_string (xa.x_comment));
  json_object_set_new (jxattr, A_XCOMMON, json_string (xa.x_common));
  /*
  char value[MAXLISTLEN] = {0};
  for (int i = 0; i < xattr_len; i++)
    {
      const char *attr_name = xattr[i];
      const char* attr_short_name = &attr_name[5];
      //printf ("attr_name '%s'\n", attr_name);
      //printf ("attr_short_name '%s'\n", attr_short_name);
      memset (value, 0, MAXLISTLEN);
      getxattr (path, attr_name, value, MAXLISTLEN);
      //printf ("value '%s'\n", value);
      json_object_set_new (jxattr, attr_short_name, json_string (value));
    }
   */
  return jxattr;
}

json_t
*
get_jstat (const char* path, const char* vname)
{
  json_t *jstat = json_object ();
  struct stat sb;
  stat (path, &sb);
  char lnk[1024] = {0};
  if (IsLink (path) == 0)
    readlink (path, lnk, 1024);
  json_object_set_new (jstat, "mode", json_integer (sb.st_mode));
  json_object_set_new (jstat, "uid", json_integer (sb.st_uid));
  json_object_set_new (jstat, "gid", json_integer (sb.st_gid));
  json_object_set_new (jstat, "atime", json_integer (sb.st_atime));
  json_object_set_new (jstat, "ctime", json_integer (sb.st_ctime));
  json_object_set_new (jstat, "mtime", json_integer (sb.st_mtime));
  json_object_set_new (jstat, "size", json_integer (sb.st_size));
  json_object_set_new (jstat, "inum", json_integer (sb.st_ino));
  json_object_set_new (jstat, "name", json_string (vname));
  json_object_set_new (jstat, "link", json_string (lnk));
  json_object_set_new (jstat, "path", json_string (path));
  json_object_set_new (jstat, "sub", json_integer (IsDirHasSubDirs (path)));
  return jstat;
}

/*int
set_jxattr (const char *path, json_t *jxattr)
{
  int ret = 0;
  for (int i = 0; i < xattr_len; i++)
    {
      const char *attr_name = xattr[i];
      const char* attr_short_name = &attr_name[5];
      const char *value = json_string_value (json_object_get (jxattr, attr_short_name));
      ret = setxattr (path, attr_name, value, strlen (value), 0);
      if (ret != 0)
        break;
    }
  return ret;
}*/

int
acl_modify_entry (const char *path, int t, int q, int p, int acl_type)
{
  int ret = 0;
  acl_t acl = acl_get_file (path, acl_type);
  if (acl != NULL)
    {
      int entryId;
      acl_entry_t ent;
      acl_tag_t tag;
      for (entryId = ACL_FIRST_ENTRY;; entryId = ACL_NEXT_ENTRY)
        {
          if (acl_get_entry (acl, entryId, &ent) != 1)
            break;
          if (acl_get_tag_type (ent, &tag) != -1)
            {
              if (tag == t)
                {
                  if (tag == ACL_USER || tag == ACL_GROUP)
                    {
                      uid_t *qualp;
                      qualp = acl_get_qualifier (ent);
                      if (*qualp == q)
                        {
                          acl_permset_t permset;
                          if (acl_get_permset (ent, &permset) != -1)
                            ((mode_t*) permset)[0] = p;
                          break;
                        }
                    }
                  else
                    {
                      acl_permset_t permset;
                      if (acl_get_permset (ent, &permset) != -1)
                        ((mode_t*) permset)[0] = p;
                      break;
                    }
                }
            }
        }
      //acl_calc_mask (&acl);
      ret = acl_valid (acl);
      if (ret == 0)
        ret = acl_set_file (path, acl_type, acl);
      acl_free (acl);
    }
  else
    ret = errno;
  return ret;
}

int
acl_remove_entry (const char* path, int t, int q, int acl_type)
{
  int ret = 0;
  if (t == ACL_USER || t == ACL_GROUP)
    {
      acl_t acl = acl_get_file (path, acl_type);
      if (acl != NULL)
        {
          int entryId;
          acl_entry_t ent;
          acl_tag_t tag;
          for (entryId = ACL_FIRST_ENTRY;; entryId = ACL_NEXT_ENTRY)
            {
              if (acl_get_entry (acl, entryId, &ent) != 1)
                break;
              if (acl_get_tag_type (ent, &tag) != -1)
                {
                  if (tag == t)
                    {
                      uid_t *qualp;
                      qualp = acl_get_qualifier (ent);
                      if (*qualp == q)
                        {
                          acl_delete_entry (acl, ent);
                          break;
                        }
                    }
                }
            }
          //acl_calc_mask (&acl);
          ret = acl_valid (acl);
          if (ret == 0)
            ret = acl_set_file (path, acl_type, acl);
          acl_free (acl);
        }
    }
  else
    ret = EINVAL;
  return ret;
}

int
acl_add_entry (const char* path, int t, int q, int p, int acl_type)
{
  int ret = 0;
  acl_t acl = acl_get_file (path, acl_type);
  if (acl != NULL)
    {
      acl_entry_t entry;
      if (acl_create_entry (&acl, &entry) != -1)
        {

          acl_set_tag_type (entry, t);
          if (t == ACL_USER || t == ACL_GROUP)
            acl_set_qualifier (entry, &q);
          acl_permset_t permset;
          if (acl_get_permset (entry, &permset) != -1)
            ((mode_t*) permset)[0] = p;
          //acl_calc_mask (&acl);
          ret = acl_valid (acl);
          if (ret == 0)
            ret = acl_set_file (path, acl_type, acl);
          acl_free (acl);
        }
      else
        ret = errno;
    }
  else
    ret = errno;
  return ret;
}

void
logmsg (const char *path, LOGMSG act, const char *parm)
{
  char value[MAXLISTLEN] = {0};
  getxattr (path, A_LOG, value, MAXLISTLEN);
  json_error_t jerror;
  json_t *jlog;
  jlog = json_loads (value, 0, &jerror);
  json_t *arr;
  if (jlog == NULL)
    {
      jlog = json_object ();
      json_object_set_new (jlog, "log", json_array ());
    }

  arr = json_object_get (jlog, "log");
  time_t now = time (NULL);
  json_t *o = json_object ();
  json_object_set_new (o, "time", json_integer (now));
  json_object_set_new (o, "uid", json_integer (currentUid));
  json_object_set_new (o, "act", json_integer (act));
  json_object_set_new (o, "parm", json_string (parm));
  LOG ("log length: %d, max: %d", json_array_size (arr), MAXLOGSIZE);
  while (json_array_size (arr) > MAXLOGSIZE)
    {
      json_array_remove (arr, 0);
    }
  LOG ("log length: %d, max: %d", json_array_size (arr), MAXLOGSIZE);
  //if (act != Opened)
  json_array_append (arr, o);
  json_decref (o);
  char *str = json_dumps (jlog, JSON_COMPACT);
  setxattr (path, A_LOG, str, strlen (str), 0);
  free (str);
  json_decref (arr);
  json_decref (jlog);

}

void
jfree (json_t *o)
{
  while (o->refcount != 0)
    json_decref (o);
}

void
set_mode (const char *path, mode_t m)
{
  acl_t acl = acl_get_file (path, ACL_TYPE_ACCESS);
  if (acl != NULL)
    {
      int entryId;
      for (entryId = ACL_FIRST_ENTRY;; entryId = ACL_NEXT_ENTRY)
        {
          acl_entry_t entry;
          if (acl_get_entry (acl, entryId, &entry) != 1)
            break;
          acl_tag_t tag;
          if (acl_get_tag_type (entry, &tag) != -1)
            {
              if (tag != ACL_MASK)
                {
                  acl_permset_t permset;
                  if (acl_get_permset (entry, &permset) != -1)
                    {
                      ((mode_t*) permset)[0] = m;
                    }
                }
            }
        }
      acl_free (acl);
    }
}

int
get_mode (const char *str)
{
  return strtol (str, 0, 8);
}

int
xattr_equal (const char *path, const char *attr_name, const char *attr_value)
{
  if (attr_value == NULL)
    {
      return 1;
    }
  char value[MAXLISTLEN] = {0};
  memset (value, 0, MAXLISTLEN);
  getxattr (path, attr_name, value, MAXLISTLEN);
  return strcmp (value, attr_value) == 0;
}

json_t *
get_jchild (const char *path, int mask, int flags)
{
  DIR *dirp;
  json_t *o = json_object ();
  struct dirent *dp;
  dirp = opendir (path);
  if (dirp != NULL)
    {
      json_t *jstats = json_array ();
      json_t *jxattrs = json_array ();
      json_t *jacls = json_array ();
      json_t *jret;
      for (;;)
        {
          errno = 0;
          dp = readdir (dirp);
          if (dp == NULL)
            break;
          if (strcmp (dp->d_name, ".") == 0 || strcmp (dp->d_name, "..") == 0)
            continue;
          char *_path;
          asprintf (&_path, "%s/%s", path, dp->d_name);
          if (flags != 0)
            {
              struct xattr xa;
              get_xattr (_path, &xa);
              if (xflag_check (&xa, flags) != 1)
                continue;
            }
          struct stat sb;
          if (stat (_path, &sb) != 0 && lstat (_path, &sb) == 0)
            {
              unlink (_path);
              continue;
            }
          if (mask == 0 || (dp->d_type & (mask >> 12)))
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
          free (_path);
        }
      json_object_set_new (o, "stats", jstats);
      json_object_set_new (o, "acls", jacls);
      json_object_set_new (o, "xattrs", jxattrs);
    }
  closedir (dirp);
  return o;
}

char
*
get_ro_fso (const char *path)
{
  LOG ("get_fso_temp: %s", path);
  struct xattr xa;
  get_xattr (path, &xa);
  const char *name = xa.x_name;
  if (strlen (name) == 0)
    name = path_name (path);
  char *user = userNameFromId (currentUid);
  char *path_get = NULL;
  int ret = access (path, F_OK);
  if (ret == 0)
    {
      struct conf_param *cfp = get_cfp ();
      char *tdir, *tfile;
      ino_t inum = GetInode (path);
      asprintf (&tdir, cfp->tdir, user, inum);
      asprintf (&tfile, cfp->tfile, user, inum, name);
      if (access (tdir, F_OK) != 0)
        {
          ret = mkdir (tdir, S_IRUSR | S_IXUSR);
          ret = chown (tdir, currentUid, -1);

        }
      if (ret == 0 && access (tfile, F_OK) == 0)
        {
          ret = rmrf (tfile);
        }
      ret = cpax (path, tfile);
      // (path, Opened, "");
      if (ret == 0)
        asprintf (&path_get, cfp->tpath, inum, name);
      free (tdir);
      free (tfile);
    }
  return path_get;
}

char
*
get_fso (int op, const char *target_path)
{
  const char* path = target_path;
  if (IsLink (path) == 0)
    {
      char lnk[PATH_MAX] = {0};
      int lnk_len = readlink (path, lnk, PATH_MAX);
      LOG ("open symlink: %s -> %s", path, lnk);
      path = lnk;
    }
  struct xattr xa;
  get_xattr (path, &xa);
  char parent[PATH_MAX] = {0};
  const char *name = path_parent (path, parent);

  if (op == FSOP_Read)
    return get_ro_fso (path);

  if (strlen (xa.x_name) != 0)
    name = xa.x_name;
  char *user = userNameFromId (currentUid);
  char *path_get = NULL;
  int ret = OK (path);
  if (ret == 0)
    {
      struct conf_param *cfp = get_cfp ();
      char *tdir;
      ino_t inum = GetInode (parent);
      asprintf (&tdir, cfp->tdir, user, inum);
      //if (IsLink (tdir) > 0)
      int isLink = IsLink (tdir);
      if (isLink > 0)
        {
          unlink (tdir);
          LOG ("unlink: %s, %s", tdir, strerror (errno));
        }
      if (isLink < 0 && IsDirectory (tdir))
        {
          rmrf (tdir);
          LOG ("unlink: %s, %s", tdir, strerror (errno));
        }
      if (OK (tdir) != 0)
        ret = symlink (parent, tdir);
      else
        ret = 0;
      LOG ("symlink: %s, %s, %d", parent, tdir, ret);
      if (ret == 0)
        {
          switch (op)
            {
            case FSOP_Link://link
              // (path, Opened, "");
              break;
            case FSOP_Copy://copy
              logmsg (path, Copy, "");
              break;
            case FSOP_Move://move
              logmsg (path, Moved, path);
              break;
            }
          if (IsMSDoc (path))
            cache_xattr (path);
        }
      if (ret == 0)
        asprintf (&path_get, cfp->tpath, inum, name);
      free (tdir);
    }
  LOG ("path_get: %s", path_get);
  return path_get;
}

void
get_md5_hash (const char *key, char *buf)
{
  MD5_CTX md5handler;
  unsigned char md5digest[MD5_DIGEST_LENGTH];
  MD5 (key, strlen (key), md5digest);
  int i = 0;
  char *ptr = buf;
  while (i < MD5_DIGEST_LENGTH)
    {
      sprintf (ptr, "%02x", md5digest[i]);
      ptr++;
      i++;
    }
}

char *
get_cache_path (const char *path)
{
  char hash[33] = {0};
  get_md5_hash (path, hash);
  char *cpath;
  asprintf (&cpath, "/monolit.back/storage/cache/%s", hash);
  return cpath;
}

void
cache_xattr (const char *path)
{
  char *cpath = get_cache_path (path);
  LOG ("cache_xattr: %s -> %s", path, cpath);
  char *cpath_log, *cpath_prn, *cpath_bin;
  int ret;
  asprintf (&cpath_log, "%s.log", cpath);
  asprintf (&cpath_prn, "%s.prn", cpath);
  asprintf (&cpath_bin, "%s.bin", cpath);
  //
  FILE *cbin = fopen (cpath_bin, "w+");
  int sz = sizeof (struct xattr);
  struct xattr xa;
  ret = getxattr (path, A_XBIN, &xa, sz);
  if (ret != sz)
    {
      memcpy (&xa, &XATTR_DEF, sz);
      const char *name = path_name (path);
      memcpy (xa.x_name, name, strlen (name));
    }
  ret = fwrite (&xa, sz, 1, cbin);
  fclose (cbin);
  free (cpath_bin);
  //
  char prn[MAXLISTLEN] = {0};
  FILE *cprn = fopen (cpath_prn, "w+");
  if (getxattr (path, A_PRINTJOB, prn, MAXLISTLEN) > 0)
    ret = fwrite ((void*) prn, strlen (prn), 1, cprn);
  fclose (cprn);
  free (cpath_prn);
  //
  char log[MAXLISTLEN] = {0};
  FILE *clog = fopen (cpath_log, "w+");
  if (getxattr (path, A_LOG, log, MAXLISTLEN) > 0)
    ret = fwrite ((void*) log, strlen (log), 1, clog);
  fclose (clog);
  free (cpath_log);
  //
  frees (cpath);
}

int
get_xattr_from_cache (const char *path)
{
  int ret = 0;
  char *cpath = get_cache_path (path);
  //LOG ("get_xattr_from_cache: %s -> %s", cpath, path);
  char *cpath_log, *cpath_prn, *cpath_bin;
  int sz = 0;
  asprintf (&cpath_log, "%s.log", cpath);
  asprintf (&cpath_prn, "%s.prn", cpath);
  asprintf (&cpath_bin, "%s.bin", cpath);
  //
  if (OK (cpath_bin) == 0)
    {
      FILE *cbin = fopen (cpath_bin, "r");
      if (cbin != NULL)
        {
          sz = sizeof (struct xattr);
          struct xattr xa;
          ret = fread (&xa, sz, 1, cbin);
          setxattr (path, A_XBIN, &xa, sz, 0);
          fclose (cbin);
          remove (cpath_bin);
        }
      else
        {
          LOGD ("ERROR: fopen '%s' failed: %d (%s)", cpath_bin, errno, strerror (errno));
        }
    }
  //
  if (OK (cpath_prn) == 0)
    {
      char prn[MAXLISTLEN] = {0};
      FILE *cprn = fopen (cpath_prn, "r");
      if (cprn != NULL)
        {
          fseek (cprn, 0, SEEK_END);
          sz = ftell (cprn);
          fseek (cprn, 0, SEEK_SET);
          ret = fread (prn, sz, 1, cprn);
          setxattr (path, A_PRINTJOB, prn, strlen (prn), 0);
          fclose (cprn);
          remove (cpath_prn);
        }
      else
        {
          LOGD ("ERROR: fopen '%s' failed: %d (%s)", cpath_prn, errno, strerror (errno));
        }
    }
  //
  if (OK (cpath_log) == 0)
    {
      char log[MAXLISTLEN] = {0};
      FILE *clog = fopen (cpath_log, "r");
      if (clog != NULL)
        {
          fseek (clog, 0, SEEK_END);
          sz = ftell (clog);
          fseek (clog, 0, SEEK_SET);
          ret = fread (log, sz, 1, clog);
          setxattr (path, A_LOG, log, strlen (log), 0);
          fclose (clog);
          remove (cpath_log);
        }
      else
        {
          LOGD ("ERROR: fopen '%s' failed: %d (%s)", cpath_log, errno, strerror (errno));
        }
    }
  //
  frees (cpath_bin);
  frees (cpath_prn);
  frees (cpath_log);
  frees (cpath);
  return ret;
}

int
IsMSDoc (const char *path)
{
  return strstr (path, ".doc") != NULL || strstr (path, ".docx") != NULL \
          || strstr (path, ".xls") != NULL || strstr (path, ".xlsx") != NULL \
          || strstr (path, ".ppt") != NULL || strstr (path, ".pptx") != NULL;
}

void
frees (void *ptr)
{
  if (ptr != NULL)
    free (ptr);
}

struct proc_data *
get_proc_data ()
{
  struct proc_data *pd = malloc (sizeof (struct proc_data));
  memset (pd, 0, sizeof (struct proc_data));
  return pd;
}

json_t *
get_async_jobject (int id)
{
  json_t *o = json_object ();
  json_t *retint = json_integer (id);
  json_object_set_new (o, "id", retint);
  return o;
}

int
str_equal (const char* str1, const char *str2)
{
  return strcmp (str1, str2) == 0;
}

int
xflag_check (struct xattr *xa, unsigned short f)
{
  if (IS_BASKET (xa->x_flags))
    return 1;

  unsigned short flags = HIWORD (f);
  unsigned short type = LOWORD (f);
  unsigned short xflags = HIWORD (xa->x_flags);
  unsigned short xtype = LOWORD (xa->x_flags);

  if (flags & xflags != flags)
    return 0;
  if (type & xtype != type)
    return 0;
  //return (xa->x_flags & f) == f ? 1 : 0;
  return 1;
}

void
xflag_set (struct xattr *xa, unsigned short f)
{
  xa->x_flags |= f & A_FLAG_ALL;
}

void
xflag_unset (struct xattr *xa, unsigned short f)
{
  xa->x_flags &= f ^ A_FLAG_ALL;
}

int
set_xattr (const char *path, struct xattr *xa)
{

  int sz = sizeof (struct xattr);
  int ret = setxattr (path, A_XBIN, xa, sz, 0);
  if (ret < 0)
    {
      // try make xattr log member small
      char value[MAXLISTLEN] = {0};
      getxattr (path, A_LOG, value, MAXLISTLEN);
      json_error_t jerror;
      json_t *jlog;
      jlog = json_loads (value, 0, &jerror);
      json_t *arr;
      if (jlog == NULL)
        {
          jlog = json_object ();
          json_object_set_new (jlog, "log", json_array ());
        }
      arr = json_object_get (jlog, "log");
      LOG ("log length: %d, max: %d", json_array_size (arr), MAXLOGSIZE);
      while (json_array_size (arr) > MAXLOGSIZE)
        {
          json_array_remove (arr, 0);
        }
      char *str = json_dumps (jlog, JSON_COMPACT);
      setxattr (path, A_LOG, str, strlen (str), 0);
      free (str);
      json_decref (arr);
      json_decref (jlog);
      LOG ("log length: %d, max: %d", json_array_size (arr), MAXLOGSIZE);
      ret = setxattr (path, A_XBIN, xa, sz, 0);
    }
  LOG ("set_xattr: %s, %d, %s", path, ret, strerror (errno));
  return ret;
}

int
get_xattr (const char *target_path, struct xattr *xa)
{
  const char *path = target_path;
  if (IsLink (path) == 0)
    {
      char lnk[PATH_MAX] = {0};
      int lnk_len = readlink (path, lnk, PATH_MAX);
      LOG ("open symlink: %s -> %s", path, lnk);
      path = lnk;
    }
  int sz = sizeof (struct xattr);
  int ret = getxattr (path, A_XBIN, xa, sz);
  if (ret != sz && IsMSDoc (path))
    {
      get_xattr_from_cache (path);
      ret = getxattr (path, A_XBIN, xa, sz);
    }
  if (ret != sz)
    {
      memcpy (xa, &XATTR_DEF, sz);
      const char *name = path_name (path);
      memcpy (xa->x_name, name, strlen (name));
    }
  return ret;
}

int
xname_set (const char *path, const char *val)
{
  struct xattr xa;
  const char *name = val;
  if (val == NULL)
    name = path_name (path);
  get_xattr (path, &xa);
  memset (xa.x_name, 0, A_NAME_MAX);
  strncpy (xa.x_name, name, A_NAME_MAX);
  return set_xattr (path, &xa);
}
