/***********************************************************

Copyright 1987, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/*****************************************************************

Copyright (c) 1991, 1997 Digital Equipment Corporation, Maynard, Massachusetts.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
DIGITAL EQUIPMENT CORPORATION BE LIABLE FOR ANY CLAIM, DAMAGES, INCLUDING,
BUT NOT LIMITED TO CONSEQUENTIAL OR INCIDENTAL DAMAGES, OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of Digital Equipment Corporation
shall not be used in advertising or otherwise to promote the sale, use or other
dealings in this Software without prior written authorization from Digital
Equipment Corporation.

******************************************************************/


#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#define NEED_EVENTS
#include <X11/Xproto.h>
#include <X11/Xprotostr.h>

#include "misc.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmap.h"
#include "input.h"

#include "dixstruct.h"
#include "mi.h"
#include <X11/Xmd.h>

#include "globals.h"

#ifdef PANORAMIX
#include "panoramiX.h"
#include "panoramiXsrv.h"
#endif

/*
    machine-independent graphics exposure code.  any device that uses
the region package can call this.
*/

#ifndef RECTLIMIT
#define RECTLIMIT 25		/* pick a number, any number > 8 */
#endif

/* miHandleExposures 
    generate a region for exposures for areas that were copied from obscured or
non-existent areas to non-obscured areas of the destination.  Paint the
background for the region, if the destination is a window.

NOTE:
     this should generally be called, even if graphicsExposures is false,
because this is where bits get recovered from backing store.

NOTE:
     added argument 'plane' is used to indicate how exposures from backing
store should be accomplished. If plane is 0 (i.e. no bit plane), CopyArea
should be used, else a CopyPlane of the indicated plane will be used. The
exposing is done by the backing store's GraphicsExpose function, of course.

*/

_X_EXPORT RegionPtr
miHandleExposures(DrawablePtr pSrcDrawable, DrawablePtr pDstDrawable,
		  GCPtr pGC, int srcx, int srcy, int width, int height,
		  int dstx, int dsty, unsigned long plane)
{
    ScreenPtr pscr;
    RegionPtr prgnSrcClip;	/* drawable-relative source clip */
    RegionRec rgnSrcRec;
    RegionPtr prgnDstClip;	/* drawable-relative dest clip */
    RegionRec rgnDstRec;
    BoxRec srcBox;		/* unclipped source */
    RegionRec rgnExposed;	/* exposed region, calculated source-
				   relative, made dst relative to
				   intersect with visible parts of
				   dest and send events to client, 
				   and then screen relative to paint 
				   the window background
				*/
    WindowPtr pSrcWin;
    BoxRec expBox;
    Bool extents;

    /* This prevents warning about pscr not being used. */
    pGC->pScreen = pscr = pGC->pScreen;

    /* avoid work if we can */
    if (!pGC->graphicsExposures &&
	(pDstDrawable->type == DRAWABLE_PIXMAP) &&
	((pSrcDrawable->type == DRAWABLE_PIXMAP) ||
	 (((WindowPtr)pSrcDrawable)->backStorage == NULL)))
	return NULL;
	
    srcBox.x1 = srcx;
    srcBox.y1 = srcy;
    srcBox.x2 = srcx+width;
    srcBox.y2 = srcy+height;

    if (pSrcDrawable->type != DRAWABLE_PIXMAP)
    {
	BoxRec TsrcBox;

	TsrcBox.x1 = srcx + pSrcDrawable->x;
	TsrcBox.y1 = srcy + pSrcDrawable->y;
	TsrcBox.x2 = TsrcBox.x1 + width;
	TsrcBox.y2 = TsrcBox.y1 + height;
	pSrcWin = (WindowPtr) pSrcDrawable;
	if (pGC->subWindowMode == IncludeInferiors)
 	{
	    prgnSrcClip = NotClippedByChildren (pSrcWin);
	    if ((RECT_IN_REGION(pscr, prgnSrcClip, &TsrcBox)) == rgnIN)
	    {
		REGION_DESTROY(pscr, prgnSrcClip);
		return NULL;
	    }
	}
 	else
 	{
	    if ((RECT_IN_REGION(pscr, &pSrcWin->clipList, &TsrcBox)) == rgnIN)
		return NULL;
	    prgnSrcClip = &rgnSrcRec;
	    REGION_NULL(pscr, prgnSrcClip);
	    REGION_COPY(pscr, prgnSrcClip, &pSrcWin->clipList);
	}
	REGION_TRANSLATE(pscr, prgnSrcClip,
				-pSrcDrawable->x, -pSrcDrawable->y);
    }
    else
    {
	BoxRec	box;

	if ((srcBox.x1 >= 0) && (srcBox.y1 >= 0) &&
	    (srcBox.x2 <= pSrcDrawable->width) &&
 	    (srcBox.y2 <= pSrcDrawable->height))
	    return NULL;

	box.x1 = 0;
	box.y1 = 0;
	box.x2 = pSrcDrawable->width;
	box.y2 = pSrcDrawable->height;
	prgnSrcClip = &rgnSrcRec;
	REGION_INIT(pscr, prgnSrcClip, &box, 1);
	pSrcWin = (WindowPtr)NULL;
    }

    if (pDstDrawable == pSrcDrawable)
    {
	prgnDstClip = prgnSrcClip;
    }
    else if (pDstDrawable->type != DRAWABLE_PIXMAP)
    {
	if (pGC->subWindowMode == IncludeInferiors)
	{
	    prgnDstClip = NotClippedByChildren((WindowPtr)pDstDrawable);
	}
	else
	{
	    prgnDstClip = &rgnDstRec;
	    REGION_NULL(pscr, prgnDstClip);
	    REGION_COPY(pscr, prgnDstClip,
				&((WindowPtr)pDstDrawable)->clipList);
	}
	REGION_TRANSLATE(pscr, prgnDstClip,
				 -pDstDrawable->x, -pDstDrawable->y);
    }
    else
    {
	BoxRec	box;

	box.x1 = 0;
	box.y1 = 0;
	box.x2 = pDstDrawable->width;
	box.y2 = pDstDrawable->height;
	prgnDstClip = &rgnDstRec;
	REGION_INIT(pscr, prgnDstClip, &box, 1);
    }

    /* drawable-relative source region */
    REGION_INIT(pscr, &rgnExposed, &srcBox, 1);

    /* now get the hidden parts of the source box*/
    REGION_SUBTRACT(pscr, &rgnExposed, &rgnExposed, prgnSrcClip);

    /* move them over the destination */
    REGION_TRANSLATE(pscr, &rgnExposed, dstx-srcx, dsty-srcy);

    /* intersect with visible areas of dest */
    REGION_INTERSECT(pscr, &rgnExposed, &rgnExposed, prgnDstClip);

    /*
     * If we have LOTS of rectangles, we decide to take the extents
     * and force an exposure on that.  This should require much less
     * work overall, on both client and server.  This is cheating, but
     * isn't prohibited by the protocol ("spontaneous combustion" :-)
     * for windows.
     */
    extents = pGC->graphicsExposures &&
	      (REGION_NUM_RECTS(&rgnExposed) > RECTLIMIT) &&
	      (pDstDrawable->type != DRAWABLE_PIXMAP);
    if (pSrcWin)
    {
	RegionPtr	region;
    	if (!(region = wClipShape (pSrcWin)))
    	    region = wBoundingShape (pSrcWin);
    	/*
     	 * If you try to CopyArea the extents of a shaped window, compacting the
     	 * exposed region will undo all our work!
     	 */
    	if (extents && pSrcWin && region &&
    	    (RECT_IN_REGION(pscr, region, &srcBox) != rgnIN))
	    	extents = FALSE;
    }
    if (extents)
    {
	expBox = *REGION_EXTENTS(pscr, &rgnExposed);
	REGION_RESET(pscr, &rgnExposed, &expBox);
    }
    if ((pDstDrawable->type != DRAWABLE_PIXMAP) &&
	(((WindowPtr)pDstDrawable)->backgroundState != None))
    {
	WindowPtr pWin = (WindowPtr)pDstDrawable;

	/* make the exposed area screen-relative */
	REGION_TRANSLATE(pscr, &rgnExposed, 
				 pDstDrawable->x, pDstDrawable->y);

	if (extents)
	{
	    /* PaintWindowBackground doesn't clip, so we have to */
	    REGION_INTERSECT(pscr, &rgnExposed, &rgnExposed, &pWin->clipList);
	}
	miPaintWindow((WindowPtr)pDstDrawable, &rgnExposed, PW_BACKGROUND);

	if (extents)
	{
	    REGION_RESET(pscr, &rgnExposed, &expBox);
	}
	else
	    REGION_TRANSLATE(pscr, &rgnExposed,
				     -pDstDrawable->x, -pDstDrawable->y);
    }
    if (prgnDstClip == &rgnDstRec)
    {
	REGION_UNINIT(pscr, prgnDstClip);
    }
    else if (prgnDstClip != prgnSrcClip)
    {
	REGION_DESTROY(pscr, prgnDstClip);
    }

    if (prgnSrcClip == &rgnSrcRec)
    {
	REGION_UNINIT(pscr, prgnSrcClip);
    }
    else
    {
	REGION_DESTROY(pscr, prgnSrcClip);
    }

    if (pGC->graphicsExposures)
    {
	/* don't look */
	RegionPtr exposed = REGION_CREATE(pscr, NullBox, 0);
	*exposed = rgnExposed;
	return exposed;
    }
    else
    {
	REGION_UNINIT(pscr, &rgnExposed);
	return NULL;
    }
}

/* send GraphicsExpose events, or a NoExpose event, based on the region */

void
miSendGraphicsExpose (ClientPtr client, RegionPtr pRgn, XID drawable,
                      int major, int minor)
{
    if (pRgn && !REGION_NIL(pRgn))
    {
        xEvent *pEvent;
	xEvent *pe;
	BoxPtr pBox;
	int i;
	int numRects;

	numRects = REGION_NUM_RECTS(pRgn);
	pBox = REGION_RECTS(pRgn);
	if(!(pEvent = (xEvent *)xalloc(numRects * sizeof(xEvent))))
		return;
	pe = pEvent;

	for (i=1; i<=numRects; i++, pe++, pBox++)
	{
	    pe->u.u.type = GraphicsExpose;
	    pe->u.graphicsExposure.drawable = drawable;
	    pe->u.graphicsExposure.x = pBox->x1;
	    pe->u.graphicsExposure.y = pBox->y1;
	    pe->u.graphicsExposure.width = pBox->x2 - pBox->x1;
	    pe->u.graphicsExposure.height = pBox->y2 - pBox->y1;
	    pe->u.graphicsExposure.count = numRects - i;
	    pe->u.graphicsExposure.majorEvent = major;
	    pe->u.graphicsExposure.minorEvent = minor;
	}
	TryClientEvents(client, NULL, pEvent, numRects,
			    (Mask)0, NoEventMask, NullGrab);
	xfree(pEvent);
    }
    else
    {
        xEvent event;
	event.u.u.type = NoExpose;
	event.u.noExposure.drawable = drawable;
	event.u.noExposure.majorEvent = major;
	event.u.noExposure.minorEvent = minor;
	TryClientEvents(client, NULL, &event, 1,
	    (Mask)0, NoEventMask, NullGrab);
    }
}


void
miSendExposures( WindowPtr pWin, RegionPtr pRgn, int dx, int dy)
{
    BoxPtr pBox;
    int numRects;
    xEvent *pEvent, *pe;
    int i;

    pBox = REGION_RECTS(pRgn);
    numRects = REGION_NUM_RECTS(pRgn);
    if(!(pEvent = (xEvent *) xalloc(numRects * sizeof(xEvent))))
	return;

    for (i=numRects, pe = pEvent; --i >= 0; pe++, pBox++)
    {
	pe->u.u.type = Expose;
	pe->u.expose.window = pWin->drawable.id;
	pe->u.expose.x = pBox->x1 - dx;
	pe->u.expose.y = pBox->y1 - dy;
	pe->u.expose.width = pBox->x2 - pBox->x1;
	pe->u.expose.height = pBox->y2 - pBox->y1;
	pe->u.expose.count = i;
    }

#ifdef PANORAMIX
    if(!noPanoramiXExtension) {
	int scrnum = pWin->drawable.pScreen->myNum;
	int x = 0, y = 0;
	XID realWin = 0;

	if(!pWin->parent) {
	    x = panoramiXdataPtr[scrnum].x;
	    y = panoramiXdataPtr[scrnum].y;
	    pWin = WindowTable[0];
	    realWin = pWin->drawable.id;
	} else if (scrnum) {
	    PanoramiXRes *win;
	    win = PanoramiXFindIDByScrnum(XRT_WINDOW, 
			pWin->drawable.id, scrnum);
	    if(!win) {
		xfree(pEvent);
		return;
	    }
	    realWin = win->info[0].id;
	    pWin = LookupIDByType(realWin, RT_WINDOW);
	}
	if(x || y || scrnum)
	  for (i = 0; i < numRects; i++) {
	      pEvent[i].u.expose.window = realWin;
	      pEvent[i].u.expose.x += x;
	      pEvent[i].u.expose.y += y;
	  }
    }
#endif

    DeliverEvents(pWin, pEvent, numRects, NullWindow);

    xfree(pEvent);
}

_X_EXPORT void 
miWindowExposures( WindowPtr pWin, RegionPtr prgn, RegionPtr other_exposed)
{
    RegionPtr   exposures = prgn;
    if ((prgn && !REGION_NIL(prgn)) || 
	(exposures && !REGION_NIL(exposures)) || other_exposed)
    {
	RegionRec   expRec;
	int	    clientInterested;

	/*
	 * Restore from backing-store FIRST.
	 */
	clientInterested = (pWin->eventMask|wOtherEventMasks(pWin)) & ExposureMask;
	if (other_exposed)
	{
	    if (exposures)
	    {
		REGION_UNION(pWin->drawable.pScreen, other_exposed,
						  exposures,
					          other_exposed);
		if (exposures != prgn)
		    REGION_DESTROY(pWin->drawable.pScreen, exposures);
	    }
	    exposures = other_exposed;
	}
	if (clientInterested && exposures && (REGION_NUM_RECTS(exposures) > RECTLIMIT))
	{
	    /*
	     * If we have LOTS of rectangles, we decide to take the extents
	     * and force an exposure on that.  This should require much less
	     * work overall, on both client and server.  This is cheating, but
	     * isn't prohibited by the protocol ("spontaneous combustion" :-).
	     */
	    BoxRec box;

	    box = *REGION_EXTENTS( pWin->drawable.pScreen, exposures);
	    if (exposures == prgn) {
		exposures = &expRec;
		REGION_INIT( pWin->drawable.pScreen, exposures, &box, 1);
		REGION_RESET( pWin->drawable.pScreen, prgn, &box);
	    } else {
		REGION_RESET( pWin->drawable.pScreen, exposures, &box);
		REGION_UNION( pWin->drawable.pScreen, prgn, prgn, exposures);
	    }
	    /* PaintWindowBackground doesn't clip, so we have to */
	    REGION_INTERSECT( pWin->drawable.pScreen, prgn, prgn, &pWin->clipList);
	}
	if (prgn && !REGION_NIL(prgn))
	    miPaintWindow(pWin, prgn, PW_BACKGROUND);
	if (clientInterested && exposures && !REGION_NIL(exposures))
	    miSendExposures(pWin, exposures,
			    pWin->drawable.x, pWin->drawable.y);
	if (exposures == &expRec)
	{
	    REGION_UNINIT( pWin->drawable.pScreen, exposures);
	}
	else if (exposures && exposures != prgn && exposures != other_exposed)
	    REGION_DESTROY( pWin->drawable.pScreen, exposures);
	if (prgn)
	    REGION_EMPTY( pWin->drawable.pScreen, prgn);
    }
    else if (exposures && exposures != prgn)
	REGION_DESTROY( pWin->drawable.pScreen, exposures);
}

#ifdef ROOTLESS
/* Ugly, ugly, but we lost our hooks into miPaintWindow... =/ */
void RootlessSetPixmapOfAncestors(WindowPtr pWin);
void RootlessStartDrawing(WindowPtr pWin);
void RootlessDamageRegion(WindowPtr pWin, RegionPtr prgn);
Bool IsFramedWindow(WindowPtr pWin);
#endif 

void
miPaintWindow(WindowPtr pWin, RegionPtr prgn, int what)
{
    ScreenPtr	pScreen = pWin->drawable.pScreen;
    ChangeGCVal gcval[6];
    BITS32	gcmask;
    GCPtr	pGC;
    int		i;
    BoxPtr	pbox;
    xRectangle	*prect;
    int		numRects;
    /*
     * Distance from screen to destination drawable, use this
     * to adjust rendering coordinates which come in in screen space
     */
    int		draw_x_off, draw_y_off;
    /*
     * Tile offset for drawing; these need to align the tile
     * to the appropriate window origin
     */
    int		tile_x_off, tile_y_off;
    PixUnion	fill;
    Bool	solid = TRUE;
    DrawablePtr	drawable = &pWin->drawable;

#ifdef ROOTLESS
    if(IsFramedWindow(pWin)) {
        RootlessStartDrawing(pWin);
        RootlessDamageRegion(pWin, prgn);
    
        if(pWin->backgroundState == ParentRelative) {
            if((what == PW_BACKGROUND) || 
               (what == PW_BORDER && !pWin->borderIsPixel))
                RootlessSetPixmapOfAncestors(pWin);
        }
    }
#endif
    
    if (what == PW_BACKGROUND)
    {
	while (pWin->backgroundState == ParentRelative)
	    pWin = pWin->parent;

	draw_x_off = drawable->x;
	draw_y_off = drawable->y;

	tile_x_off = pWin->drawable.x - draw_x_off;
	tile_y_off = pWin->drawable.y - draw_y_off;
	fill = pWin->background;
	switch (pWin->backgroundState) {
	case None:
	    return;
	case BackgroundPixmap:
	    solid = FALSE;
	    break;
	}
    }
    else
    {
	PixmapPtr   pixmap;

	tile_x_off = drawable->x;
	tile_y_off = drawable->y;
	
	/* servers without pixmaps draw their own borders */
	if (!pScreen->GetWindowPixmap)
	    return;
	pixmap = (*pScreen->GetWindowPixmap) ((WindowPtr) drawable);
	drawable = &pixmap->drawable;
#ifdef COMPOSITE
	draw_x_off = pixmap->screen_x;
	draw_y_off = pixmap->screen_y;
	tile_x_off -= draw_x_off;
	tile_y_off -= draw_y_off;
#else
	draw_x_off = 0;
	draw_y_off = 0;
#endif
	fill = pWin->border;
	solid = pWin->borderIsPixel;
    }
    
    gcval[0].val = GXcopy;
    gcmask = GCFunction;

#ifdef ROOTLESS_SAFEALPHA
/* Bit mask for alpha channel with a particular number of bits per
 * pixel. Note that we only care for 32bpp data. Mac OS X uses planar
 * alpha for 16bpp.
 */
#define RootlessAlphaMask(bpp) ((bpp) == 32 ? 0xFF000000 : 0)
#endif
    
    if (solid)
    {
#ifdef ROOTLESS_SAFEALPHA
	gcval[1].val = fill.pixel | RootlessAlphaMask(pWin->drawable.bitsPerPixel);
#else
	gcval[1].val = fill.pixel;
#endif
	gcval[2].val  = FillSolid;
	gcmask |= GCForeground | GCFillStyle;
    }
    else
    {
	int c=1;
#ifdef ROOTLESS_SAFEALPHA
	gcval[c++].val = ((CARD32)-1) & ~RootlessAlphaMask(pWin->drawable.bitsPerPixel);
	gcmask |= GCPlaneMask;
#endif
	gcval[c++].val = FillTiled;
	gcval[c++].ptr = (pointer)fill.pixmap;
	gcval[c++].val = tile_x_off;
	gcval[c++].val = tile_y_off;
	gcmask |= GCFillStyle | GCTile | GCTileStipXOrigin | GCTileStipYOrigin;
    }

    prect = (xRectangle *)xalloc(REGION_NUM_RECTS(prgn) *
					 sizeof(xRectangle));
    if (!prect)
	return;

    pGC = GetScratchGC(drawable->depth, drawable->pScreen);
    if (!pGC)
    {
	xfree(prect);
	return;
    }
    
    dixChangeGC (NullClient, pGC, gcmask, NULL, gcval);
    ValidateGC (drawable, pGC);

    numRects = REGION_NUM_RECTS(prgn);
    pbox = REGION_RECTS(prgn);
    for (i= numRects; --i >= 0; pbox++, prect++)
    {
	prect->x = pbox->x1 - draw_x_off;
	prect->y = pbox->y1 - draw_y_off;
	prect->width = pbox->x2 - pbox->x1;
	prect->height = pbox->y2 - pbox->y1;
    }
    prect -= numRects;
    (*pGC->ops->PolyFillRect)(drawable, pGC, numRects, prect);
    xfree(prect);

    FreeScratchGC(pGC);
}


/* MICLEARDRAWABLE -- sets the entire drawable to the background color of
 * the GC.  Useful when we have a scratch drawable and need to initialize 
 * it. */
_X_EXPORT void
miClearDrawable(DrawablePtr pDraw, GCPtr pGC)
{
    XID fg = pGC->fgPixel;
    XID bg = pGC->bgPixel;
    xRectangle rect;

    rect.x = 0;
    rect.y = 0;
    rect.width = pDraw->width;
    rect.height = pDraw->height;
    DoChangeGC(pGC, GCForeground, &bg, 0);
    ValidateGC(pDraw, pGC);
    (*pGC->ops->PolyFillRect)(pDraw, pGC, 1, &rect);
    DoChangeGC(pGC, GCForeground, &fg, 0);
    ValidateGC(pDraw, pGC);
}
