/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-95 Paul J Turner, Portland, OR
 * Copyright (c) 1996-99 GRACE Development Team
 * 
 * Maintained by Evgeny Stambulchik
 * 
 * 
 *                           All Rights Reserved
 * 
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 * 
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 * 
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __BITMAPS_H_
#define __BITMAPS_H_

///#if defined(HAVE_XPM)
static char * grace_icon_xpm[] = {
"48 39 6 1",
" 	c #000000000000",
".	c #FFFFFFFFFFFF",
"X	c #BEFBBEFBBEFB",
"o	c #51445144FFFF",
"O	c #FFFF14514103",
"+	c #0000AAAA1861",
"                                                ",
" .............................................. ",
" .............................................. ",
" ...............                  ............. ",
" .............................................. ",
" .................              ............... ",
" .............................................. ",
" .......                                 ...... ",
" ....... XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX ...... ",
" ....... XXXXXXXXXXXXXXXXXXXXXXXXXXoXXXX ...... ",
" ..... . XXoooXX      XXXXXXXXXXXoooXXXX ...... ",
" .. .... XXXXXXXXXXXXXXXXXXXXXXXXoXooOXX ...... ",
" .. .... XXOOOXX     XXXXXXXXXXXXoXOoXXX ...... ",
" .. .... XXXXXXXXXXXXXXXXXXXXXXXoOOXooXX ...... ",
" .. .... XX+++XX     XXXXooXXXXOoXXXXoXX ...... ",
" .. .. . XXXXXXXXXXXXXXXoXoXXXOoXXXXXXXX ...... ",
" .. .... XXXXXXXXXXXXXXXoXoXOOooXXXXXXXX ...... ",
" .. .... XXXXXXXXXXXXXXoXXooXXoXXXXXXXXX ...... ",
" .. .... XXXXXXXoXXXXXooXOOoXoXXXXXXXXXX ...... ",
" .. .... XXXXXXooXXXXXoOOXXooXXXXXXXXXXX ...... ",
" .. .. . XXXXXXoXoXXXoOXXXXXXXXXXXXXXXXX ...... ",
" .. .... XXXXXooXoXXOoXXXXXXXXXXXXXXXXXX ...... ",
" .. .... XXXXooXXooOooXXXXXXXXXXXXXXXXXX ...... ",
" .. .... XXXXoXXOOoXoXXXXXXXXXX+++++XXXX ...... ",
" .. .... XXXXoXOXXoXoXXXXXXXXXX+++++XXXX ...... ",
" .. .. . XXXooOXXXXoXXXX+++++XX+++++XXXX ...... ",
" .. .... XXOoXXXXXXXXXXX+++++XX+++++XXXX ...... ",
" .. .... XOoXXXXX+++++XX+++++XX+++++XXXX ...... ",
" .. .... XXoXXXXX+++++XX+++++XX+++++XXXX ...... ",
" ....... XooXXXXX+++++XX+++++XX+++++XXXX ...... ",
" ..... . XXXXXXXX+++++XX+++++XX+++++XXXX ...... ",
" .......                                 ...... ",
" .............................................. ",
" ........ .... .... .... .... .... .... ....... ",
" .............................................. ",
" ..............                    ............ ",
" .............................................. ",
" .............................................. ",
"                                                "};
/*#else
#define grace_icon_width 48
#define grace_icon_height 48
static unsigned char grace_icon_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0xfe, 0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0xf8, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0xfe, 0xff, 0xff, 0xff, 0x03, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02,
   0x00, 0x02, 0x00, 0x00, 0x10, 0x02, 0x20, 0x02, 0x00, 0x00, 0x98, 0x02,
   0x20, 0x02, 0x00, 0x00, 0x78, 0x02, 0x20, 0x02, 0x00, 0x00, 0x3c, 0x02,
   0x20, 0x02, 0x00, 0x00, 0x66, 0x02, 0x20, 0x02, 0x00, 0x0c, 0x43, 0x02,
   0x20, 0x02, 0x00, 0xce, 0x81, 0x02, 0x20, 0x02, 0x00, 0xaa, 0x01, 0x02,
   0x20, 0x02, 0x00, 0xdb, 0x00, 0x02, 0x20, 0x02, 0x82, 0x57, 0x00, 0x02,
   0x20, 0x02, 0x83, 0x71, 0x00, 0x02, 0x20, 0x02, 0xc7, 0x20, 0x00, 0x02,
   0x20, 0x82, 0x65, 0x00, 0x00, 0x02, 0x20, 0xc2, 0x7c, 0x00, 0x00, 0x02,
   0x20, 0x42, 0x2e, 0x00, 0x00, 0x02, 0x20, 0x42, 0x29, 0x00, 0x0e, 0x02,
   0x20, 0xe2, 0x38, 0x00, 0x0e, 0x02, 0x20, 0x32, 0x10, 0x1c, 0x0e, 0x02,
   0x20, 0x1a, 0x00, 0x1c, 0x0e, 0x02, 0x00, 0x12, 0x38, 0x1c, 0x0e, 0x02,
   0x00, 0x1a, 0x38, 0x1c, 0x0e, 0x02, 0x00, 0x02, 0x38, 0x1c, 0x0e, 0x02,
   0x00, 0xfe, 0xff, 0xff, 0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x0f, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static unsigned char grace_mask_bits[] = {
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
#endif*/

#define auto_width 16
#define auto_height 16
static unsigned char auto_bits[] = {
   0x00, 0x00, 0x18, 0x00, 0x18, 0x00, 0x3c, 0x00, 0x3c, 0x06, 0x66, 0x1e,
   0x7e, 0x76, 0xc3, 0x76, 0xc3, 0x06, 0x00, 0x1c, 0x00, 0x78, 0x00, 0x60,
   0x00, 0x66, 0x00, 0x7e, 0x00, 0x78, 0x00, 0x00};
#define down_width 16
#define down_height 16
static unsigned char down_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01,
   0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xf8, 0x0f, 0xf0, 0x07,
   0xe0, 0x03, 0xc0, 0x01, 0x80, 0x00, 0x00, 0x00};
#define expand_width 16
#define expand_height 16
static unsigned char expand_bits[] = {
   0x00, 0x00, 0xfe, 0x7f, 0xfe, 0x3f, 0x02, 0x18, 0x00, 0x0c, 0x00, 0x06,
   0x00, 0x03, 0x80, 0x01, 0xc0, 0x00, 0x60, 0x00, 0x30, 0x00, 0x18, 0x00,
   0x0c, 0x40, 0xfe, 0x7f, 0xfe, 0x7f, 0x00, 0x00};
#define left_width 16
#define left_height 16
static unsigned char left_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x30, 0x00, 0x38, 0x00,
   0xfc, 0x3f, 0xfe, 0x3f, 0xfc, 0x3f, 0x38, 0x00, 0x30, 0x00, 0x20, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#define right_width 16
#define right_height 16
static unsigned char right_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x0c,
   0x00, 0x1c, 0xfc, 0x3f, 0xfc, 0x7f, 0xfc, 0x3f, 0x00, 0x1c, 0x00, 0x0c,
   0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#define shrink_width 16
#define shrink_height 16
static unsigned char shrink_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x1f, 0xf0, 0x1f,
   0x10, 0x0c, 0x00, 0x06, 0x00, 0x03, 0x80, 0x01, 0xc0, 0x00, 0x60, 0x10,
   0xf0, 0x1f, 0xf0, 0x1f, 0x00, 0x00, 0x00, 0x00};
#define up_width 16
#define up_height 16
static unsigned char up_bits[] = {
   0x00, 0x00, 0x00, 0x01, 0x80, 0x03, 0xc0, 0x07, 0xe0, 0x0f, 0xf0, 0x1f,
   0x80, 0x03, 0x80, 0x03, 0x80, 0x03, 0x80, 0x03, 0x80, 0x03, 0x80, 0x03,
   0x80, 0x03, 0x80, 0x03, 0x00, 0x00, 0x00, 0x00};
#define zoom_width 16
#define zoom_height 16
static unsigned char zoom_bits[] = {
   0x00, 0x00, 0x70, 0x00, 0xd8, 0x00, 0x8c, 0x01, 0x06, 0x03, 0x02, 0x02,
   0x06, 0x03, 0x8c, 0x01, 0xd8, 0x03, 0x70, 0x07, 0x00, 0x0e, 0x00, 0x1c,
   0x00, 0x38, 0x00, 0x70, 0x00, 0xe0, 0x00, 0x40};

#endif /* __BITMAPS_H_ */
