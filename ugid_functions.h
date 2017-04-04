/* 
 * File:   ugid_functions.h
 * Author: alexcon
 *
 * Created on 28 Март 2013 г., 14:28
 */

#ifndef UGID_FUNCTIONS_H
#define	UGID_FUNCTIONS_H

#ifdef	__cplusplus
extern "C" {
#endif
/* ugid_functions.h

   Header file for ugid_functions.c.
*/

#include "tlpi_hdr.h"

char *userNameFromId(uid_t uid);
uid_t userIdFromName(const char *name);
char *groupNameFromId(gid_t gid);
gid_t groupIdFromName(const char *name);

#ifdef	__cplusplus
}
#endif

#endif	/* UGID_FUNCTIONS_H */

