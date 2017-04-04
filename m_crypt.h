/* 
 * File:   m_crypt.h
 * Author: alexcon
 *
 * Created on 29 Март 2013 г., 12:37
 */

#ifndef M_CRYPT_H
#define	M_CRYPT_H

#ifdef	__cplusplus
extern "C" {
#endif
char *m_decrypt (char *buff, int* size);
char *m_crypt (char *buff, int *rlen);
#ifdef	__cplusplus
}
#endif

#endif	/* M_CRYPT_H */

