#include "config.h"
#include "libbeye/libbeye.h"
using namespace	usr;
/**
 * @namespace	usr_plugins_auto
 * @file        plugins/bin/opharlap.c
 * @brief       This file contains implementation of Old PharLap file format decoder.
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
#include <sstream>
#include <iomanip>

#include <stdio.h>
#include <string.h>

#include "colorset.h"
#include "udn.h"
#include "beyehelp.h"
#include "bconsole.h"
#include "plugins/bin/pharlap.h"
#include "plugins/disasm.h"
#include "libbeye/kbd_code.h"
#include "libbeye/bstream.h"
#include "plugins/binary_parser.h"
#include "beye.h"

namespace	usr {
    class oldPharLap_Parser : public Binary_Parser {
	public:
	    oldPharLap_Parser(BeyeContext& b,binary_stream&,CodeGuider&,udn&);
	    virtual ~oldPharLap_Parser();

	    virtual const char*		prompt(unsigned idx) const;
	    virtual __filesize_t	action_F1();

	    virtual __filesize_t	show_header() const;
	    virtual int			query_platform() const;
	    virtual std::string		address_resolving(__filesize_t);
	private:
	    oldPharLap			oph;
	    BeyeContext&		bctx;
	    binary_stream&		main_handle;
	    udn&			_udn;
    };
static const char* txt[]={"PLHelp","","","","","","","","",""};
const char* oldPharLap_Parser::prompt(unsigned idx) const { return txt[idx]; }

__filesize_t oldPharLap_Parser::show_header() const
{
  __filesize_t fpos,entrypoint;
  TWindow * w;
  unsigned keycode;
  fpos = bctx.tell();
  entrypoint = oph.plHeadSize*16 + oph.plEIP;
  w = CrtDlgWndnls(" Old PharLap executable ",54,11);
  w->goto_xy(1,1);
  w->printf(
	   "Image size reminder on last page   = %04XH\n"
	   "Image size in pages                = %04XH\n"
	   "Number of relocation items         = %04XH\n"
	   "Header size in paragraphs          = %04XH\n"
	   "Min. number of extra 4K pages      = %04XH\n"
	   "Max. number of extra 4K pages      = %04XH\n"
	   "Initial ESP                        = %08lXH\n"
	   "File checksum                      = %04XH\n"
	   "Initial EIP                        = %08lXH\n"
	   "Offset of first relocation item    = %04XH\n"
	   "Number of overlays                 = %04XH\n"
	   ,oph.plSizeRemaind
	   ,oph.plImageSize
	   ,oph.plNRelocs
	   ,oph.plHeadSize
	   ,oph.plMinExtraPages
	   ,oph.plMaxExtraPages
	   ,oph.plESP
	   ,oph.plCheckSum
	   ,oph.plEIP
	   ,oph.plFirstReloc
	   ,oph.plNOverlay);
  w->set_color(dialog_cset.entry);
  w->printf("Entry Point                        = %08lXH",entrypoint);
  w->clreol();
  while(1)
  {
    keycode = GetEvent(drawEmptyPrompt,NULL,w);
    if(keycode == KE_ENTER) { fpos = entrypoint; break; }
    else
      if(keycode == KE_ESCAPE || keycode == KE_F(10)) break;
  }
  delete w;
  return fpos;
}

oldPharLap_Parser::oldPharLap_Parser(BeyeContext& b,binary_stream& h,CodeGuider& code_guider,udn& u)
		:Binary_Parser(b,h,code_guider,u)
		,bctx(b)
		,main_handle(h)
		,_udn(u)
{
    char sign[2];
    main_handle.seek(0,binary_stream::Seek_Set);
    binary_packet bp=main_handle.read(2); memcpy(sign,bp.data(),bp.size());
    if(!(sign[0] == 'M' && sign[1] == 'P')) throw bad_format_exception();

    main_handle.seek(0,binary_stream::Seek_Set);
    bp=main_handle.read(sizeof(oph)); memcpy(&oph,bp.data(),bp.size());
}

oldPharLap_Parser::~oldPharLap_Parser(){}

std::string oldPharLap_Parser::address_resolving(__filesize_t cfpos)
{
    std::ostringstream oss;
    /* Since this function is used in references resolving of disassembler
	it must be seriously optimized for speed. */
    if(cfpos < sizeof(oldPharLap)) oss<<"oplhdr:"<<std::hex<<std::setfill('0')<<std::setw(2)<<cfpos;
    return oss.str();
}

__filesize_t oldPharLap_Parser::action_F1()
{
    Beye_Help bhelp(bctx);
    if(bhelp.open(true)) {
	bhelp.run(10008);
	bhelp.close();
    }
    return bctx.tell();
}

int oldPharLap_Parser::query_platform() const { return DISASM_CPU_IX86; }

static Binary_Parser* query_interface(BeyeContext& b,binary_stream& h,CodeGuider& _parent,udn& u) { return new(zeromem) oldPharLap_Parser(b,h,_parent,u); }
extern const Binary_Parser_Info oldpharlap_info = {
    "PharLap",	/**< plugin name */
    query_interface
};
} // namespace	usr
