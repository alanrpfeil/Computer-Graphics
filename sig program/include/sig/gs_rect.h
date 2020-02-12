/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

/** \file gs_rect.h
 * Axis-aligned 2D rectangle management class
 */

# ifndef GS_RECT_H
# define GS_RECT_H

# include <sig/gs_vec2.h>
# include <sig/gs_output.h>

/*! GsRect represents an axis-aligned 2D rectangle by given lowest-values position (x,y) and dimension (w,h) parameters.
	In windows coordinates (x,y) is the top-left corner, in scene coordinates (x,y) is the bottom-left corner.
 */
class GsRect
{  public :
	float x, y, w, h;
   public :
	GsRect () { x=y=w=h=0; }
	GsRect ( const GsRect& r ) { x=r.x; y=r.y; w=r.w; h=r.h; }
	GsRect ( int rx, int ry, int rw, int rh ) { x=(float)rx; y=(float)ry; w=(float)rw; h=(float)rh; }
	GsRect ( float rx, float ry, float rw, float rh ) { x=rx; y=ry; w=rw; h=rh; }
	float xp () const { return x+w; } // x prime is the maximum x coordinate of the rectangle
	float yp () const { return y+h; } // y prime is the maximum y coordinate of the rectangle
	void set ( float rx, float ry, float rw, float rh ) { x=rx; y=ry; w=rw; h=rh; }
	void set ( int rx, int ry, int rw, int rh ) { x=(float)rx; y=(float)ry; w=(float)rw; h=(float)rh; }
	void size ( float rw, float rh ) { w=rw; h=rh; }
	void size ( int rw, int rh ) { w=(float)rw; h=(float)rh; }
	void pos ( float rx, float ry ) { x=rx; y=ry; }
	void pos ( int rx, int ry ) { x=(float)rx; y=(float)ry; }
	void move ( float dx, float dy ) { x+=dx; y+=dy; }
	void grow ( float dw, float dh ) { x-=dw; y-=dh; w+=dw+dw; h+=dh+dh; }
	void set_center ( float cx, float cy ) { x=cx-w/2; y=cy-h/2; }
	void centerw ( float rw ) { x += (rw-w)/2.0f; }
	void centerh ( float rh ) { y += (rh-h)/2.0f; }
	void center ( float rw, float rh ) { centerw(rw); centerh(rh); }
	void round_coordinates ();
	bool contains ( float a, float b ) const { return x<=a && a<=xp() && y<=b && b<=yp(); }
	bool contains ( float a, float b, float ds ) const { return x+ds<=a && a<=xp()-ds && y+ds<=b && b<=yp()-ds; }
	bool contains ( const GsRect& r ) const { return contains(r.x,r.y,r.w,r.h); }
	bool contains ( float a, float b, float dw, float dh ) const { return x<=a && a+dw<=xp() && y<=b && b+dh<=yp(); }

	/*! Returns vertices with coordinates (x,y), (xp,y), (x,yp), (xp,yp).
		In window coordinates: top-left, top-right, bottom-left, bottom-right order;
		in scene coordinates: bottom-left, bottom-right, top-left, top-right. */
	void get_vertices ( GsPnt2& a, GsPnt2& b, GsPnt2& c, GsPnt2& d ) const;

	/*! Output as 4 space-separated values*/
	friend GsOutput& operator<< ( GsOutput& o, const GsRect& r ) { return o<<r.x<<gspc<<r.y<<gspc<<r.w<<gspc<<r.h; }
};

//================================ End of File =================================================

# endif // GS_RECT_H
