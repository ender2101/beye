#include "config.h"
#include "libbeye/libbeye.h"
using namespace	usr;
/**
 * @namespace	usr
 * @file        main.cpp
 * @brief       This file contains entry point of BEYE project.
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
 * @author      Kostya Nosov <k-nosov@yandex.ru>
 * @date        27.11.2000
 * @note        Changing technology recognition of new-exe files
**/
#include <iostream>
#include <string>
#include <vector>
#include <map>

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <limits.h>
#include <sys/stat.h>
#ifdef HAVE_SYS_RESOURCE
#include <sys/resource.h>
#endif
#include <stdlib.h>
#include <errno.h>

#include "addons/addendum.h"
#include "addons/sysinfo.h"
#include "beye.h"
#include "bconsole.h"
#include "colorset.h"
#include "bin_util.h"
#include "codeguid.h"
#include "editor.h"
#include "tstrings.h"
#include "beyeutil.h"
#include "search.h"
#include "setup.h"
#include "libbeye/bbio.h"
#include "libbeye/mmfile.h"
#include "libbeye/file_ini.h"
#include "libbeye/kbd_code.h"
#include "libbeye/libbeye.h"
#include "libbeye/osdep/system.h"
#include "libbeye/osdep/tconsole.h"

#include "plugins/plugin.h"

namespace	usr {
extern const Plugin_Info binMode;
extern const Plugin_Info textMode;
extern const Plugin_Info hexMode;
extern const Plugin_Info disMode;

static volatile char antiviral_hole1[__VM_PAGE_SIZE__] __PAGE_ALIGNED__;

static Opaque		opaque1;
static BeyeContext* BeyeCtx=NULL;
static Opaque		opaque2;

static volatile char antiviral_hole2[__VM_PAGE_SIZE__] __PAGE_ALIGNED__;

static const unsigned SHORT_PATH_LEN=__TVIO_MAXSCREENWIDTH-54;

static volatile char antiviral_hole3[__VM_PAGE_SIZE__] __PAGE_ALIGNED__;

static int malloc_debug=0;

static volatile char antiviral_hole4[__VM_PAGE_SIZE__] __PAGE_ALIGNED__;

BeyeContext& beye_context() { return *BeyeCtx; }

bool BeyeContext::select_mode()
{
    size_t i,nModes = modes.size();
    const char *modeName[nModes];
    int retval;

    for(i = 0;i < nModes;i++) modeName[i] = modes[i]->name;
    retval = SelBoxA(modeName,nModes," Select translation mode: ",defMainModeSel);
    if(retval != -1) {
	defMainModeSel = retval;
	delete activeMode;
	activeMode = modes[defMainModeSel]->query_interface(bin_format(),bm_file_handle,*MainWnd,*code_guider);
	return true;
    }
    return false;
}

void BeyeContext::init_modes( Ini_Profile& ini )
{
    if(!activeMode) activeMode = modes[defMainModeSel]->query_interface(bin_format(),bm_file_handle,*MainWnd,*code_guider);
    activeMode->read_ini(ini);
}

void BeyeContext::quick_select_mode()
{
    size_t nModes = modes.size();
    if(defMainModeSel < nModes - 1) defMainModeSel++;
    else                            defMainModeSel = 0;
    delete activeMode;
    activeMode = modes[defMainModeSel]->query_interface(bin_format(),bm_file_handle,*MainWnd,*code_guider);
}

void BeyeContext::make_shortname()
{
  unsigned l;
  unsigned slen = TitleWnd->client_width()-54;
  l = ArgVector1.length();
  if(l <= slen) strcpy(_shortname,ArgVector1.c_str());
  else
  {
    strncpy(_shortname,ArgVector1.c_str(),slen/2 - 3);
    _shortname[slen/2-4] = 0;
    strcat(_shortname,"...");
    strcat(_shortname,&ArgVector1.c_str()[l - slen/2]);
  }
  _system->nls_cmdline2oem((unsigned char *)_shortname,strlen(_shortname));
}

void BeyeContext::auto_detect_mode()
{
    size_t i,n = modes.size();
    Plugin* mode;
    for(i = 0;i < n;i++) {
	mode = modes[i]->query_interface(bin_format(),bm_file_handle,*MainWnd,*code_guider);
	if(mode->detect()) {
	    defMainModeSel = i;
	    break;
	}
	delete mode; mode = NULL;
    }
    if(mode) delete mode;
    activeMode = modes[defMainModeSel]->query_interface(bin_format(),bm_file_handle,*MainWnd,*code_guider);
    beye_context().bm_file().seek(0,binary_stream::Seek_Set);
}

static const struct tagbeyeArg {
    const char key[4];
    const char *prompt;
}beyeArg[] = {
  { "-a", "autodetect mode (default)" },
  { "-b", "view file in binary mode" },
  { "-d", "view file in disassembler mode" },
  { "-h", "view file in hexadecimal mode" },
  { "-t", "view file in text mode" },
  { "-s", "change size of file to NNN bytes (create, if file does not exist)" },
  { "-i", "ignore .ini file (create new)" },
  { "-m", "debug mp_malloc: (1 - bounds; 2 - prebounds; 3-leaks)" },
  { "-k", "test anti-viral protection" },
  { "-?", "display this screen" }
};

int BeyeContext::queryKey(const std::string& arg)
{
  int ret = -1;
  size_t i;
  for(i = 0;i < sizeof(beyeArg)/sizeof(struct tagbeyeArg);i++)
  {
    if(arg==beyeArg[i].key) { ret = i; break; }
  }
  return ret;
}

void BeyeContext::parse_cmdline( const std::vector<std::string>& ArgVector )
{
  unsigned i;
  ListFile.clear();
  for(i = 1;i < ArgVector.size();i++)
  {
     int beye_key;
     beye_key = queryKey(ArgVector[i]);
     switch(beye_key)
     {
       case 0: beye_mode = UINT_MAX; break;
       case 1: beye_mode = 1; break;
       case 2: beye_mode = 3; break;
       case 3: beye_mode = 2; break;
       case 4: beye_mode = 0; break;
       case 5:
		new_file_size = strtoull(ArgVector[++i].c_str(),NULL,10);
		break;
       case 6: UseIniFile = false; break;
       case 7: i++; break; // parsed early
       case 8: i++; break; // parsed early
       case 9: ListFile.clear(); return;
       default: ListFile.push_back(ArgVector[i]);
     }
  }
  if(!ListFile.empty()) ArgVector1 = ListFile[0];
}

bool BeyeContext::LoadInfo( )
{
    make_shortname();
    if(new_file_size != FILESIZE_MAX) {
	binary_stream* h = new(zeromem) binary_stream;
	if(binary_stream::exists(beye_context().ArgVector1) == false) h->create(beye_context().ArgVector1);
	else {
	    if(!h->open(beye_context().ArgVector1,binary_stream::FO_READWRITE | binary_stream::SO_DENYNONE))
		h->open(ArgVector1,binary_stream::FO_READWRITE | binary_stream::SO_COMPAT);
	}
	h->chsize(new_file_size);
        delete h;
    }
    if(BMOpen(ArgVector1) != true) return false;
    if(beye_mode != UINT_MAX) defMainModeSel = beye_mode;
    else {
	if(LastMode >= modes.size() || !is_valid_ini_args()) auto_detect_mode();
	else defMainModeSel = LastMode;
    }
    return true;
}

void BeyeContext::PaintTitle() const
{
 TitleWnd->freeze();
 TitleWnd->goto_xy(1,1);
 TitleWnd->clreol();
 TitleWnd->printf("File : %s",short_name());
 TitleWnd->goto_xy(TitleWnd->client_width()-43,1);
 TitleWnd->printf("Size : %8llu bytes",flength());
 TitleWnd->refresh();
}

static void MyAtExit()
{
  delete BeyeCtx;
  mp_uninit_malloc(malloc_debug?1:0);
}

bool BeyeContext::is_valid_ini_args( ) const
{
  return iniSettingsAnywhere ? true :
	 !ArgVector1.empty() ?
	 ArgVector1==LastOpenFileName ?
	 beye_mode != UINT_MAX && beye_mode != LastMode ?
	 false : true : false : false;
}

Ini_Profile& BeyeContext::load_ini_info()
{
  char buf[20];
  std::string tmp,stmp;
  Ini_Profile& ini = *new(zeromem) Ini_Profile;
  const char* iname;
  iname = ::getenv("BEYE_INI");
  if(iname) ini_name=iname;
  if(ini_name.empty()) ini_name = _system->get_ini_name("beye");
  if(UseIniFile) ini.open(ini_name);
  help_name=read_profile_string(ini,"Beye","Setup","HelpName","");
  skin_name=read_profile_string(ini,"Beye","Setup","SkinName","");
  syntax_name=read_profile_string(ini,"Beye","Setup","SyntaxName","");
  stmp=read_profile_string(ini,"Beye","Search","String","");
  strcpy((char*)search_buff,stmp.c_str());
  search_len = stmp.length();
  tmp=read_profile_string(ini,"Beye","Search","Case","off");
  beyeSearchFlg = stricmp(tmp.c_str(),"on") == 0 ? SF_CASESENS : SF_NONE;
  tmp=read_profile_string(ini,"Beye","Search","Word","off");
  if(stricmp(tmp.c_str(),"on") == 0) beyeSearchFlg |= SF_WORDONLY;
  tmp=read_profile_string(ini,"Beye","Search","Backward","off");
  if(stricmp(tmp.c_str(),"on") == 0) beyeSearchFlg |= SF_REVERSE;
  tmp=read_profile_string(ini,"Beye","Search","Template","off");
  if(stricmp(tmp.c_str(),"on") == 0) beyeSearchFlg |= SF_WILDCARDS;
  tmp=read_profile_string(ini,"Beye","Search","UsePlugin","off");
  if(stricmp(tmp.c_str(),"on") == 0) beyeSearchFlg |= SF_PLUGINS;
  tmp=read_profile_string(ini,"Beye","Search","AsHex","off");
  if(stricmp(tmp.c_str(),"on") == 0) beyeSearchFlg |= SF_ASHEX;
  LastOpenFileName=read_profile_string(ini,"Beye","Browser","LastOpen","");
  sprintf(buf,"%u",LastMode); /* [dBorca] so that src and dst won't overlap for strncpy */
  tmp=read_profile_string(ini,"Beye","Browser","LastMode",buf);
  LastMode = (size_t)strtoul(tmp.c_str(),NULL,10);
  tmp=read_profile_string(ini,"Beye","Browser","Offset","0");
  LastOffset = atoll(tmp.c_str());
  ini_ver=read_profile_string(ini,"Beye","Setup","Version","");
  tmp=read_profile_string(ini,"Beye","Setup","DirectConsole","yes");
  if(stricmp(tmp.c_str(),"yes") == 0) vioIniFlags = __TVIO_FLG_DIRECT_CONSOLE_ACCESS;
  tmp=read_profile_string(ini,"Beye","Setup","ForceMono","no");
  if(stricmp(tmp.c_str(),"yes") == 0) twinIniFlags = TWIF_FORCEMONO;
  tmp=read_profile_string(ini,"Beye","Setup","Force7Bit","no");
  if(stricmp(tmp.c_str(),"yes") == 0) vioIniFlags |= __TVIO_FLG_USE_7BIT;
  tmp=read_profile_string(ini,"Beye","Setup","MouseSens","yes");
  if(stricmp(tmp.c_str(),"yes") == 0) kbdFlags = KBD_NONSTOP_ON_MOUSE_PRESS;
  tmp=read_profile_string(ini,"Beye","Setup","IniSettingsAnywhere","no");
  if(stricmp(tmp.c_str(),"yes") == 0) iniSettingsAnywhere = true;
  tmp=read_profile_string(ini,"Beye","Setup","FioUseMMF","no");
  if(stricmp(tmp.c_str(),"yes") == 0) fioUseMMF = true;
  if(!MMFile::has_mmio) fioUseMMF = false;
  tmp=read_profile_string(ini,"Beye","Setup","PreserveTimeStamp","no");
  if(stricmp(tmp.c_str(),"yes") == 0) iniPreserveTime = true;
  tmp=read_profile_string(ini,"Beye","Setup","UseExternalProgs","no");
  if(stricmp(tmp.c_str(),"yes") == 0) iniUseExtProgs = true;
  codepage=read_profile_string(ini,"Beye","Setup","Codepage","CP866");
  return ini;
}

void BeyeContext::save_ini_info() const
{
  char tmp[20];
  search_buff[search_len] = 0;
  Ini_Profile& ini = *new(zeromem) Ini_Profile;
  ini.open(ini_name);
  write_profile_string(ini,"Beye","Setup","HelpName",help_name.c_str());
  write_profile_string(ini,"Beye","Setup","SkinName",skin_name.c_str());
  write_profile_string(ini,"Beye","Setup","SyntaxName",syntax_name.c_str());
  write_profile_string(ini,"Beye","Setup","Version",BEYE_VERSION);
  write_profile_string(ini,"Beye","Search","String",(char *)search_buff);
  write_profile_string(ini,"Beye","Search","Case",beyeSearchFlg & SF_CASESENS ? "on" : "off");
  write_profile_string(ini,"Beye","Search","Word",beyeSearchFlg & SF_WORDONLY ? "on" : "off");
  write_profile_string(ini,"Beye","Search","Backward",beyeSearchFlg & SF_REVERSE ? "on" : "off");
  write_profile_string(ini,"Beye","Search","Template",beyeSearchFlg & SF_WILDCARDS ? "on" : "off");
  write_profile_string(ini,"Beye","Search","UsePlugin",beyeSearchFlg & SF_PLUGINS ? "on" : "off");
  write_profile_string(ini,"Beye","Search","AsHex",beyeSearchFlg & SF_ASHEX ? "on" : "off");
  write_profile_string(ini,"Beye","Browser","LastOpen",ArgVector1.c_str());
  sprintf(tmp,"%u",defMainModeSel);
  write_profile_string(ini,"Beye","Browser","LastMode",tmp);
  sprintf(tmp,"%llu",LastOffset);
  write_profile_string(ini,"Beye","Browser","Offset",tmp);
  strcpy(tmp,vioIniFlags & __TVIO_FLG_DIRECT_CONSOLE_ACCESS ? "yes" : "no");
  write_profile_string(ini,"Beye","Setup","DirectConsole",tmp);
  strcpy(tmp,twinIniFlags & TWIF_FORCEMONO ? "yes" : "no");
  write_profile_string(ini,"Beye","Setup","ForceMono",tmp);
  strcpy(tmp,(vioIniFlags & __TVIO_FLG_USE_7BIT) == __TVIO_FLG_USE_7BIT ? "yes" : "no");
  write_profile_string(ini,"Beye","Setup","Force7Bit",tmp);
  strcpy(tmp,kbdFlags & KBD_NONSTOP_ON_MOUSE_PRESS ? "yes" : "no");
  write_profile_string(ini,"Beye","Setup","MouseSens",tmp);
  strcpy(tmp,iniSettingsAnywhere ? "yes" : "no");
  write_profile_string(ini,"Beye","Setup","IniSettingsAnywhere",tmp);
  strcpy(tmp,fioUseMMF ? "yes" : "no");
  write_profile_string(ini,"Beye","Setup","FioUseMMF",tmp);
  strcpy(tmp,iniPreserveTime ? "yes" : "no");
  write_profile_string(ini,"Beye","Setup","PreserveTimeStamp",tmp);
  strcpy(tmp,iniUseExtProgs ? "yes" : "no");
  write_profile_string(ini,"Beye","Setup","UseExternalProgs",tmp);
  write_profile_string(ini,"Beye","Setup","Codepage",codepage.c_str());
  activeMode->save_ini(ini);
  udnTerm(ini);
  delete &ini;
}

static binary_stream::ftime ftim;
static bool ftim_ok = false;

void BeyeContext::show_usage() const {
    unsigned evt,i,nln,h,y;
    TWindow *win;
    nln = sizeof(beyeArg)/sizeof(struct tagbeyeArg);
    h = nln+4;
    y = _tconsole->vio_height()/2-h/2;
    win = new(zeromem) TWindow(2,y,_tconsole->vio_width()-2,h+1,TWindow::Flag_None | TWindow::Flag_NLS);
    if(!win) goto done;
    win->set_title(BEYE_VER_MSG,TWindow::TMode_Center,error_cset.border);
    win->into_center();
    win->set_color(error_cset.main);
    win->set_frame(TWindow::DOUBLE_FRAME,error_cset.border);
    win->set_footer(" Press [ ESC ] to quit ",TWindow::TMode_Right,error_cset.border);
    win->clear();
    win->goto_xy(1,1);
    win->puts(" Usage: beye [OPTIONS] file...");
    for(i = 0;i < nln;i++)
    {
	win->goto_xy(1,4+i);
	win->printf("  %s     %s\n",beyeArg[i].key,beyeArg[i].prompt);
    }
    win->show();
    do {
	evt = GetEvent(NULL,NULL,ErrorWnd);
    }while(!(evt == KE_ESCAPE || evt == KE_F(10) || evt == KE_ENTER));
    delete win;
    done:
    termBConsole();
}

void	BeyeContext::create_windows() {
    ErrorWnd = new(zeromem) TWindow(1,1,51,17,TWindow::Flag_None | TWindow::Flag_NLS);
    if(ErrorWnd) ErrorWnd->set_title(" Error ",TWindow::TMode_Center,error_cset.border);
    else { std::cerr<<"fatal error: can't create window"<<std::endl; ::exit(EXIT_FAILURE); }
    ErrorWnd->into_center();
    ErrorWnd->set_color(error_cset.main);
    ErrorWnd->set_frame(TWindow::DOUBLE_FRAME,error_cset.border);
    PromptWnd = new(zeromem) TWindow(1,_tconsole->vio_height(),_tconsole->vio_width()-1,1,TWindow::Flag_NLS);
    PromptWnd->set_color(prompt_cset.digit);
    PromptWnd->clear();
    PromptWnd->show();
    if(BeyeCtx->ini_ver!=BEYE_VERSION) {
	class Setup* setup = new class Setup;
	setup->run();
	delete setup;
    }
    TitleWnd = new(zeromem) TWindow(1,1,_tconsole->vio_width(),1,TWindow::Flag_None);
    TitleWnd->set_color(title_cset.main);
    TitleWnd->clear();
    TitleWnd->show();
    MainWnd = new(zeromem) TWindow(1,2,_tconsole->vio_width(),_tconsole->vio_height()-2,TWindow::Flag_None);
    MainWnd->set_color(browser_cset.main);
    MainWnd->clear();
}

int Beye(const std::vector<std::string>& argv, const std::map<std::string,std::string>& envm)
{
 bool skin_err;
 int retval;
#ifndef NDEBUG
#ifdef RLIMIT_CORE
  {
    /* on many systems default coresize is 0.
       Enable any coresize here. */
    struct rlimit rl;
    getrlimit(RLIMIT_CORE,&rl);
    rl.rlim_cur = rl.rlim_max;
    setrlimit(RLIMIT_CORE,&rl);
  }
#endif
#endif
  BeyeCtx=new(zeromem) BeyeContext(argv,envm);
/*
    flg=MPA_FLG_RANDOMIZER;
    flg=MPA_FLG_BOUNDS_CHECK;
    flg=MPA_FLG_BEFORE_CHECK;
    flg=MPA_FLG_BACKTRACE;
*/
 Ini_Profile& ini=BeyeCtx->load_ini_info();
 skin_err = csetReadIniFile(BeyeCtx->skin_name.c_str());
 initBConsole(BeyeCtx->vioIniFlags,BeyeCtx->twinIniFlags);
 if(argv.size() < 2) goto show_usage;
 BeyeCtx->parse_cmdline(argv);
 if(BeyeCtx->list_file().empty())
 {
   /** print usage message */
    size_t i;
    show_usage:
    BeyeCtx->show_usage();
    std::cerr<<BEYE_VER_MSG<<std::endl;
    std::cerr<<" Usage: beye [OPTIONS] file...<<"<<std::endl<<std::endl;
    for(i = 0;i < sizeof(beyeArg)/sizeof(struct tagbeyeArg);i++) {
	std::cerr<<beyeArg[i].key<<" "<<beyeArg[i].prompt<<std::endl;
    }
    std::cerr<<std::endl;
    return EXIT_FAILURE;
 }
 udnInit(ini);
 BeyeCtx->create_windows();
 atexit(MyAtExit);
 retval = EXIT_SUCCESS;
 if(skin_err)
 {
   char sout[256];
   sprintf(sout,"Error in skin file detected: '%s'",BeyeCtx->last_skin_error.c_str());
   BeyeCtx->ErrMessageBox(sout,"");
 }
 /* We must do it before opening a file because of some RTL has bug
    when are trying to open already open file with no sharing access */
 ftim_ok = binary_stream::get_ftime(BeyeCtx->ArgVector1,ftim);
 if(!BeyeCtx->LoadInfo())
 {
   delete &ini;
   retval = EXIT_FAILURE;
   goto Bye;
 }
 BeyeCtx->bin_format().detect_format(BeyeCtx->sc_bm_file());
 BeyeCtx->init_modes(ini);
 delete &ini;
 BeyeCtx->PaintTitle();
 if(!BeyeCtx->is_valid_ini_args() || BeyeCtx->LastOffset > BeyeCtx->flength()) BeyeCtx->LastOffset = 0;
 BeyeCtx->main_wnd().show();
 BeyeCtx->main_loop();
 BeyeCtx->LastOffset = BeyeCtx->tell();
 BeyeCtx->save_ini_info();
 if(BeyeCtx->iniPreserveTime && ftim_ok) binary_stream::set_ftime(BeyeCtx->ArgVector1,ftim);
 Bye:
 return retval;
}

bool BeyeContext::new_source()
{
  int i;
    bool ret;
    unsigned j,freq;
    static int prev_file;
    char ** nlsListFile;
    size_t sz=ListFile.size();
    nlsListFile = new char*[sz];
    if(nlsListFile) {
	for(j = 0;j < sz;j++) {
	    nlsListFile[j] = new char [ListFile[j].length()+1];
	    if(!nlsListFile[j]) break;
	}
    } else { MemOutBox("Initializing List of File\n"); return 0; }
    for(freq = 0;freq < j;freq++) {
	unsigned ls;
	ls = ListFile[freq].length();
	::memcpy(nlsListFile[freq],ListFile[freq].c_str(),ls+1);
	_system->nls_cmdline2oem((unsigned char *)nlsListFile[freq],ls);
    }
    i = SelBoxA(const_cast<const char**>(nlsListFile),j," Select new file: ",prev_file);
    ret = 0;
    for(freq = 0;freq < j;freq++) delete nlsListFile[freq];
    delete nlsListFile;
    if(i != -1) {
	if(iniPreserveTime && ftim_ok) binary_stream::set_ftime(ArgVector1,ftim);
	BMClose();
	ftim_ok = binary_stream::get_ftime(ListFile[i],ftim);
	if(BMOpen(ListFile[i]) == true) {
	    ArgVector1 = ListFile[i];
	    delete _bin_format;
	    delete activeMode;
	    make_shortname();
	    _bin_format = new(zeromem) Bin_Format(*code_guider);
	    _bin_format->detect_format(sc_bm_file_handle);
	    activeMode=modes[defMainModeSel]->query_interface(bin_format(),bm_file_handle,*MainWnd,*code_guider);
	    ret = true;
	} else {
	    if(BMOpen(ArgVector1) != true) ::exit(EXIT_FAILURE);
	    delete _bin_format;
	    delete activeMode;
	    make_shortname();
	    _bin_format = new(zeromem) Bin_Format(*code_guider);
	    _bin_format->detect_format(sc_bm_file_handle);
	    activeMode=modes[defMainModeSel]->query_interface(bin_format(),bm_file_handle,*MainWnd,*code_guider);
	    ret = false;
	}
    }
    return ret;
}

std::string BeyeContext::read_profile_string(Ini_Profile& ini,
			       const std::string& section,
			       const std::string& subsection,
			       const std::string& _item,
			       const std::string& def_value) const
{
  return UseIniFile ? ini.read(section,subsection,_item,def_value)
		    : def_value;
}

bool BeyeContext::write_profile_string(Ini_Profile& ini,
					  const std::string& section,
					  const std::string& subsection,
					  const std::string& item,
					  const std::string& value) const
{
  return ini.write(section,subsection,item,value);
}


BeyeContext::BeyeContext(const std::vector<std::string>& _argv, const std::map<std::string,std::string>& _envm)
	    :vioIniFlags(0L)
	    ,twinIniFlags(0L)
	    ,kbdFlags(0L)
	    ,iniSettingsAnywhere(false)
	    ,fioUseMMF(false)
	    ,iniPreserveTime(false)
	    ,iniUseExtProgs(false)
	    ,LastOffset(0L)
	    ,argv(_argv)
	    ,envm(_envm)
	    ,UseIniFile(true)
	    ,beye_mode(UINT_MAX)
	    ,defMainModeSel(1)
	    ,new_file_size(FILESIZE_MAX)
	    ,code_guider(new(zeromem) CodeGuider)
	    ,bm_file_handle(bNull)
	    ,sc_bm_file_handle(bNull)
	    ,_system(new(zeromem) System)
{
    addons = new(zeromem) addendum;
    sysinfo= new(zeromem) class sysinfo;

    modes.push_back(&textMode);
    modes.push_back(&binMode);
    modes.push_back(&hexMode);
    modes.push_back(&disMode);

    _bin_format = new(zeromem) Bin_Format(*code_guider);

    codepage="CP866";
    scheme_name="Built-in";
    if(argv.size()>1) ArgVector1 = argv[1];
    _shortname = new char[SHORT_PATH_LEN + 1];
    LastMode = modes.size()+10;
}

BeyeContext::~BeyeContext() {
    delete activeMode;
    delete _bin_format;

    delete sysinfo;
    delete addons;
    delete _shortname;
    delete code_guider;
    BMClose();

    if(MainWnd) delete MainWnd;
    if(PromptWnd) delete PromptWnd;
    if(TitleWnd) delete TitleWnd;
    if(ErrorWnd) delete ErrorWnd;
    termBConsole();
}
const std::vector<std::string>& BeyeContext::list_file() const { return ListFile; }

void BeyeContext::select_tool() const { addons->select(); }
void BeyeContext::select_sysinfo() const { sysinfo->select(); }

binary_stream* BeyeContext::beyeOpenRO(const std::string& fname,unsigned cache_size)
{
    binary_stream* fret;
    if(!binary_stream::exists(fname)) return &bNull;
    if(beye_context().fioUseMMF)fret= new(zeromem) MMFile;
    else			fret= new(zeromem) BBio_File(cache_size,BBio_File::Opt_Db);
    bool rc;
    rc = fret->open(fname,binary_stream::FO_READONLY | binary_stream::SO_DENYNONE);
    if(rc == false)
	rc = fret->open(fname,binary_stream::FO_READONLY | binary_stream::SO_COMPAT);
    if(rc==false) { delete fret; fret=&bNull; }
    return fret;
}

binary_stream* BeyeContext:: beyeOpenRW(const std::string& fname,unsigned cache_size)
{
    binary_stream* fret;
    if(!binary_stream::exists(fname)) return &bNull;
    if(beye_context().fioUseMMF)fret= new(zeromem) MMFile;
    else			fret= new(zeromem) BBio_File(cache_size,BBio_File::Opt_Db);
    bool rc;
    rc = fret->open(fname,binary_stream::FO_READWRITE | binary_stream::SO_DENYNONE);
    if(rc == false)
	rc = fret->open(fname,binary_stream::FO_READWRITE | binary_stream::SO_COMPAT);
    if(rc==false) { delete fret; fret=&bNull; }
    return fret;
}

bool BeyeContext::BMOpen(const std::string& fname)
{
  binary_stream *bm,*sc;
  bm = beyeOpenRO(fname,BBIO_CACHE_SIZE);
  if(bm == &bNull)
  {
    errnoMessageBox(OPEN_FAIL,"",errno);
    return false;
  }
  if(&bm_file_handle != &bNull) delete &bm_file_handle;
  bm_file_handle = *bm;
  sc = bm_file_handle.dup();
  if(sc == &bNull)
  {
    errnoMessageBox(DUP_FAIL,"",errno);
    return false;
  }
  if(&sc_bm_file_handle != &bNull) delete &sc_bm_file_handle;
  sc_bm_file_handle = *sc;
  return true;
}

void BeyeContext::BMClose()
{
  if(&bm_file_handle != &bNull) delete &bm_file_handle;
  bm_file_handle = bNull;
  if(&sc_bm_file_handle != &bNull) delete &sc_bm_file_handle;
  sc_bm_file_handle = bNull;
}

__filesize_t BeyeContext::flength() const { return bm_file_handle.flength(); }
__filesize_t BeyeContext::tell() const { return bm_file_handle.tell(); }

static bool test_antiviral_protection(int* verbose)
{
    if(*verbose) std::cerr<<"Your've specified test-av option!\nRight now Beye should make coredump!"<<std::endl;
    *verbose=antiviral_hole1[0]|antiviral_hole2[0]|antiviral_hole3[0]|antiviral_hole4[0];
    std::cerr<<"Antiviral protection of Beye doesn't work!"<<std::endl;
    return false;
}
} // namespace	usr

int main(int argc,char* args[], char *envp[])
{
    try {
	/* init malloc */
	int do_av_test=0;
	size_t i;
	mp_malloc_e flg=MPA_FLG_RANDOMIZER;
	malloc_debug=0;
	for(i=0;i<size_t(argc);i++) {
	    if(strcmp(args[i],"-m")==0) {
		i++;
		malloc_debug=::atoi(args[i]);
		switch(malloc_debug) {
		    default:
		    case 0: flg=MPA_FLG_RANDOMIZER; break;
		    case 1: flg=MPA_FLG_BOUNDS_CHECK; break;
		    case 2: flg=MPA_FLG_BEFORE_CHECK; break;
		    case 3: flg=MPA_FLG_BACKTRACE; break;
		}
		break;
	    }
	    if(strcmp(args[i],"-k")==0) do_av_test=1;
	}
	mp_init_malloc(args[0],1000,10,flg);
	/* init vectors */
	std::vector<std::string> ArgVector;
	std::string str,stmp;
	for(i=0;i<size_t(argc);i++) {
	    str=args[i];
	    ArgVector.push_back(str);
	}
	args[argc] = (char*)make_false_pointer((any_t*)antiviral_hole1);
	std::map<std::string,std::string> envm;
	unsigned j=0;
	size_t pos;
	while(envp[j]) {
	    str=envp[j++];
	    pos=str.find('=');
	    if(pos==std::string::npos) throw "Broken environment variable: "+str;
	    stmp=str.substr(pos+1);
	    str=str.substr(0,pos);
	    envm[str]=stmp;
	}
//	envp[j+1] = NULL;
#ifdef HAVE_MPROTECT
	/* init antiviral protection */
	int rc;
	rc=mp_mprotect((any_t*)antiviral_hole1,sizeof(antiviral_hole1),MP_DENY_ALL);
	rc|=mp_mprotect((any_t*)antiviral_hole2,sizeof(antiviral_hole2),MP_DENY_ALL);
	rc|=mp_mprotect((any_t*)antiviral_hole3,sizeof(antiviral_hole3),MP_DENY_ALL);
	rc|=mp_mprotect((any_t*)antiviral_hole4,sizeof(antiviral_hole4),MP_DENY_ALL);
	if(rc) {
		std::cerr<<"*** Error! Cannot initialize antiviral protection: '"<<strerror(errno)<<"' ***!"<<std::endl;
		return EXIT_FAILURE;
	}
#else
	std::cerr<<"*** Project was compiled without antiviral protection!"<<std::endl;
#endif
	if(do_av_test) {
	    test_antiviral_protection(&do_av_test);
	    std::cerr<<"*** Test of antiviral protection failed! ***"<<std::endl;
	    return EXIT_FAILURE;
	}
	/* call program */
	return Beye(ArgVector,envm);
    } catch(const std::string& what) {
	std::cout<<"[main_module] Exception '"<<what<<"'caught in module: MPlayerXP"<<std::endl;
    }
    return EXIT_FAILURE;
}
