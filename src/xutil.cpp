/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2001 Grace Development Team
 * 
 * Maintained by Evgeny Stambulchik
 * 
 * Modified by Andreas Winter 2008-2011
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

///#include <config.h>

#include <stdlib.h>
#include <unistd.h>
#include <QtGui>

#include "MainWindow.h"

/*#include <X11/Xlib.h>
#include <X11/cursorfont.h>*/

#include "defines.h"
#include "globals.h"

#include "utils.h"
#include "files.h"
#include "graphs.h"
#include "graphutils.h"
#include "plotone.h"
#include "device.h"

#include "x11drv.h"

#include "xprotos.h"
#include "noxprotos.h"
//#include <QRubberBand>

///extern Window root, xwin;
///extern Display *disp;
///extern Widget app_shell;
///extern XtAppContext app_con;
///extern GC gc, gcxor;
extern int depth;

static QPixmap * bufpixmap;// = (Pixmap) NULL;

extern QPainter * GeneralPainter;
extern MainWindow * mainWin;
extern QImage * MainPixmap;

extern int win_h, win_w;	/* declared in x11drv.c */

extern int inpipe;
extern char batchfile[];

extern Input_buffer *ib_tbl;
extern int ib_tblsize;

extern QImage paintXOR(QImage * canvas,QImage * toDraw);

/*
 * cursors
 */
extern QCursor * wait_cursor;
extern QCursor * line_cursor;
extern QCursor * find_cursor;
extern QCursor * move_cursor;
extern QCursor * text_cursor;
extern QCursor * kill_cursor;
extern QCursor * what_cursor;
extern int cur_cursor;

///static void xmonitor_rti(XtPointer ib, int *ptrFd, XtInputId *ptrId);

int nr_rubber_lines;
int alloc_rubber_lines;
QRubberBand ** rubber_lines;

void init_rubber_lines()
{
nr_rubber_lines=0;
alloc_rubber_lines=10;
rubber_lines=new QRubberBand*[alloc_rubber_lines];
	for (int i=0;i<alloc_rubber_lines;i++)
	{
	rubber_lines[i]=new QRubberBand(QRubberBand::Line,mainWin->mainArea);
	rubber_lines[i]->hide();
	}
}

void delete_rubber_lines()
{
	for (int i=0;i<alloc_rubber_lines;i++)
	{
	delete rubber_lines[i];
	}
	delete[] rubber_lines;
}

void hide_rubber_lines()
{
	for (int i=0;i<alloc_rubber_lines;i++)
	{
	rubber_lines[i]->hide();
	}
	nr_rubber_lines=0;
}

void more_rubber_lines(void)
{
QRubberBand ** nrl=new QRubberBand*[alloc_rubber_lines]; 
	for (int i=0;i<alloc_rubber_lines;i++)
	nrl[i]=rubber_lines[i];
	for (int i=0;i<alloc_rubber_lines;i++)
	delete[] rubber_lines;
	rubber_lines=new QRubberBand*[alloc_rubber_lines+10];
	for (int i=0;i<alloc_rubber_lines;i++)
	rubber_lines[i]=nrl[i];
	for (int i=alloc_rubber_lines;i<alloc_rubber_lines+10;i++)
	{
	rubber_lines[i]=new QRubberBand(QRubberBand::Line,mainWin->mainArea);
	rubber_lines[i]->hide();
	}
alloc_rubber_lines+=10;
delete[] nrl;
}

void DefineDialogCursor(QCursor c);
void UndefineDialogCursor();

void set_wait_cursor()
{
    if (mainWin!=NULL)
    {
    ///DefineDialogCursor(wait_cursor);
    mainWin->setCursor(*wait_cursor);
    }
}

void unset_wait_cursor()
{
    if (mainWin!=NULL)
    {
    ///UndefineDialogCursor();
    //mainWin->setCursor(Qt::ArrowCursor);
    mainWin->unsetCursor();
    if (cur_cursor >= 0) {
        set_cursor(cur_cursor);
    }
    }
}

void set_cursor(int c)
{
    if (mainWin==NULL) return;
    ///XUndefineCursor(disp, xwin);
    cur_cursor = c;
    switch (c) {
    case 0:
	mainWin->setCursor(*line_cursor);
        ///XDefineCursor(disp, xwin, line_cursor);
        break;
    case 1:
	mainWin->setCursor(*find_cursor);
        ///XDefineCursor(disp, xwin, find_cursor);
        break;
    case 2:
	mainWin->setCursor(*text_cursor);
        ///XDefineCursor(disp, xwin, text_cursor);
        break;
    case 3:
	mainWin->setCursor(*kill_cursor);
        ///XDefineCursor(disp, xwin, kill_cursor);
        break;
    case 4:
	mainWin->setCursor(*move_cursor);
        ///XDefineCursor(disp, xwin, move_cursor);
        break;
    case 5:
        mainWin->setCursor(*what_cursor);
        break;
    default:
        cur_cursor = -1;
	mainWin->unsetCursor();
        break;
    }
    ///XFlush(disp);
}

void init_cursors(void)
{
    wait_cursor = new QCursor(Qt::WaitCursor);
    line_cursor = new QCursor(Qt::CrossCursor);
    find_cursor = new QCursor(Qt::PointingHandCursor);
    text_cursor = new QCursor(Qt::IBeamCursor);
    kill_cursor = new QCursor(Qt::ForbiddenCursor);///TODO:load spezial kill-cursor
    move_cursor = new QCursor(Qt::SizeAllCursor);
    what_cursor = new QCursor(Qt::WhatsThisCursor);
    /*wait_cursor = XCreateFontCursor(disp, XC_watch);
    line_cursor = XCreateFontCursor(disp, XC_crosshair);
    find_cursor = XCreateFontCursor(disp, XC_hand2);
    text_cursor = XCreateFontCursor(disp, XC_xterm);
    kill_cursor = XCreateFontCursor(disp, XC_pirate);
    move_cursor = XCreateFontCursor(disp, XC_fleur);*/
    cur_cursor = -1;
}


/*
 * put a string in the title bar
 */
void set_title(char *ts)
{
    static char *ts_save = NULL;
    static int dstate_save = 0;
    int dstate = is_dirtystate();
    /*if (!inwin || ts == NULL) {
	cout << "set title" << endl;
        return;
    } else*/
    if (ts_save == NULL || strcmp(ts_save, ts) != 0 || dstate != dstate_save) {
        char *buf1, *buf2;
        ts_save = copy_string(ts_save, ts);
        dstate_save = dstate;
        buf1 = copy_string(NULL, "qtGrace: ");
        buf1 = concat_strings(buf1, ts);
        buf2 = copy_string(NULL, ts);
        if (dstate) {
            buf2 = concat_strings(buf2, "*");
            buf1 = concat_strings(buf1, " (modified)");
        }
        ///XtVaSetValues(app_shell, XtNtitle, buf1, XtNiconName, buf2, NULL);
mainWin->setWindowTitle(QString(buf1));
        xfree(buf1);
        xfree(buf2);
    }
}

/*
 *  Auxiliary routines for simultaneous drawing on display and pixmap
 */
static void aux_XDrawLine(int x1, int y1, int x2, int y2)
{
int startx=MIN2(x1,x2),starty=MIN2(y1,y2),c_width=abs(x2-x1),c_height=abs(y2-y1),endx=MAX2(x1,x2),endy=MAX2(y1,y2);
GeneralPainter->end();
QImage hd_copy(MainPixmap->copy(startx,starty,c_width,c_height));//get area that will be painted on
QImage content(hd_copy);
QPainter painter(&content);
painter.setBrush(Qt::white);
painter.drawRect(0,0,c_width,c_height);
painter.setPen(GeneralPainter->pen());
///temporarely changed
painter.setPen(Qt::black);

painter.drawLine(0,0,endx-startx,endy-starty);
painter.end();
QImage res(paintXOR(&hd_copy,&content));
GeneralPainter->begin(MainPixmap);
GeneralPainter->drawImage(startx,starty,res);
//GeneralPainter->drawLine(x1,y1,x2,y2);
    /*///XDrawLine(disp, xwin, gcxor, x1, y1, x2, y2);
    if (bufpixmap != (Pixmap) NULL) {
        ///XDrawLine(disp, bufpixmap, gcxor, x1, y1, x2, y2);
    }*/
}

static void aux_XDrawRectangle(int x1, int y1, int x2, int y2)
{
int startx=MIN2(x1,x2),starty=MIN2(y1,y2),c_width=abs(x2-x1),c_height=abs(y2-y1),endx=MAX2(x1,x2),endy=MAX2(y1,y2);
GeneralPainter->end();
QImage hd_copy(MainPixmap->copy(startx,starty,c_width,c_height));//get area that will be painted on
QImage content(hd_copy);
QPainter painter(&content);
painter.setBrush(Qt::white);//in XOR,, white means: no content
painter.drawRect(0,0,c_width,c_height);

//painter.setPen(GeneralPainter->pen());
///temporarely changed
painter.setPen(Qt::black);

painter.drawRect(0,0,c_width-1,c_height-1);
painter.end();
QImage res(paintXOR(&hd_copy,&content));
GeneralPainter->begin(MainPixmap);
GeneralPainter->drawImage(startx,starty,res);
//GeneralPainter->drawRect(MIN2(x1,x2),MIN2(y1,y2),abs(x2-x1),abs(y1-y2));
    /*///XDrawRectangle(disp, xwin, gcxor, x1, y1, x2, y2);
    if (bufpixmap != (Pixmap) NULL) {
        ///XDrawRectangle(disp, bufpixmap, gcxor, x1, y1, x2, y2);
    }*/
}

static void aux_XFillRectangle(int x, int y, unsigned int width, unsigned int height)
{
GeneralPainter->end();
QImage hd_copy(MainPixmap->copy(x,y,width,height));//get area that will be painted on
QImage content(hd_copy);
QPainter painter;//(&content);
painter.begin(&content);
////painter.setBrush(GeneralPainter->brush());
///temporarely changed
painter.setBrush(Qt::black);

painter.drawRect(0,0,width,height);
painter.end();
QImage res(paintXOR(&hd_copy,&content));
GeneralPainter->begin(MainPixmap);
GeneralPainter->drawImage(x,y,res);
//simple case --> no longer necessary
//GeneralPainter->drawRect(x,y,width,height);
    /*///XFillRectangle(disp, xwin, gcxor, x, y, width, height);
    if (bufpixmap != (Pixmap) NULL) {
        ///XFillRectangle(disp, bufpixmap, gcxor, x, y, width, height);
    }*/
}

/*
 * draw the graph focus indicators
 */
void draw_focus(int gno)
{
    int ix1, iy1, ix2, iy2;
    view v;
    VPoint vp;
    
    if (draw_focus_flag == TRUE) {
        get_graph_viewport(gno, &v);
        vp.x = v.xv1;
        vp.y = v.yv1;
        xlibVPoint2dev(vp, &ix1, &iy1);
        vp.x = v.xv2;
        vp.y = v.yv2;
        xlibVPoint2dev(vp, &ix2, &iy2);
        aux_XFillRectangle(ix1 - 5, iy1 - 5, 10, 10);
        aux_XFillRectangle(ix1 - 5, iy2 - 5, 10, 10);
        aux_XFillRectangle(ix2 - 5, iy2 - 5, 10, 10);
        aux_XFillRectangle(ix2 - 5, iy1 - 5, 10, 10);
    }
}

/*
 * rubber band line (optionally erasing previous one)
 */
void select_line(int x1, int y1, int x2, int y2, int erase)
{
    static int x1_old, y1_old, x2_old, y2_old;

    if (erase) {
        ///aux_XDrawLine(x1_old, y1_old, x2_old, y2_old);
    }
    x1_old = x1;
    y1_old = y1;
    x2_old = x2;
    y2_old = y2;
    VPoint vp;
    vp = xlibdev2VPoint(x1, y1);
    rg[MAXREGION].type=REGION_POLYI;
    rg[MAXREGION].n=2;
    if (rg[MAXREGION].x==NULL)
    {
    rg[MAXREGION].x=new double[2];
    rg[MAXREGION].y=new double[2];
    }
    view2world(vp.x,vp.y,&rg[MAXREGION].x[0], &rg[MAXREGION].y[0]);
    vp = xlibdev2VPoint(x2, y2);
    view2world(vp.x,vp.y,&rg[MAXREGION].x[1], &rg[MAXREGION].y[1]);
    /*
    rg[MAXREGION].x1=x1;
    rg[MAXREGION].y1=y1;
    rg[MAXREGION].x2=x2;
    rg[MAXREGION].y2=y2;*/
    rg[MAXREGION].active=TRUE;
    ///aux_XDrawLine(x1, y1, x2, y2);
	
        /*
        if (nr_rubber_lines==alloc_rubber_lines)
	{
	cout << "more rubber lines" << endl;
	more_rubber_lines();
	}
	///rubber_lines[nr_rubber_lines]
        nr_rubber_lines++;*/
	
}


/*
 * draw an xor'ed box (optionally erasing previous one)
 */
void select_region(int x1, int y1, int x2, int y2, int erase)
{
    static int x1_old, y1_old, dx_old, dy_old;
    int dx = x2 - x1;
    int dy = y2 - y1;

    if (dx < 0) {
	iswap(&x1, &x2);
	dx = -dx;
    }
    if (dy < 0) {
	iswap(&y1, &y2);
	dy = -dy;
    }
    if (erase) {
        ///aux_XDrawRectangle(x1_old, y1_old, dx_old, dy_old);
    }
    x1_old = x1;
    y1_old = y1;
    dx_old = dx;
    dy_old = dy;
    ///aux_XDrawRectangle(x1, y1, dx, dy);
}

/*
 * slide an xor'ed bbox shifted by shift_*, (optionally erasing previous one)
 */
void slide_region(view bb, int shift_x, int shift_y, int erase)
{
    int x1, x2;
    int y1, y2;
    VPoint vp;

    vp.x = bb.xv1;
    vp.y = bb.yv1;
    xlibVPoint2dev(vp, &x1, &y1);
    x1 += shift_x;
    y1 += shift_y;
    
    vp.x = bb.xv2;
    vp.y = bb.yv2;
    xlibVPoint2dev(vp, &x2, &y2);
    x2 += shift_x;
    y2 += shift_y;
    
    select_region(x1, y1, x2, y2, erase);
}

static int crosshair_erase = FALSE;

void reset_crosshair(void)
{
    crosshair_erase = FALSE;
}

/*
 * draw a crosshair cursor
 */
void crosshair_motion(int x, int y)
{
    //static int cursor_oldx, cursor_oldy;
    
    /* Erase the previous crosshair */
    /*if (crosshair_erase == TRUE) {
        aux_XDrawLine(0, cursor_oldy, win_w, cursor_oldy);
        aux_XDrawLine(cursor_oldx, 0, cursor_oldx, win_h);
    }*/
mainWin->mainArea->completeRedraw();
    /* Draw the new crosshair */
    /*aux_XDrawLine(0, y, win_w, y);
    aux_XDrawLine(x, 0, x, win_h);
    crosshair_erase = TRUE;
    cursor_oldx = x;
    cursor_oldy = y;*/
}


/*
 * expose/resize proc
 */
/*void expose_resize(Widget w, XtPointer client_data,XmDrawingAreaCallbackStruct *cbs)
{
    static int inc = 0;

#if defined(DEBUG)
    if (get_debuglevel() == 7) {
	printf("Call to expose_resize(); reason == %d\n", cbs->reason);
    }
#endif
    
    // HACK 
    if (xwin == 0) {
        return;
    }
    
    if (!inc) {
	inwin = TRUE;
	inc++;
        
	if (batchfile[0]) {
            getparms(batchfile);
	}
	
	if (inpipe == TRUE) {
	    getdata(get_cg(), "stdin", SOURCE_DISK, LOAD_SINGLE);
	    inpipe = FALSE;
	}

        update_all();
        drawgraph();

        return;
    }
    
    if (cbs->reason == XmCR_EXPOSE) {
  	xlibredraw(cbs->window, cbs->event->xexpose.x,
                                cbs->event->xexpose.y,
                                cbs->event->xexpose.width,
                                cbs->event->xexpose.height);
        return;
    }
    
    if (get_pagelayout() == PAGE_FREE) {
        unsigned int w, h;
        sync_canvas_size(&w, &h, TRUE);
        drawgraph();
    }
}*/

/* 
 * redraw all
 */
void xdrawgraph(void)
{
//cout << "inwin="<< inwin << endl;
    ///if (inwin && (auto_redraw)) {
        set_wait_cursor();
        drawgraph();
	unset_wait_cursor();
    ///}
}


void xlibredraw(MainArea * ma, int x, int y, int width, int height)
{
///cout << "xlibredraw" << endl;
///bufpixmap != (Pixmap) NULL
    if (inwin == TRUE && 1) {
        ///XCopyArea(disp, bufpixmap, window, gc, x, y, width, height, x, y);
    }
}

QPixmap resize_bufpixmap(unsigned int w, unsigned int h)
{
    /*static unsigned int pixmap_w = 0, pixmap_h = 0;
    
    if (w == 0 || h == 0) {
        return (*bufpixmap);
    }
    ///Pixmap=leer
    if (0) {
        ///bufpixmap = XCreatePixmap(disp, root, w, h, depth);
    } else if (pixmap_w != w || pixmap_h != h) {
        ///XFreePixmap(disp, bufpixmap);
        ///bufpixmap = XCreatePixmap(disp, root, w, h, depth);
    }
    
///bufpixmap == (Pixmap) NULL
    if (0) {
        errmsg("Can't allocate buffer pixmap");
        pixmap_w = 0;
        pixmap_h = 0;
        return (*bufpixmap);
	///return (xwin);
    } else {
        pixmap_w = w;
        pixmap_h = h;
        return (*bufpixmap);
    }*/

 return (*bufpixmap);
}

/*static void xmonitor_rti(XtPointer ib, int *ptrFd, XtInputId *ptrId)
{
    set_wait_cursor();
    
    monitor_input((Input_buffer *) ib, 1, 1);
    
    unset_wait_cursor();
}*/

/*void xunregister_rti(XtInputId id)
{
    if (disp != (Display *) NULL) {
        // the screen has been initialized : we can remove the buffer 
        ///XtRemoveInput(id);
    }
}*/

/*void xregister_rti(Input_buffer *ib)
{
    if (disp != (Display *) NULL) {
        // the screen has been initialized : we can register the buffer 
        ib->id = (unsigned long) XtAppAddInput(app_con,
                                               ib->fd,
                                               (XtPointer) XtInputReadMask,
                                               xmonitor_rti,
                                               (XtPointer) ib);
    }
}*/

/*
 * for the goto point feature
 */
void setpointer(VPoint vp)
{
    int x, y;
    
    xlibVPoint2dev(vp, &x, &y);
    
    /* Make sure we remain inside the DA widget dimensions */
    x = MAX2(x, 0);
    x = MIN2(x, win_w)+4;
    y = MAX2(y, 0);
    y = MIN2(y, win_h)+4;
    QCursor::setPos(mainWin->mainArea->mapToGlobal(QPoint(x,y)));
    ///XWarpPointer(disp, None, xwin, 0, 0, 0, 0, x, y);
}

char *display_name(void)
{
    //return DisplayString(disp);
    return QDesktopServices::displayName(QDesktopServices::DesktopLocation).toAscii().data();
}
