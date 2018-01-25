#ifndef  _HASH_FUNCTIONS_H_
#define  _HASH_FUNCTIONS_H_
#include "os_include.h"

#ifdef __cplusplus
extern "C" {
#endif


unsigned int crc(unsigned char *buf,int len);			// (881)
unsigned int RSHash(char *str,unsigned int len);		// (951)	+
unsigned int JSHash(char *str,unsigned int len);		// (861)
unsigned int PJWHash(char *str,unsigned int len);		// (1001)
unsigned int ELFHash(char *str,unsigned int len);		// (961)
unsigned int BKDRHash (char *str,unsigned int len);		// (921)
unsigned int SDBMHash(char *str,unsigned int len);		// (881)	+
unsigned int DJBHash(char *str,unsigned int len);		// (841)
unsigned int DEKHash(char *str,unsigned int len);		// (851)
unsigned int APHash(char *str,unsigned int len);		// (941)
unsigned int LYHash(char *str,unsigned int len);		// (921)
unsigned int ROT13Hash(char *str,unsigned int len);		// (912)
unsigned int FAQ6hash(char *str,unsigned int len);		// (991)	+
unsigned int LOOKUP3_hash(char *str,unsigned int len8);	// (861)

//#define mvi_hash(str,len,key)	key += str[0] + len;

#define mvi_hash(str,len,key)	key += FAQ6hash(str,len);

#ifdef __cplusplus
}
#endif


#endif /* _HASH_FUNCTIONS_ */ 
