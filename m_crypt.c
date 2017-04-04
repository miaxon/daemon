#include <mcrypt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "libcmd.h"
/* #include <mhash.h> */
char MKEY[32] = "c48bfd62c1f32d481a6e0e005516df0a";
int MKEY_SIZE = 32;
int MBLOCK_SIZE = 32;
int MIV_SIZE = 32;

char *
m_decrypt (char *buff, int* size)
{  
  MCRYPT mfd;
  int i;
  mfd = mcrypt_module_open (MCRYPT_RIJNDAEL_256, NULL, MCRYPT_CBC, NULL);
  if (mfd == MCRYPT_FAILED)
    {
      *size = -1;
      return buff;
    }  
  int len = *size - MIV_SIZE;  
  i = mcrypt_generic_init (mfd, MKEY, MKEY_SIZE, buff);
  if (i < 0)
    {
      *size = -1;
      mcrypt_module_close (mfd);
      return buff;
    }
  
  //LOG ("crypt public key: ");
 // for (i = 0; i < MIV_SIZE; i++)
 //   LOG ("%02hhx", buff[i]);
  
  mdecrypt_generic (mfd, &buff[MIV_SIZE], len);
  mcrypt_generic_deinit (mfd);
  mcrypt_module_close (mfd);
  //LOG(&buff[MIV_SIZE]);
  //memcpy (buff, &buff[MIV_SIZE], len);
  //buff[len] = 0;
  //LOG(buff);
  //int blocks = (int) (len / MBLOCK_SIZE);
  //LOG ("blocks: %d, len: %d\n", blocks, len);

  return &buff[MIV_SIZE];
}

char *
m_crypt (char *buff, int *rlen)
{
  MCRYPT mfd;
  int i, mlen;
  char *str;
  mlen = strlen (buff);
  int blocks = (int) (mlen / MBLOCK_SIZE) + 1;
  int buf_size = blocks * MBLOCK_SIZE;

  mfd = mcrypt_module_open (MCRYPT_RIJNDAEL_256, NULL, MCRYPT_CBC, NULL);
  if (mfd == MCRYPT_FAILED)
    {
      *rlen = -1;
      return NULL;
    }
  int size = buf_size + MIV_SIZE;
  str = (char*) malloc (size + 1);
  if (str == NULL)
    {
      *rlen = -1;
      mcrypt_module_close (mfd);
      return NULL;
    }
  memset (str, 0, size);
  srand (time (0) + rand ());
  for (i = 0; i < MIV_SIZE; i++)
    str[i] = rand ();
//#ifndef VFS_FIFO 
//  printf ("crypt public key: ");
//  for (i = 0; i < MIV_SIZE; i++)
 //   printf ("%02hhx", str[i]);
//  printf ("\n");
//#endif
  strncpy (&str[MIV_SIZE], buff, mlen);
  i = mcrypt_generic_init (mfd, MKEY, MKEY_SIZE, str);
  if (i < 0)
    {
      *rlen = -1;
      mcrypt_module_close (mfd);
      return NULL;
    }
  mcrypt_generic (mfd, &str[MIV_SIZE], buf_size);
  mcrypt_generic_deinit (mfd);
  mcrypt_module_close (mfd);
  *rlen = size;
//#ifndef VFS_FIFO 
//  printf ("size: %d , blocks: %d\n", size, blocks);
//#endif
  return str;
}
