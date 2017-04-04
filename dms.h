/* 
 * File:   dms.h
 * Author: alexcon
 *
 * Created on 29 Март 2013 г., 12:51
 */

#ifndef DMS_H
#define	DMS_H

#ifdef	__cplusplus
extern "C" {
#endif
#define OK(x) (access(x, F_OK))

    typedef enum {
        Renamed, Deleted, Recovered, Added, Chowned, Chgrped, ChUsrAcled, ChGrpAcled, Opened, Replaced, Closed, UnClosed, Copy, Moved, PutArchiveCopyToUser, Printed
    } LOGMSG;

    typedef enum {
        FSOP_Read,
        FSOP_Write,
        FSOP_Link,
        FSOP_Copy,
        FSOP_Move,
        FSOP_Paste,
        FSOP_Recover,
        FSOP_ClearRecycle,
        FSOP_MakeBook,
        FSOP_DeleteBook,
        FSOP_MakeProject,
        FSOP_DeleteProject,
        FSOP_MakeDir,
        FSOP_DeleteDir,
        FSOP_MakeFile,
        FSOP_DeleteFile,
        FSOP_ChangeAcl,
        FSOP_ChangeDate, // изм. даты срока выполнения
        FSOP_ChangeDateRev, // изм. даты ревизии в архиве для проекта
        FSOP_ChangeSign, // изм. флагов готов, подписан
        FSOP_ChangeNums, // изм. полей процент, листы, часы (владелец)
        FSOP_Rename,
        FSOP_RenameBook,
        FSOP_Close,
        FSOP_UnClose,
        FSOP_PrintFSO,
        FSOP_PrintAdd,
        FSOP_PrintDel,
        FSOP_PrintSign,
        FSOP_PrintReady,
        FSOP_PrintFoul,
        FSOP_PrintUrgent,
        FSOP_MakeTemplate

    } FSOP;

#define ACL_TAG "tag"
#define ACL_QUALIFIER "qualifier"
#define ACL_PERMSET "permset"
#define MAXLISTLEN 10000
#define MAXLOGSIZE 30



#define A_XCOMMENT "comment"
#define A_XDATE1 "date1"
#define A_XDATE2 "date2"
#define A_XSIGN "sign"
#define A_XREADY "ready"
#define A_XCLOSE "close"
#define A_XLIST "list"
#define A_XELAPSE "elapse"
#define A_XPROGRESS "progress"
#define A_XCOMMON "common"
#define A_XFLAGS "flags"
#define A_XTYPE "type"
#define A_XNAME "name"

#define A_XBIN "user.xbin"
#define A_RPATH "user.rpath"
#define A_PRINTJOB "user.printjob"
#define A_LOG "user.log"

#define A_FLAG_ALL      0xFFFF
#define A_FLAG_NULL     0x0
#define A_FLAG_PROJECT  0x1
#define A_FLAG_BOOK     0x2
#define A_FLAG_BASKET   0x4
#define A_FLAG_SCAN     0x8
#define A_FLAG_SIGN     0x100
#define A_FLAG_READY    0x200
#define A_FLAG_CLOSE    0x400

#define A_COMMENT_MAX 256
#define A_COMMON_MAX 512
#define A_NAME_MAX 256
#define LOWORD(x) (x & 0xFFFF)
#define HIWORD(x) (x >> 16)
#define LOBYTE(x) (x & 0xff)
#define HIBYTE(x) (x >> 8)
#define IS_SIGN(x) (x & A_FLAG_SIGN)
#define IS_READY(x) (x & A_FLAG_READY)
#define IS_CLOSE(x) (x & A_FLAG_CLOSE)
#define IS_BOOK(x) (x & A_FLAG_BOOK)
#define IS_PROJECT(x) (x & A_FLAG_PROJECT)
#define IS_SCAN(x) (x & A_FLAG_SCAN)
#define IS_BASKET(x) (x & A_FLAG_BASKET)
#define IS_DTYPE(x, y) (x & (y >> 12))

typedef struct xattr {
        char x_name[A_NAME_MAX];
        char x_comment[A_COMMENT_MAX];
        char x_common[A_COMMON_MAX];
        unsigned int x_date_begin;
        unsigned int x_date_end;
        unsigned int x_elapse;
        unsigned short x_list;
        unsigned short x_progress;
        unsigned short x_flags;
    } xattr_t;
#define A_TYPE_BOOK "book"
#define A_TYPE_PROJECT "project"

//#define MAIL_DIR "/monolit.back/storage/mail"
//#define PACKT_DIR "/monolit.back/storage/packts"
    
//#define PATH_GET "\\\\debian\\temp\\dms\\%ld\\%s"

    typedef struct proc_data {
        int s_id;
        char s_path[1024];
        int s_gid;
        int s_uid;
        char s_name[256];
        char s_comment[256];
        int s_mask;
        int s_close;
        int s_sign;
        int s_ready;
    } proc_data_t;
    
#ifdef	__cplusplus
}
#endif

#endif	/* DMS_H */

