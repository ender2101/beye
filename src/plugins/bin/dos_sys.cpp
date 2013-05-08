#include "config.h"
#include "libbeye/libbeye.h"
using namespace	usr;
/**
 * @namespace	usr_plugins_auto
 * @file        plugins/bin/dos_sys.c
 * @brief       This file contains implementation of DOS driver file format.
 * @version     -
 * @remark      this source file is part of Binary EYE project (BEYE).
 *              The Binary EYE (BEYE) is copyright (C) 1995 Nickols_K.
 *              All rights reserved. This software is redistributable under the
 *              licence given in the file "Licence.en" ("Licence.ru" in russian
 *              translation) distributed in the BEYE archive.
 * @note        Requires POSIX compatible development system
 *
 * @author      Nickols_K
 * @since       1995
 * @note        Development, fixes and improvements
**/
#include <string.h>
#include <stdio.h>
#include <stddef.h>

#include "bconsole.h"
#include "bmfile.h"
#include "beyehelp.h"
#include "colorset.h"
#include "reg_form.h"
#include "libbeye/kbd_code.h"
#include "plugins/disasm.h"
#include "plugins/bin/dos_sys.h"

namespace	usr {
static DOSDRIVER drv;

static __filesize_t __FASTCALL__ ShowSysHeader()
{
 int keycode;
 TWindow *hwnd;
 bool charun;
 __fileoff_t fpos;
 fpos = BMGetCurrFilePos();
 hwnd = CrtDlgWndnls(" DOS Device Driver Header ",57,13);
 charun = (drv.ddAttribute & 0x8000) == 0x8000;
 if(charun) hwnd->printf("Device Name               = %8s\n",drv.ddName);
 else       hwnd->printf("Device Supports             %u Block Units\n",(unsigned)drv.ddName[0]);
 hwnd->printf(
	  "Attributes :\n"
	  "  [%c]  STDIN\n"
	  "  [%c]  STDOUT\n"
	  "  [%c]  STDNUL\n"
	  "  [%c]  CLOCK\n"
	  "  [%c]  Support Logical Device\n"
	  "  [%c]  Support OPEN/CLOSE/RM\n"
	  "  [%c]  Non IBM Block-Device\n"
	  "  [%c]  Support IOCTL\n"
	  "  Device is                %s\n"
	 ,Gebool(drv.ddAttribute & 0x0001)
	 ,Gebool(drv.ddAttribute & 0x0002)
	 ,Gebool(drv.ddAttribute & 0x0004)
	 ,Gebool(drv.ddAttribute & 0x0008)
	 ,Gebool(drv.ddAttribute & 0x0040)
	 ,Gebool(drv.ddAttribute & 0x0800)
	 ,Gebool(drv.ddAttribute & 0x2000)
	 ,Gebool(drv.ddAttribute & 0x4000)
	 ,(drv.ddAttribute & 0x8000 ? "CHARACTERS" : "BLOCKS"));
 hwnd->set_color(dialog_cset.entry);
 hwnd->printf(">STRATEGY routine offset  = %04hXH bytes [Enter]",drv.ddStrategyOff);
 hwnd->printf("\n"); hwnd->clreol();
 hwnd->set_color(dialog_cset.altentry);
 hwnd->printf(">INTERRUPT routine offset = %04hXH bytes [Ctrl+Enter | F5]",drv.ddInterruptOff);
 hwnd->clreol();
 while(1)
 {
   keycode = GetEvent(drawEmptyPrompt,NULL,hwnd);
   if(keycode == KE_ENTER) { fpos = drv.ddStrategyOff; break; }
   else
     if(keycode == KE_CTL_ENTER || keycode == KE_F(5)) { fpos = drv.ddInterruptOff; break; }
     else
       if(keycode == KE_ESCAPE || keycode == KE_F(10)) break;
 }
 delete hwnd;
 return fpos;
}


static bool  __FASTCALL__ dossys_check_fmt()
{
  unsigned char id[4];
  bool ret = false;
  bmReadBufferEx(id,sizeof(id),0,BFile::Seek_Set);
  if(id[0] == 0xFF && id[1] == 0xFF && id[2] == 0xFF && id[3] == 0xFF)
  {
     bmReadBufferEx((any_t*)&drv,sizeof(DOSDRIVER),4,BFile::Seek_Set);
     ret = true;
  }
  return ret;
}

static void __FASTCALL__ dossys_init_fmt(CodeGuider& code_guider) { UNUSED(code_guider); }
static void __FASTCALL__ dossys_destroy_fmt() {}
static int  __FASTCALL__ dossys_platform() { return DISASM_CPU_IX86; }

static bool __FASTCALL__ dossys_AddressResolv(char *addr,__filesize_t cfpos)
{
  bool bret = true;
  if(cfpos < sizeof(DOSDRIVER)+4) sprintf(addr,"SYSH:%s",Get4Digit(cfpos));
  else bret = false;
  return bret;
}

static __filesize_t __FASTCALL__ SYSHelp()
{
  hlpDisplay(10014);
  return BMGetCurrFilePos();
}

static __filesize_t __FASTCALL__ sysVA2PA(__filesize_t va)
{
  return va;
}

static __filesize_t __FASTCALL__ sysPA2VA(__filesize_t pa)
{
  return pa;
}

extern const REGISTRY_BIN dossysTable =
{
  "DOS driver",
  { "SYSHlp", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
  { SYSHelp, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
  dossys_check_fmt,
  dossys_init_fmt,
  dossys_destroy_fmt,
  ShowSysHeader,
  NULL,
  dossys_platform,
  NULL,
  NULL,
  dossys_AddressResolv,
  sysVA2PA,
  sysPA2VA,
  NULL,
  NULL
};
} // namespace	usr
