/* 
 * File:   libcmd_sys.h
 * Author: alexcon
 *
 * Created on 29 Март 2013 г., 12:57
 */

#ifndef LIBCMD_SYS_H
#define	LIBCMD_SYS_H
#include "dms.h"
#include <ftw.h>
#ifdef	__cplusplus
extern "C" {
#endif
#define ACL_EA_ACCESS		"system.posix_acl_access"
#define ACL_EA_DEFAULT		"system.posix_acl_default"  
#define ACL_EA_SIZE 134 // 16 acl_entry_t (8 byte each) + 4 byte (int version header)
    
    int xname_set(const char *path, const char *val);
    int xflag_check(struct xattr *xa, unsigned short f);
    void xflag_set(struct xattr *xa, unsigned short f);
    void xflag_unset(struct xattr *xa, unsigned short f);
    int get_xattr(const char *path, struct xattr *xa);
    int set_xattr(const char *path, struct xattr *xa);

    struct proc_data *get_proc_data(void);
    void send_event(json_t *event_object);
    json_t *get_async_jobject(int id);
    void *proc_search(void *parm);
    //
    int str_equal(const char* str1, const char *str2);
    const char *path_parent(const char *path, char *buf);
    const char *path_name(const char *path);
    int clear_dir(const char* path);
    int close_dir(const char* path);
    int setacl_from_file(const char *src, const char *dst);
    int mvf(const char* src, const char* dst);
    int inherit_perms(const char *src, const char *dst);
    int ftw_setAcl(const char *pathname, const struct stat *sbuf, int type, struct FTW *ftwb);

    void jfree(json_t *o);
    void logmsg(const char *path, LOGMSG act, const char *parm);
    char *m_crypt(char *m, int *rlen);
    char *m_decrypt(char *m, int *rlen);
    int setup_tmp_fd(void);
    int shell_internal(const char *command, int *outfd);
    char* read_tmp_fd(int outFd);
    int copyFile(const char *path1, const char *path2);
    void shell_cmd(json_t *response, char *cmd);
    unsigned long int GetInode(const char *path);
    int IsRegularFile(const char *path);
    int IsDirectory(const char *path);
    int IsDirectoryEmpty(const char *path);    
    int IsDirHasSubDirs(const char *path);
    int IsLink(const char *path);
    int acl_count(const char* path, const char* name);
    int clearAcl(const char* path, int acl_type);
    json_t *get_jxattr(const char* path);
    json_t *get_jacl(const char* path, int acl_type);
    int set_jacl(json_t *ent_array, const char *path, int acl_type);
    json_t *get_jstat(const const char* path, const char *vname);
    int set_jxattr(const char *path, json_t *jxattr);
    int acl_modify_entry(const char* path, int tag, int qualifier, int permset, int acl_type);
    int acl_remove_entry(const char* path, int tag, int qualifier, int acl_type);
    int acl_add_entry(const char* path, int tag, int qualifier, int permset, int acl_type);
    void set_mode(const char *path, mode_t m);
    int xattr_equal(const char *path, const char *attr_name, const char *attr_value);
    json_t *get_jchild(const char *path, int mask, int flags);
    void make_index(const char* path, const char *name);
    int get_mode(const char *str);
    int cpax(const char* src, const char* dst);
    int cprx(const char* src, const char* dst);
    int rmrf(const char* path);
    int chmodR(const char *path, mode_t mode);
    int chownR(const char *path, const char *user, const char *group);
    char* get_fso(int op, const char *path);
    void frees(void *ptr);
    int log_echo (const char *path, char *str, int max_size);
    int get_xattr_from_cache(const char *path);
    void cache_xattr(const char *path);
    int IsMSDoc(const char *path);
#ifdef	__cplusplus
}
#endif

#endif	/* LIBCMD_SYS_H */

