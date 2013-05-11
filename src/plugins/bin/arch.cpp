#include "config.h"
#include "libbeye/libbeye.h"
using namespace	usr;
/**
 * @namespace	usr_plugins_auto
 * @file        plugins/bin/arch.c
 * @brief       This file contains implementation of Archive file format decoder.
 * @version     -
 * @remark      this source file is part of Binary EYE project (BEYE).
 *              The Binary EYE (BEYE) is copyright (C) 1995 Nickols_K.
 *              All rights reserved. This software is redistributable under the
 *              licence given in the file "Licence.en" ("Licence.ru" in russian
 *              translation) distributed in the BEYE archive.
 * @note        Requires POSIX compatible development system
 *
 * @author      Nickols_K
 * @since       1999
 * @note        Development, fixes and improvements
**/
#include <algorithm>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "libbeye/bswap.h"
#include "bmfile.h"
#include "bin_util.h"
#include "beyehelp.h"
#include "bconsole.h"
#include "beyeutil.h"
#include "reg_form.h"
#include "tstrings.h"
#include "plugins/bin/arch.h"
#include "plugins/disasm.h"
#include "libbeye/libbeye.h"
#include "libbeye/kbd_code.h"

namespace	usr {
ar_hdr arch;

static __filesize_t __FASTCALL__ ShowARCHHeader()
{
  __filesize_t fpos;
  unsigned evt;
  TWindow * w;
  struct tm * tm;
  time_t ldat;
  char sout[50];
  fpos = BMGetCurrFilePos();
  w = CrtDlgWndnls(" This is COFF or a.out archive ",54,6);
  w->goto_xy(1,1);
  strncpy(sout,(char *)arch.ar_name,16);
  sout[16] = 0;
  w->printf("Name           = %s\n",sout);
  strncpy(sout,(char *)arch.ar_date,12);
  sout[12] = 0;
  ldat = atol(sout);
  tm = localtime(&ldat);
  strftime(sout,sizeof(sout),"%X %x",tm);
  w->printf("Date           = %s\n",sout);
  strncpy(sout,(char *)arch.ar_uid,6);
  sout[6] = 0;
  w->printf("Owner UID      = %s\n",sout);
  strncpy(sout,(char *)arch.ar_gid,6);
  sout[6] = 0;
  w->printf("Owner GID      = %s\n",sout);
  strncpy(sout,(char *)arch.ar_mode,8);
  sout[8] = 0;
  w->printf("File mode      = %s\n",sout);
  strncpy(sout,(char *)arch.ar_size,10);
  sout[10] = 0;
  w->printf("File size      = %s bytes",sout);
  do
  {
    evt = GetEvent(drawEmptyPrompt,NULL,w);
  }
  while(!(evt == KE_ESCAPE || evt == KE_F(10)));
  delete w;
  return fpos;
}

static bool  __FASTCALL__ archReadModList(memArray *obj,unsigned nnames,__filesize_t *addr)
{
  __filesize_t foff,flen;
  unsigned i;
  char stmp[80];
  flen = bmGetFLength();
  for(i = 0;i < nnames;i++)
  {
    bool is_eof;
    /**
       Some archives sometimes have big and sometimes little endian.
       Here is a horrible attempt to determine it.
    */
    foff = addr[i];
    if(foff > flen)  foff = be2me_32(foff);
    if(IsKbdTerminate()) break;
    bmReadBufferEx(stmp,sizeof(ar_sub_hdr),foff,binary_stream::Seek_Set);
    is_eof = bmEOF();
    stmp[sizeof(ar_sub_hdr)-2] = 0;
    if(!ma_AddString(obj,is_eof ? CORRUPT_BIN_MSG : stmp,true)) break;
    if(is_eof) break;
  }
  return true;
}

static __filesize_t __FASTCALL__ archModLst()
{
   memArray *obj;
   __filesize_t *addr;
   unsigned long rnames,bnames;
   unsigned nnames;
   __filesize_t fpos,flen;
   fpos = BMGetCurrFilePos();
   flen = bmGetFLength();
   rnames = bmReadDWordEx(sizeof(ar_hdr),binary_stream::Seek_Set);
   bnames = be2me_32(rnames);
   /**
      Some archives sometimes have big and sometimes little endian.
      Here is a horrible attempt to determine it.
   */
   if(!(nnames = (unsigned)std::min(rnames,bnames))) { beye_context().NotifyBox(NOT_ENTRY,"Archive modules list"); return fpos; }
   /**
      Some archives sometimes have length and sometimes number of entries
      Here is a horrible attempt to determine it.
   */
   if(!(nnames%4)) nnames/=sizeof(unsigned long);
   if(!(obj = ma_Build(nnames,true))) return fpos;
   if(!(addr = new __filesize_t [nnames])) goto exit;
   bmReadBufferEx(addr,sizeof(unsigned long)*nnames,sizeof(ar_hdr)+sizeof(unsigned long),binary_stream::Seek_Set);
   if(archReadModList(obj,nnames,addr))
   {
     int ret;
     ret = ma_Display(obj," Archive modules list ",LB_SELECTIVE,-1);
     if(ret != -1)
     {
       /**
	  Some archives sometimes have big and sometimes little endian.
	  Here is a horrible attempt to determine it.
       */
       fpos = addr[ret];
       if(fpos > flen) fpos = be2me_32(fpos);
       fpos += sizeof(ar_sub_hdr);
     }
   }
   delete addr;
   exit:
   ma_Destroy(obj);
   return fpos;
}

static bool __FASTCALL__ IsArch()
{
  char str[16];
  bmReadBufferEx(str,sizeof(str),0,binary_stream::Seek_Set);
  return strncmp(str,"!<arch>\012",8) == 0;
}

static void __FASTCALL__ ArchInit(CodeGuider& code_guider)
{
    UNUSED(code_guider);
    bmReadBufferEx(&arch,sizeof(arch),0,binary_stream::Seek_Set);
}

static void __FASTCALL__ ArchDestroy()
{
}

static bool __FASTCALL__ archAddrResolv(char *addr,__filesize_t cfpos)
{
 /* Since this function is used in references resolving of disassembler
    it must be seriously optimized for speed. */
  bool bret = true;
  if(cfpos < sizeof(ar_hdr))
  {
    strcpy(addr,"arch.h:");
    strcpy(&addr[7],Get2Digit(cfpos));
  }
  else bret = false;
  return bret;
}

static __filesize_t __FASTCALL__ archHelp()
{
  hlpDisplay(10001);
  return BMGetCurrFilePos();
}

static int __FASTCALL__ arch_platform() { return DISASM_DEFAULT; }

extern const REGISTRY_BIN archTable =
{
  "arch (Archive)",
  { "ArcHlp", NULL, "ModLst", NULL, NULL, NULL, NULL, NULL, NULL, NULL },
  { archHelp, NULL, archModLst, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
  IsArch, ArchInit, ArchDestroy,
  ShowARCHHeader,
  NULL,
  arch_platform,
  NULL,
  NULL,
  archAddrResolv,
  NULL,
  NULL,
  NULL,
  NULL
};
} // namespace	usr
