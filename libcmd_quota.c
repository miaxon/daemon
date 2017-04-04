#include <stdio.h>
#include <stdlib.h>
#include <acl/libacl.h>
#include <sys/acl.h>

#include <time.h>
#include <pthread.h>
#include <locale.h>
#include <dlfcn.h>
#include <sys/quota.h>
#include "libcmd.h"
#include "dms.h"


void
_QuotaGet (json_t *request, json_t * response)
{
  LOG("_QuotaGet():");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *dev = json_string_value (json_array_get (params_object, 0));
  json_int_t uid = json_integer_value (json_array_get (params_object, 1));
  json_t *o = get_ret_object ();
  struct dqblk dq;
  int ret = quotactl (QCMD (Q_GETQUOTA, USRQUOTA), dev, uid, (caddr_t) & dq);
  if (ret == 0)
    {
      json_t *jqouta = json_object ();
      json_object_set_new (jqouta, "use", json_integer (dq.dqb_curspace));
      json_object_set_new (jqouta, "limit", json_integer (dq.dqb_bhardlimit));
      json_object_set_new (o, "quota", jqouta);
    }
  make_responce (response, ret, strerror (errno), o);
}

void
_QuotaSet (json_t *request, json_t * response)
{
  LOG("_QuotaSet():");
  json_t *params_object = json_object_get (request, PARAMS);
  const char *dev = json_string_value (json_array_get (params_object, 0));
  json_int_t uid = json_integer_value (json_array_get (params_object, 1));
  json_int_t limit = json_integer_value (json_array_get (params_object, 2));
  json_t *o = get_ret_object ();
  struct dqblk dq;
  int ret = quotactl (QCMD (Q_GETQUOTA, USRQUOTA), dev, uid, (caddr_t) & dq);
  if (ret == 0)
    {
      dq.dqb_bhardlimit = dq.dqb_bsoftlimit = limit;
      ret = quotactl (QCMD (Q_SETQUOTA, USRQUOTA), dev, uid, (caddr_t) & dq);
    }
  make_responce (response, ret, strerror (errno), o);
}
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        