/**
 * @namespace   libbeye
 * @file        libbeye/libbeye.h
 * @brief       This file contains extensions of standard C library, that needed
 *              for BEYE project.
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
#ifndef __BEYELIB_H
#define __BEYELIB_H 1
#include "config.h"

#include <stdexcept>
#include <vector>
#include <string>

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#ifdef HAVE_ENDIAN_H
#include <endian.h>
#endif

#include "libbeye/sysdep/_sys_dep.h"

#include "libbeye/mp_malloc.h"

		   /** Converts all alphabetic characters in buffer to upper case.
		     * @return                none
		     * @param buff            buffer to be converted
		     * @param cbBuff          size of buffer
		     * @see                   memlwr
		    **/
extern void __FASTCALL__ memupr(any_t*buffer,unsigned cb_buffer);

		   /** Converts all alphabetic characters in buffer to lower case.
		     * @return                none
		     * @param buff            buffer to be converted
		     * @param cbBuff          size of buffer
		     * @see                   memlwr
		    **/
extern void __FASTCALL__ memlwr(any_t*buffer,unsigned cb_buffer);

#if defined(__GLIBC__) || defined (__UNIX__)
inline void strupr(char* s) { memupr(s,::strlen(s)); } /**< C library of *nix systems lacks strupr function */
inline void strlwr(char* s) { memlwr(s,::strlen(s)); } /**< C library of *nix systems lacks strlwr function */
inline int stricmp(const char* s1,const char* s2) { return strcasecmp(s1,s2); } /**< Alias of stricmp for *nix systems */
#endif
namespace	usr {
    typedef int64_t __fileoff_t;
    typedef uint64_t __filesize_t;
#define FILEOFF_MAX std::numeric_limits<int64_t>::max()
#define FILESIZE_MAX std::numeric_limits<uint64_t>::max()

    enum beye_aka_binary_eye_project_data_type_qualifier__byte_t{ type_byte=0 };
    enum beye_aka_binary_eye_project_data_type_qualifier__word_t{ type_word=0 };
    enum beye_aka_binary_eye_project_data_type_qualifier_dword_t{ type_dword=0 };
    enum beye_aka_binary_eye_project_data_type_qualifier_qword_t{ type_qword=0 };


    /* KEYBOARD handling */
    enum {
	KBD_NONSTOP_ON_MOUSE_PRESS=0x00000001L /**< Defines that \e kbdGetKey must receive mouse events as frequently as it possible. Otherwise each press on mouse button will send only one event. */
    };
    /* MOUSE handling */
    enum {
	MS_LEFTPRESS    =1,  /**< Defines that left button of mouse have been pressed */
	MS_RIGHTPRESS   =2,  /**< Defines that middle button of mouse have been pressed */
	MS_MIDDLEPRESS  =4   /**< Defines that right button of mouse have been pressed */
    };
    enum {
	__TVIO_MAXSCREENWIDTH		=255, /**< Defines maximal width of screen */
	__TVIO_FLG_DIRECT_CONSOLE_ACCESS=0x00000001L, /**< Defines that video subsystem must access to console directly, if it possible */
	__TVIO_FLG_USE_7BIT		=0x00000002L  /**< Defines that video subsystem must strip high bit of video characters */
    };
    typedef uint8_t ColorAttr; /**< This is the data type used to represent attributes of color */
    typedef uint8_t t_vchar;   /**< This is the data type used to represent video character */
    typedef unsigned tAbsCoord; /**< This is the data type used to represent screen-related coordinates */

    class missing_device_exception : public std::exception {
	public:
	    missing_device_exception() throw();
	    virtual ~missing_device_exception() throw();

	    virtual const char*	what() const throw();
    };

    class missing_driver_exception : public std::exception {
	public:
	    missing_driver_exception() throw();
	    virtual ~missing_driver_exception() throw();

	    virtual const char*	what() const throw();
    };

    class Opaque {
	public:
	    Opaque();
	    virtual ~Opaque();
	
	any_t*		false_pointers[RND_CHAR0];
	any_t*		unusable;
    };

    template <typename T> class LocalPtr : public Opaque {
	public:
	    LocalPtr(T* value):ptr(value) {}
	    virtual ~LocalPtr() { delete ptr; }

	    T& operator*() const { return *ptr; }
	    T* operator->() const { return ptr; }
	private:
	    LocalPtr<T>& operator=(LocalPtr<T> a) { return this; }
	    LocalPtr<T>& operator=(LocalPtr<T>& a) { return this; }
	    LocalPtr<T>& operator=(LocalPtr<T>* a) { return this; }

	    Opaque	opaque1;
	    T*		ptr;
	    Opaque	opaque2;
    };

    inline bool __CONST_FUNC__ TESTFLAG(long x, long y) { return (x&y)==y; } /**< Test y bits in x */

		   /** Tests wether character is a separator
		     * @return                true if given character is separator
		     * @param ch              character to be tested
		     * @note                  returns true if character is space
		     *                        or punctuator
		    **/
    bool  __FASTCALL__ isseparate(int ch) __PURE_FUNC__;

/** ASCIIZ string extended support */

		   /** Removes all trailing spaces from string
		     * @return                number of removed spaces
		     * @param str             pointer to string to be trimmed
		     * @see                   szTrimLeadingSpace szKillSpaceAround
		    **/
    int         __FASTCALL__ szTrimTrailingSpace(char *str);
    std::string __FASTCALL__ szTrimTrailingSpace(const std::string& str);

		   /** Removes all leading spaces from string
		     * @return                number of removed spaces
		     * @param str             pointer to string to be trimmed
		     * @see                   szTrimTrailingSpace szKillSpaceAround
		    **/
    int   __FASTCALL__ szTrimLeadingSpace(char *str);

		   /** Converts space into tabulation characters
		     * @return                string with tabs.
		     * @param src             pointer to source string
		     * @see                   szTab2Space
		    **/
    std::string  __FASTCALL__ szSpace2Tab(const std::string& src);

		   /** Removes all spaces around given position
		     * @return                pointer onto next non space character
		     * @param str             pointer to string to be converted
		     * @param point_to        specifies position to be unspaced
		     * @see                   szTrimLeadingSpace szTrimTrailingSpace
		    **/
    char *__FASTCALL__ szKillSpaceAround(char *str,char *point_to);
} // namespace	usr
#endif
