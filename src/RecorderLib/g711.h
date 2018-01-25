#pragma once
/*
* This source code is a product of Sun Microsystems, Inc. and is provided
* for unrestricted use.  Users may copy or modify this source code without
* charge.
*
* SUN SOURCE CODE IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING
* THE WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
* PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
*
* Sun source code is provided with no support and without any obligation on
* the part of Sun Microsystems, Inc. to assist in its use, correction,
* modification or enhancement.
*
* SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
* INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY THIS SOFTWARE
* OR ANY PART THEREOF.
*
* In no event will Sun Microsystems, Inc. be liable for any lost revenue
* or profits or other special, indirect and consequential damages, even if
* Sun has been advised of the possibility of such damages.
*
* Sun Microsystems, Inc.
* 2550 Garcia Avenue
* Mountain View, California  94043
*/

unsigned char linear2alaw(int pcm_val);  /* 2's complement (16-bit range) */
int alaw2linear(unsigned char a_val);	// UK
unsigned char linear2ulaw(int pcm_val);  /* 2's complement (16-bit range) */
int ulaw2linear(unsigned char u_val); // US
unsigned char alaw2ulaw(unsigned char aval);
unsigned char ulaw2alaw(unsigned char uval);
