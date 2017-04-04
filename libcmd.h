/* 
 * File:   libcmd.h
 * Author: alexcon
 *
 * Created on 29 Март 2013 г., 12:39
 */

#ifndef LIBCMD_H
#define	LIBCMD_H
#define LOG logMessage
#define LOGD logDaemonMessage
#include "dmsd.h"
#include "m_crypt.h"
#include <jansson.h>
#include <jansson_config.h>
#ifdef	__cplusplus
extern "C" {
#endif
#define METHOD "method"
#define PARAMS "parms"
#define RETSTR "retstr"
#define RETINT "retint"
#define RETERR "reterr"
#define RETVAL "retval"
#define RETOBJ "retobj"
#define DEFSTR ""
#define DEFINT 0
#define DECRYPT 1
#define ENCRYPT 0
#define JUNKNOWN "{\"retval\": -1, \"reterr\": \"Ошибка json.\", \"retobj\": {\"retstr\": \"\", \"retint\": 0}}"
#define MUNKNOWN "{\"retval\": -1, \"reterr\": \"Ошибка libmcrypt на сервере.\", \"retobj\": {\"retstr\": \"\", \"retint\": 0}}" 

    char* jsonrpc_process(char* request, int *rlen);
    json_t* get_ret_object (void);
    void _Shell
    (json_t *request, json_t *response);
    void _UserAdd
    (json_t *request, json_t *response);
    void _FindByInum
    (json_t *request, json_t *response);
    void _GetFSO
    (json_t *request, json_t *response);
    void _GetFSOTemp
    (json_t *request, json_t *response);
    void _PutFSOTemp
    (json_t *request, json_t *response);
    void _ExistsFSO
    (json_t *request, json_t *response);
    void _RemoveFSO
    (json_t *request, json_t *response);
    void _DeleteFSO
    (json_t *request, json_t *response);
    void _RecoverFSO
    (json_t *request, json_t *response);
    void _RenameFSO
    (json_t *request, json_t *response);
    void _MoveFSO
    (json_t *request, json_t *response);

    void _DirMake
    (json_t *request, json_t *response);
    void _DirRead
    (json_t *request, json_t *response);

    void _XAttrGetValue
    (json_t *request, json_t *response);
    void _XAttrSetValue
    (json_t *request, json_t *response);
    void _XAttrGetLog
    (json_t *request, json_t *response);
    void _XAttrClearLog 
    (json_t *request, json_t *response);
    void _XAttrGetPrintJob
    (json_t *request, json_t *response);
    void _XAttrSetPrintJob
    (json_t *request, json_t *response);

    void _XFlagsSet
    (json_t *request, json_t *response);
    void _XListSet
    (json_t *request, json_t *response);
    void _XProgressSet
    (json_t *request, json_t *response);
    void _XElapseSet
    (json_t *request, json_t *response);
    void _XDateBeginSet
    (json_t *request, json_t *response);
    void _XDateEndSet
    (json_t *request, json_t *response);
    void _XCommentSet
    (json_t *request, json_t *response);
    void _XCommonSet
    (json_t *request, json_t *response);
    void _XNameSet
    (json_t *request, json_t *response);
    void _AclGet
    (json_t *request, json_t *response);
    void _AclSet
    (json_t *request, json_t *response);
    void _AclClear
    (json_t *request, json_t *response);
    void _AclChown
    (json_t *request, json_t *response);
    void _AclChmod
    (json_t *request, json_t *response);
    void _AclAddEntry
    (json_t *request, json_t * response);
    void _AclRemoveEntry
    (json_t *request, json_t * response);
    void _AclModifyEntry
    (json_t *request, json_t * response);
    void _QuotaGet
    (json_t *request, json_t *response);
    void _QuotaSet
    (json_t *request, json_t *response);
    void _UtilClose
    (json_t *request, json_t *response);
    void _UtilUnClose
    (json_t *request, json_t *response);
    void _UtilDuplicateFSO
    (json_t *request, json_t *response);
    void _UtilDirEmpty
    (json_t *request, json_t *response);
    void _UtilMakeTmplate
    (json_t *request, json_t *response);    
    void _PrintPut
    (json_t *request, json_t * response);
    void _PrintPutFSO
    (json_t *request, json_t * response);
    void _AsyncSearch
    (json_t *request, json_t * response);
    void _PutArchiveCopyToUser
    (json_t *request, json_t * response);
    void _UtilCacheXattr
    (json_t *request, json_t *response);
    void _UtilGetThumb
    (json_t *request, json_t *response);
    void _GetInUseInfo 
    (json_t *request, json_t *response);

#ifdef	__cplusplus
}
#endif

#endif	/* LIBCMD_H */

