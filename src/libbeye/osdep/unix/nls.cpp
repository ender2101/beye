/**
 * @namespace   libbeye
 * @file        libbeye/osdep/unix/nls.c
 * @brief       This file contains implementation of cyrillic codepages support
 *              for KOI8-R designed by Chernov.
 * @version     -
 * @remark      this source file is part of Binary EYE project (BEYE).
 *              The Binary EYE (BEYE) is copyright (C) 1995 Nickols_K.
 *              All rights reserved. This software is redistributable under the
 *              licence given in the file "Licence.en" ("Licence.ru" in russian
 *              translation) distributed in the BEYE archive.
 * @note        Requires POSIX compatible development system
 *
 * @author      Konstantin Boldyshev
 * @since       1999
 * @note        Development, fixes and improvements
 * @author      Alexander Krisak and Andrew Golovnia
 * @date        23.07.2003
 * @note        Russian locales support: KOI-8, CP866, CP1251, ISO8859-5.
 *              Tested at ASPLinux 7.3 and ASPLinux 9
**/

/*
    Copyright (C) 1999-2001 Konstantin Boldyshev <konst@linuxassembly.org>

    $Id: nls.c,v 1.11 2009/09/20 14:37:19 nickols_k Exp $
*/

#ifndef lint
static const char copyright[] = "$Id: nls.c,v 1.11 2009/09/20 14:37:19 nickols_k Exp $";
#endif
#include <errno.h>
#include <stdlib.h>
#include <stddef.h>
#include "libbeye/libbeye.h"

int do_nls = 0;

static unsigned char alt2koi[] =
			 { 0xe1, 0xe2, 0xf7, 0xe7, 0xe4, 0xe5, 0xf6, 0xfa,
			   0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0,
			   0xf2, 0xf3, 0xf4, 0xf5, 0xe6, 0xe8, 0xe3, 0xfe,
			   0xfb, 0xfd, 0xff, 0xf9, 0xf8, 0xfc, 0xe0, 0xf1,
			   0xc1, 0xc2, 0xd7, 0xc7, 0xc4, 0xc5, 0xd6, 0xda,
			   0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0,
			   0x90, 0x91, 0x92, 0x81, 0x87, 0xb2, 0xb4, 0xa7,
			   0xa6, 0xb5, 0xa1, 0xa8, 0xae, 0xad, 0xac, 0x83,
			   0x84, 0x89, 0x88, 0x86, 0x80, 0x8a, 0xaf, 0xb0,
			   0xab, 0xa5, 0xbb, 0xb8, 0xb1, 0xa0, 0xbe, 0xb9,
			   0xba, 0xb6, 0xb7, 0xaa, 0xa9, 0xa2, 0xa4, 0xbd,
			   0xbc, 0x85, 0x82, 0x8d, 0x8c, 0x8e, 0x8f, 0x8b,
			   0xd2, 0xd3, 0xd4, 0xd5, 0xc6, 0xc8, 0xc3, 0xde,
			   0xdb, 0xdd, 0xdf, 0xd9, 0xd8, 0xdc, 0xc0, 0xd1,
			   0xb3, 0xa3, 0x99, 0x98, 0x93, 0x9b, 0x9f, 0x97,
			   0x9c, 0x95, 0x9e, 0x96, 0xbf, 0x9d, 0x94, 0x9a
			 };
static unsigned char alt2win[] =
			 { 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
			   0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
			   0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
			   0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
			   0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
			   0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
			   0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
			   0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
			   0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
			   0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
			   0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
			   0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
			   0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
			   0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
			   0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
			   0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf
			 };
unsigned char alt2iso[] =
			 { 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
			   0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
			   0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
			   0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
			   0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
			   0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
			   0x82, 0x83, 0x84, 0x85, 0x86, 0x00, 0x00, 0x00,
			   0x00, 0x87, 0x88, 0x89, 0x8a, 0x00, 0x00, 0x8b,
			   0x8c, 0x8d, 0x8e, 0x8f, 0x90, 0x91, 0x00, 0x00,
			   0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x00,
			   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			   0x00, 0x9a, 0x9b, 0x9c, 0x9d, 0x00, 0x00, 0x9E,
			   0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
			   0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
			   0xa1, 0xf1, 0xa4, 0xf4, 0xa7, 0xf7, 0xae, 0xfe,
			   0x00, 0x00, 0x00, 0x00, 0xf0, 0x99, 0x9f, 0xa0
			 };

void __FASTCALL__ __nls_OemToOsdep(unsigned char *buff, unsigned int len)
{
    if (do_nls) __nls_OemToFs(buff, len);
}

void __FASTCALL__ __nls_OemToFs(unsigned char *buff, unsigned int len)
{
    unsigned int i;
    static char* loc = NULL;
    static unsigned char* code = NULL;

    if ((loc == NULL) && ((loc = getenv("LANG")) != NULL))
    { if ((strcmp(loc, "ru_RU.KOI8-R") == 0) ||
	  (strcmp(loc, "ru_UA.KOI8-U") == 0))
	code = alt2koi;
      else if ((strcmp(loc, "ru_RU.CP1251") == 0) ||
	       (strcmp(loc, "ru_UA.CP1251") == 0))
	code = alt2win;
      else if ((strcmp(loc, "ru_RU.ISO88595") == 0) ||
	       (strcmp(loc, "ru_UA.ISO88595") == 0) ||
	       (strcmp(loc, "ru_RU") == 0) ||
	       (strcmp(loc, "ru_UA") == 0))
	code = alt2iso;
    }

    if (code != NULL)
    {
	for (i = 0; i < len; i++)
	    if (buff[i] >= 0x80) buff[i] = __Xlat__((const uint8_t*)code, buff[i] - 0x80);
    }
}

void __FASTCALL__ __nls_CmdlineToOem(unsigned char *buff, unsigned int len)
{
    UNUSED(buff), UNUSED(len);
}

#ifdef HAVE_ICONV
#include <iconv.h>
#endif

static const char * langs[] = { "LANG", "LANGUAGE", "LINGUAS" };

char *nls_get_screen_cp(void)
{
    unsigned i;
    char *nls;
    static char to_cp[256];
    strcpy(to_cp,"UTF-8");
    for(i=0;i<sizeof(langs)/sizeof(char *);i++)
    {
	if((nls=getenv(langs[i]))!=NULL)
	{
		nls=strchr(nls,'.');
		if(nls) strcpy(to_cp,nls+1);
		break;
	}
    }
    return to_cp;
}

any_t* nls_init(const char *to_cp,const char *src_cp) {
#ifdef HAVE_ICONV
    iconv_t ic;
    errno=0;
    ic=iconv_open(to_cp,src_cp);
    if(errno) {
	printm("ICONV(%s,%s): Open with error: %s\n",to_cp,src_cp,strerror(errno));
    }
    return ic;
#else
    return NULL;
#endif
}

void nls_term(any_t* ic) {
#ifdef HAVE_ICONV
 iconv_close(ic);
#endif
}

char *nls_recode2screen_cp(any_t* ic,const char *srcb,unsigned* len)
{
    char *obuff;
#ifdef HAVE_ICONV
    if(ic)
    {
	static int warned=0;
	const char *ibuff,*ib;
	char *ob;
	size_t inb,outb;
	errno=0;
	inb=*len;
	outb=((*len)+1)*4;
	obuff=new char[outb];
	ibuff=srcb;
	ob=obuff;
	ib=ibuff;
	if(iconv(ic,(char **)&ib,&inb,&ob,&outb) != (size_t)(-1))
	{
	    *ob='\0';
	    *len = ((*len)+1)*4 - outb;
	}
	else
	{
	    delete obuff;
	    if(warned<2) {
		printm("ICONV: Can't recode: %s\n",strerror(errno));
		warned++;
	    }
	    goto do_def;
	}
    }
    else
    {
	do_def:
	obuff=strdup(srcb);
    }
#else
    obuff=strdup(srcb);
#endif
    return obuff;
}

int nls_test(any_t* ic,const char *srcb,unsigned* len)
{
#ifdef HAVE_ICONV
    if(ic)
    {
	const char *ibuff,*ib;
	char *obuff;
	char *ob;
	size_t inb,outb,rval;
	errno=0;
	inb=*len;
	outb=((*len)+1)*4;
	obuff=new char[outb];
	ibuff=srcb;
	ob=obuff;
	ib=ibuff;
	rval=iconv(ic,(char **)&ib,&inb,&ob,&outb);
	delete obuff;
	if(iconv(ic,(char **)&ib,&inb,&ob,&outb) != (size_t)(-1))
	    return 0;
	else return errno;
    }
#endif
    return 0;
}