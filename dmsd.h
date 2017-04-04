/* 
 * File:   dmsd.h
 * Author: alexcon
 *
 * Created on 29 Декабрь 2012 г., 9:16
 */

#ifndef DMSD_H
#define	DMSD_H

#ifdef	__cplusplus
extern "C" {
#endif
#include <stdlib.h>
#include <stdio.h>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>
#include <dlfcn.h>
#include <ftw.h>
#include <limits.h>
#include "tlpi_hdr.h"
#define PIPE_BUFSZE 4096
#define BUF_REQ_SIZE 1024*1024
#define BUF_FIFO_SIZE 4096*16
#define CONFIG_FILE "/etc/samba/dmsd.conf"

    typedef struct start_param {
        char userName[32];
        char hostName[32];
        char confFile[1024];
    } start_param_t;

    typedef struct conf_param {
        char pipePortLocal[1024];
        char pipePortRemote[1024];
        char pipeIpcRemote[1024];
        char pipePortEvent[1024];
        char pipeLockSem[1024];
        char pipeAuthCreds[1024];
        char pidFile[1024];
        char tdir[1024];
        char tfile[1024];
        char tpath[1024];
    } conf_param_t;

    typedef struct cred_struct {
        char username[32];
        char workgroup[32];
        char password[32];
        int init;
    } cred_struct_t;
    conf_param_t *cfp_mem;
    struct conf_param *get_cfp();
    cred_struct_t *get_crd();
    void cleanupHandler(void *arg);
    pid_t currentPid;
    uid_t currentUid;
    char currentUser[32];
    BOOL configureSignalHandlers();
    BOOL become_user(const char* name);
    BOOL unbecome_user();
    BOOL load_conf(const char *confFile, struct conf_param *cfp, struct start_param *sp);
    void init_pipe_local(struct conf_param *cfp);
    int read_fifo(const char* name);
    BOOL write_pipe(const char* name, const char* sem_name, char *data, int rlen);
    void panic (char *msg);

#ifdef	__cplusplus
}
#endif

#endif	/* DMSD_H */

