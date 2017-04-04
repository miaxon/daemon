#define _GNU_SOURCE
#define _XOPEN_SOURCE 500 

#include <acl/libacl.h>
#include <sys/acl.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <locale.h>
#include <dlfcn.h>
#include "libcmd.h"
#include "libcmd_sys.h"
#include "dms.h"
#include "libsmbclient.h"
#include <pthread.h>
#include "dmsd.h"

void
send_event (json_t *event_object)
{
  char *jstring = json_dumps (event_object, JSON_COMPACT);
  int rlen = 0;
  char *data = m_crypt (jstring, &rlen);
  free (jstring);
  //json_object_clear (event_object);
  json_decref (event_object);
  conf_param_t *cfp = get_cfp ();
  LOGD ("===== send event pipe_port '%s', sem_name '%s'", cfp->pipePortEvent, cfp->pipeLockSem);
  write_pipe (cfp->pipePortEvent, cfp->pipeLockSem, data, rlen);
  free (data);
}

int
start_async_proc (void *(*start)(void *), void *arg)
{
  int ret = -1;
  pthread_t thr;
  pthread_attr_t attr;
  ret = pthread_attr_init (&attr); /* Assigns default values */
  if (ret != 0)
    goto exit;
  ret = pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
  if (ret != 0)
    goto exit;
  ret = pthread_create (&thr, &attr, start, arg);
  pthread_attr_destroy (&attr); /* No longer needed */
  pthread_detach (thr);
exit:
  return ret;
}

void
_AsyncSearch (json_t *request, json_t * response)
{
  LOG ("_AsyncSearch():");

  json_t *params_object = json_object_get (request, PARAMS);
  json_int_t id = json_integer_value (json_array_get (params_object, 0));
  json_t *filter = json_array_get (params_object, 1);
  const char *path = json_string_value (json_object_get (filter, "path"));
  json_int_t gid = json_integer_value (json_object_get (filter, "gid"));
  json_int_t uid = json_integer_value (json_object_get (filter, "uid"));
  json_int_t mask = json_integer_value (json_object_get (filter, "mask"));
  const char *name = json_string_value (json_object_get (filter, "name"));
  const char *comment = json_string_value (json_object_get (filter, "comment"));
  json_int_t f_ready = json_integer_value (json_object_get (filter, "ready"));
  json_int_t f_sign = json_integer_value (json_object_get (filter, "sign"));
  json_int_t f_close = json_integer_value (json_object_get (filter, "close"));
  json_t *o = get_ret_object ();
  int ret = OK (path);
  if (ret == 0)
    {
      LOG (path);
      struct proc_data *pd = get_proc_data ();
      pd->s_id = (int) id;
      strncpy (pd->s_path, path, 1024);
      strncpy (pd->s_name, name, 256);
      strncpy (pd->s_comment, comment, 256);
      pd->s_gid = (int) gid;
      pd->s_uid = (int) uid;
      pd->s_mask = (int) mask;
      pd->s_ready = (int) f_ready;
      pd->s_sign = (int) f_sign;
      pd->s_close = (int) f_close;
      //ret = start_async_proc (proc_search, pd);

      //-----------
      LOGD ("proc_search(%s)", pd->s_path);
      
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
      free (pd);
      //-----------


    }
  make_responce (response, ret, strerror (errno), o);
}
