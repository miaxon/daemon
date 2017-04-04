
#include "libcmd.h"


typedef void (*fns_t) (json_t *, json_t *);
fns_t fns[] = {
  _Shell,
  _UserAdd,
  _FindByInum,
  _GetFSO,
  _GetFSOTemp,
  _PutFSOTemp,
  _ExistsFSO,
  _RemoveFSO,
  _DeleteFSO,
  _RecoverFSO,
  _RenameFSO,
  _MoveFSO,//11

  _DirMake,
  _DirRead,//13

  _XAttrGetValue,
  _XAttrSetValue,
  _XAttrGetLog,
  _XAttrClearLog,
  _XAttrGetPrintJob,
  _XAttrSetPrintJob,//19

  _XFlagsSet,
  _XListSet,
  _XProgressSet,
  _XElapseSet,
  _XDateBeginSet,
  _XDateEndSet,
  _XCommentSet,
  _XCommonSet,
  _XNameSet,//28

  _AclGet,
  _AclSet,
  _AclClear,
  _AclChown,
  _AclChmod,
  _AclAddEntry,
  _AclRemoveEntry,
  _AclModifyEntry,//36

  _QuotaGet,
  _QuotaSet,
  _UtilClose,
  _UtilUnClose,
  _UtilDuplicateFSO,
  _UtilDirEmpty,
  _UtilMakeTmplate,
  _PrintPutFSO,
  _PrintPut,
  _AsyncSearch,
  _PutArchiveCopyToUser,
  _UtilCacheXattr,
  _UtilGetThumb,
  _GetInUseInfo//50
};

json_t*
get_ret_object (void)
{
  json_t *o = json_object ();
  json_t *retstr = json_string (DEFSTR);
  json_t *retint = json_integer (DEFINT);
  json_object_set_new (o, RETSTR, retstr);
  json_object_set_new (o, RETINT, retint);
  return o;
}

void
make_responce (json_t *response, int retval, char *reterr, json_t * retobj)
{
  json_object_set_new (response, RETVAL, json_integer (retval));
  json_object_set_new (response, RETERR, json_string (reterr));
  json_object_set_new (response, RETOBJ, retobj);
}

void
jsonrpc_service (json_t *request, json_t *response)
{
  json_t *metod_object = json_object_get (request, METHOD);
  int method = json_integer_value (metod_object);
  (*fns[method])(request, response);
}

json_t *
get_error (json_error_t *err)
{
  json_t *e = json_object ();
  json_object_set_new (e, RETSTR, json_string (err->text));
  json_object_set_new (e, RETINT, json_integer (-1));
  return e;
}

char*
jsonrpc_process (char* request, int *rlen)
{
  request = m_decrypt (request, rlen);
  if (*rlen == -1)
    return NULL;
  LOG ("decrypt request: %s", request);
  char* response;
  json_error_t jerror;
  json_t *jrequest;
  json_t *jresponse = json_object ();
  if (!jresponse)
    {
      char *errstr;
      asprintf (&errstr, MUNKNOWN);
      char * cerrstr = m_crypt (errstr, rlen);
      free (errstr);
      return cerrstr;
    }
  jrequest = json_loads (request, 0, &jerror);
  if (!jrequest)
    {
      LOG("jerror:%s", jerror.text);
      make_responce (jresponse, -1, jerror.text, json_object ());
      char *errstr = json_dumps (jresponse, 0);
      char * cerrstr = m_crypt (errstr, rlen);
      free (errstr);
      return cerrstr;
    }
  seteuid (0);
  jsonrpc_service (jrequest, jresponse);
  seteuid (currentUid);
  response = json_dumps (jresponse, JSON_COMPACT);
  char *cresponse = m_crypt (response, rlen);  
  free (response);
  json_object_clear (jrequest);
  json_object_clear (jresponse);
  json_decref (jrequest);
  json_decref (jresponse);
  return cresponse;
}

