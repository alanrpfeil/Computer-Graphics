/*=======================================================================
	Copyright (c) 2018-2019 Marcelo Kallmann.
	This software is distributed under the Apache License, Version 2.0.
	All copies must contain the full copyright notice licence.txt located
	at the base folder of the distribution. 
  =======================================================================*/

# include <stdio.h>
# include <math.h>

# include <sig/gs_geo2.h>
# include <sig/gs_output.h>

//================================= Macros =========================================

# define gPARAM(a,b,x)		((x)-(a))/((b)-(a)) // return t : x = a(1-t) + bt
# define gDIST(a,b)			(a>b? (a-b):(b-a))
# define gABS(x)			(x>0? (x):-(x))
# define gMAX(a,b)			(a>b? (a):(b))
# define gMIN3(a,b,c)		((a)<(b)? ((a)<(c)?(a):(c)):((b)<(c)?(b):(c)))
# define gMAX3(a,b,c)		((a)>(b)? ((a)>(c)?(a):(c)):((b)>(c)?(b):(c)))
# define gSWAP(a,b)			{ tmp=a; a=b; b=tmp; }
# define gOUTP(a,b)			printf("(%+6.4f,%+6.4f) ", a, b )
# define gOUTL				printf("\n");
# define gPI				3.1415926535897932
# define gEPS				1.0E-14
# define gNEXTZERO(a)		a>-gEPS && a<gEPS

//================================= funcs =====================================

inline bool gssegsintersect ( double p1x, double p1y, double p2x, double p2y,
							  double p3x, double p3y, double p4x, double p4y, double &t )
{
	double d, s;
	d = (p4y-p3y)*(p1x-p2x)-(p1y-p2y)*(p4x-p3x);
	if ( gNEXTZERO(d) ) return false; // they are parallel
	t = ((p4y-p3y)*(p4x-p2x)-(p4x-p3x)*(p4y-p2y)) / d;
	t = 1.0-t; // convert t to be from p1 to p2
	if ( t<0.0 || t>1.0 ) return false; // outside [p1,p2]
	s = ((p4y-p2y)*(p1x-p2x)-(p1y-p2y)*(p4x-p2x)) / d;
	if ( s<0.0 || s>1.0 ) return false; // outside [p3,p4]
	return true;
}

bool gs_segments_intersect ( double p1x, double p1y, double p2x, double p2y,
							 double p3x, double p3y, double p4x, double p4y )
{
	double t;
	return gssegsintersect ( p1x, p1y, p2x, p2y, p3x, p3y, p4x, p4y, t );
}

bool gs_segments_intersect ( double p1x, double p1y, double p2x, double p2y,
							 double p3x, double p3y, double p4x, double p4y,
							 double& qx, double &qy )
{
	double t;
	if ( !gssegsintersect ( p1x, p1y, p2x, p2y, p3x, p3y, p4x, p4y, t ) ) return false;
	qx = (1-t)*p1x+t*p2x;
	qy = (1-t)*p1y+t*p2y;
	return true;
}

bool gs_segments_intersect ( double p1x, double p1y, double p2x, double p2y,
							 double p3x, double p3y, double p4x, double p4y,
							 double& qx, double &qy, double& t )
{
	if ( !gssegsintersect ( p1x, p1y, p2x, p2y, p3x, p3y, p4x, p4y, t ) ) return false;
	qx = (1-t)*p1x+t*p2x;
	qy = (1-t)*p1y+t*p2y;
	return true;
}

bool gs_segments_intersect ( double p1x, double p1y, double p2x, double p2y,
							 double p3x, double p3y, double p4x, double p4y, double& t )
{
	return gssegsintersect ( p1x, p1y, p2x, p2y, p3x, p3y, p4x, p4y, t );
}

bool gs_lines_intersect ( double p1x, double p1y, double p2x, double p2y,
						  double p3x, double p3y, double p4x, double p4y )
{
	double d = (p4y-p3y)*(p1x-p2x)-(p1y-p2y)*(p4x-p3x);
	if ( gNEXTZERO(d) ) return false; // they are parallel
	return true;
}

bool inline gslinesintersect ( double p1x, double p1y, double p2x, double p2y,
								double p3x, double p3y, double p4x, double p4y, double& t )
{
	double d = (p4y-p3y)*(p1x-p2x)-(p1y-p2y)*(p4x-p3x);
	if ( gNEXTZERO(d) ) return false; // they are parallel
	t = ((p4y-p3y)*(p4x-p2x)-(p4x-p3x)*(p4y-p2y)) / d; // t here is from p2 to p1
	t = 1.0-t; // convert t to be from p1 to p2
	return true;
}

bool inline gslinesintersect ( double p1x, double p1y, double p2x, double p2y,
								double p3x, double p3y, double p4x, double p4y,
								double& qx, double& qy, double& t )
{
	double d = (p4y-p3y)*(p1x-p2x)-(p1y-p2y)*(p4x-p3x);
	if ( gNEXTZERO(d) ) return false; // they are parallel
	t = ((p4y-p3y)*(p4x-p2x)-(p4x-p3x)*(p4y-p2y)) / d;
	qx = t*p1x+(1.0-t)*p2x; // Note that t here is from p2 to p1
	qy = t*p1y+(1.0-t)*p2y;
	t = 1.0-t; // convert t to be from p1 to p2
	return true;
}

bool gs_lines_intersect ( double p1x, double p1y, double p2x, double p2y,
						  double p3x, double p3y, double p4x, double p4y,
						  double& qx, double& qy )
{
	double t;
	return gslinesintersect(p1x,p1y,p2x,p2y,p3x,p3y,p4x,p4y,qx,qy,t);
}

bool gs_lines_intersect ( double p1x, double p1y, double p2x, double p2y,
						  double p3x, double p3y, double p4x, double p4y,
						  double& qx, double& qy, double& t )
{
	return gslinesintersect(p1x,p1y,p2x,p2y,p3x,p3y,p4x,p4y,qx,qy,t);
}

bool gs_segment_line_intersect ( double p1x, double p1y, double p2x, double p2y,
								 double p3x, double p3y, double p4x, double p4y,
								 double& qx, double& qy )
{
	double t;
	if ( !gslinesintersect(p1x,p1y,p2x,p2y,p3x,p3y,p4x,p4y,qx,qy,t) ) return false;
	if ( t<0 || t>1 ) return false;
	return true;
}

void gs_line_projection ( double p1x, double p1y, double p2x, double p2y, double px, double py, double& qx, double& qy )
{
	double t;
	// ortho vect = ( -(p2y-p1y), p2x-p1x ) = (p2-p1).ortho(), ortho==(-y,x)
	gslinesintersect ( p1x, p1y, p2x, p2y, px, py, px-(p2y-p1y), py+(p2x-p1x), qx, qy, t );
}

void gs_line_projection ( double p1x, double p1y, double p2x, double p2y, double px, double py, double& qx, double& qy, double& t )
{
	// ortho vect = ( -(p2y-p1y), p2x-p1x ) = (p2-p1).ortho(), ortho==(-y,x)
	gslinesintersect ( p1x, p1y, p2x, p2y, px, py, px-(p2y-p1y), py+(p2x-p1x), qx, qy, t );
}

void gs_line_projection ( double p1x, double p1y, double p2x, double p2y, double px, double py, double& t )
{
	// ortho vect = ( -(p2y-p1y), p2x-p1x ) = (p2-p1).ortho(), ortho==(-y,x)
	gslinesintersect ( p1x, p1y, p2x, p2y, px, py, px-(p2y-p1y), py+(p2x-p1x), t );
}

int gs_segment_projection ( double p1x, double p1y, double p2x, double p2y, double px, double py, double& qx, double& qy, double epsilon )
{
	double t;
	// ortho vect = ( -(p2y-p1y), p2x-p1x ) = (p2-p1).ortho(), ortho==(-y,x)
	gslinesintersect ( p1x, p1y, p2x, p2y, px, py, px-(p2y-p1y), py+(p2x-p1x), qx, qy, t );

	if ( t<0.0 ) // check dist(q,p1)
	{	px = qx-p1x;
		py = qy-p1y;
		return (px*px + py*py)>epsilon*epsilon? 0:1;
	}
	else if ( t>1.0 ) // check dist(q,p2)
	{	px = qx-p2x;
		py = qy-p2y;
		return (px*px + py*py)>epsilon*epsilon? 0:2;
	}
	else return 3; // inside segment, regardless of the epsilon
}

double gs_project_to_segment ( double p1x, double p1y, double p2x, double p2y, double px, double py, double& qx, double& qy )
{
	double t;
	// ortho vect = ( -(p2y-p1y), p2x-p1x ) = (p2-p1).ortho(), ortho==(-y,x)
	gslinesintersect ( p1x, p1y, p2x, p2y, px, py, px-(p2y-p1y), py+(p2x-p1x), qx, qy, t );
	if ( t<0.0 )
	{	qx=p1x; qy=p1y; }
	else if ( t>1.0 )
	{	qx=p2x; qy=p2y;
	}
	return t;
}

inline double gsdist2 ( double p1x, double p1y, double p2x, double p2y )
{
	double dx = p2x-p1x;
	double dy = p2y-p1y;
	return dx*dx + dy*dy;
}

double gs_dist ( double p1x, double p1y, double p2x, double p2y )
{
	return sqrt(gsdist2(p1x,p1y,p2x,p2y));
}

double gs_dist2 ( double p1x, double p1y, double p2x, double p2y )
{
	return gsdist2(p1x,p1y,p2x,p2y);
}

double gs_param ( double p1x, double p1y, double p2x, double p2y, double px, double py )
{
	double dx = p2x-p1x;
	double dy = p2y-p1y;
	return gABS(dx)>gABS(dy)? (px-p1x)/dx : (py-p1y)/dy;
}

double gs_angle ( double v1x, double v1y, double v2x, double v2y )
{
	double n = sqrt(v1x*v1x+v1y*v1y) * sqrt(v2x*v2x+v2y*v2y);
	// n will be 0 if a given vector is null, so check that here:
	if ( n==0 ) return 0;
	// get the cosine of the angle:
	n = (v1x*v2x + v1y*v2y) / n;
	// n has to be in [-1,1] and numerical aproximations may result in values
	// slightly outside the interval; so the test below is really needed:
	if ( n<-1.0 ) n=-1.0; else if ( n>1.0 ) n=1.0;
	return acos ( n );
}

double gs_len ( double& vx, double& vy, double len )
{
	double l = sqrt ( vx*vx+vy*vy);
	if ( l>0 ) { len = len/l; vx*=len; vy*=len; }
	return l;
}

double gs_anglen ( double v1x, double v1y, double v2x, double v2y )
{
	return acos ( v1x*v2x+v1y*v2y );
}

inline double gspsegdist2 ( double px, double py, double p1x, double p1y, double p2x, double p2y, double& t )
{
	// ortho vect = ( -(p2y-p1y), p2x-p1x ) = (p2-p1).ortho(), ortho==(-y,x)
	gslinesintersect ( p1x, p1y, p2x, p2y, px, py, px-(p2y-p1y), py+(p2x-p1x), t );

	if ( t<=0.0 ) // get dist(p,p1)
	{	px = px-p1x;
		py = py-p1y;
	}
	else if ( t>=1.0 ) // get dist(p,p2)
	{	px = px-p2x;
		py = py-p2y;
	}
	else // get dist(p,q)
	{	px = px - ( (1.0-t)*p1x + t*p2x );
		py = py - ( (1.0-t)*p1y + t*p2y );
	}

	return (px*px + py*py);
}

inline double gspsegdist2 ( double px, double py, double p1x, double p1y, double p2x, double p2y, double& t, double& qx, double& qy )
{
	// ortho vect = ( -(p2y-p1y), p2x-p1x ) = (p2-p1).ortho(), ortho==(-y,x)
	gslinesintersect ( p1x, p1y, p2x, p2y, px, py, px-(p2y-p1y), py+(p2x-p1x), qx, qy, t );

	if ( t<0.0 ) // get dist(p,p1)
	{	px = px-p1x;
		py = py-p1y;
	}
	else if ( t>1.0 ) // get dist(p,p2)
	{	px = px-p2x;
		py = py-p2y;
	}
	else // get dist(p,q)
	{	px = px-qx;
		py = py-qy;
	}

	return (px*px + py*py);
}

double gs_point_segment_dist ( double px, double py, double p1x, double p1y, double p2x, double p2y )
{
	double t;
	return sqrt ( gspsegdist2( px, py, p1x, p1y, p2x, p2y, t ) );
}

double gs_point_segment_dist2 ( double px, double py, double p1x, double p1y, double p2x, double p2y )
{
	double t;
	return gspsegdist2 ( px, py, p1x, p1y, p2x, p2y, t );
}

double gs_point_segment_dist2 ( double px, double py, double p1x, double p1y, double p2x, double p2y, double& t )
{
	return gspsegdist2 ( px, py, p1x, p1y, p2x, p2y, t );
}

double gs_point_segment_dist2 ( double px, double py, double p1x, double p1y, double p2x, double p2y, double& t, double& qx, double& qy )
{
	return gspsegdist2 ( px, py, p1x, p1y, p2x, p2y, t, qx, qy );
}

inline double gssegsegd2 ( double p1x, double p1y, double p2x, double p2y,
						   double p3x, double p3y, double p4x, double p4y )
{
	double t, d, dmin;
	dmin = gspsegdist2( p1x, p1y, p3x, p3y, p4x, p4y, t );
	d = gspsegdist2( p2x, p2y, p3x, p3y, p4x, p4y, t );
	if ( d<dmin ) dmin=d;
	d = gspsegdist2( p3x, p3y, p1x, p1y, p2x, p2y, t );
	if ( d<dmin ) dmin=d;
	d = gspsegdist2( p4x, p4y, p1x, p1y, p2x, p2y, t );
	if ( d<dmin ) dmin=d;
	return dmin;
}

double gs_segment_segment_dist ( double p1x, double p1y, double p2x, double p2y,
								 double p3x, double p3y, double p4x, double p4y )
{
	return sqrt ( gssegsegd2(p1x,p1y,p2x,p2y,p3x,p3y,p4x,p4y) );
}

double gs_segment_segment_dist2 ( double p1x, double p1y, double p2x, double p2y,
								 double p3x, double p3y, double p4x, double p4y )
{
	return gssegsegd2(p1x,p1y,p2x,p2y,p3x,p3y,p4x,p4y);
}

double gs_point_line_dist ( double px, double py, double p1x, double p1y, double p2x, double p2y )
{
	double qx, qy;
	gs_line_projection (p1x,p1y,p2x,p2y,px,py,qx,qy);
	return sqrt( gsdist2(px,py,qx,qy) );
}

bool gs_next ( double p1x, double p1y, double p2x, double p2y, double epsilon )
{ 
	return gsdist2(p1x,p1y,p2x,p2y)<=epsilon*epsilon; // only inline calls
}

bool gs_next2 ( double p1x, double p1y, double p2x, double p2y, double epsilon2 )
{ 
	return gsdist2(p1x,p1y,p2x,p2y)<=epsilon2; // only inline calls
}

double gs_ccw ( double p1x, double p1y, double p2x, double p2y, double p3x, double p3y )
{
   /* The CCW test determinant is:
	  (ax*by)-(bx*ay)+(bx*cy)-(cx*by)+(cx*ay)-(ax*cy).
	  Simplifying we have:
	  axby -aybx +cybx -cxby -axcy + aycx ( -cycx +cxcy )
	  axby -aybx +cybx -cxby -axcy + cxcy + aycx - cycx
	  axby -aybx +cybx -cxby -(axcy -cxcy)+ aycx - cycx
	  (axby -cxby) -(axcy -cxcy) +(-aybx --cybx) +(aycx - cycx)
	  (ax-cx)*by - (ax-cx)*cy - (ay-cy)bx + (ay-cy)cx
	  (ax-cx)*(by-cy) - (ay-cy)*(bx-cx) => this equation is in the macro below */
	return GS_CCW(p1x,p1y,p2x,p2y,p3x,p3y);
}

bool gs_in_segment ( double p1x, double p1y, double p2x, double p2y, double px, double py, double epsilon )
{
	double t;
	return gspsegdist2( px, py, p1x, p1y, p2x, p2y, t ) > epsilon*epsilon? false:true;
}

bool gs_in_segment ( double p1x, double p1y, double p2x, double p2y, double px, double py,
					 double epsilon, double& dist2, double& t )
{
	double qx, qy;
	dist2 = gspsegdist2( px, py, p1x, p1y, p2x, p2y, t, qx, qy );
	return dist2>epsilon*epsilon? false:true;
}

bool gs_in_triangle ( double p1x, double p1y, double p2x, double p2y, double p3x, double p3y, double px, double py )
{
	return GS_CCW(px,py,p1x,p1y,p2x,p2y)>=0 && 
		   GS_CCW(px,py,p2x,p2y,p3x,p3y)>=0 && 
		   GS_CCW(px,py,p3x,p3y,p1x,p1y)>=0;
}

bool gs_in_triangle_interior ( double p1x, double p1y, double p2x, double p2y, double p3x, double p3y, double px, double py )
{
	return GS_CCW(px,py,p1x,p1y,p2x,p2y)>0 && 
		   GS_CCW(px,py,p2x,p2y,p3x,p3y)>0 && 
		   GS_CCW(px,py,p3x,p3y,p1x,p1y)>0;
}

double gs_tangent ( double px, double py, double cx, double cy, double r, 
					double& t1x, double& t1y, double& t2x, double& t2y ) // (p,c,t1):CCW, (p,c,t2):CW
{
	double d2 = gsdist2(px,py,cx,cy);
	double d = d2-r*r;
	if ( d<=0 ) return -1;
	double s = sqrt ( d );
	d = sqrt ( d2 );
   
	if ( d==0 ) return -1;
	cx = (cx-px)/d; // c-p unit vector 
	cy = (cy-py)/d;
   
	double sa = r/d; // get sinus and cosinus
	double ca = s/d;
	t1x = (cx*ca-cy*sa) * s + px; // rotate and scale
	t1y = (cx*sa+cy*ca) * s + py;
	sa=-sa;
	t2x = (cx*ca-cy*sa) * s + px; // now rotate to the other side
	t2y = (cx*sa+cy*ca) * s + py;

	/* code fragment to test this function:
	double tx1, ty1, tx2, ty2, px=1, py=1, cx=-5, cy=5, r=3;
	gs_tangent(px,py,cx,cy,r,tx1,ty1,tx2,ty2);
	gsout<<tx1<<","<<ty1<<gsnl;
	gsout<<tx2<<","<<ty2<<gsnl; */

	return s;
}

double gs_external_tangents ( double c1x, double c1y, double r1, double c2x, double c2y, double r2, 
							  double& t1x, double& t1y, double& t2x, double& t2y,
							  double& t3x, double& t3y, double& t4x, double& t4y )
{
	double s, r, vx, vy;
	if ( r1==r2 )
	{	s = sqrt(gsdist2(c1x,c1y,c2x,c2y));
		if ( s==0 ) return -1;
		vy =  ((c2x-c1x)/s)*r1; // ortho vector
		vx = -((c2y-c1y)/s)*r1; 
		t1x=c1x-vx; t1y=c1y-vy; t2x=c2x-vx; t2y=c2y-vy;
		t3x=c1x+vx; t3y=c1y+vy; t4x=c2x+vx; t4y=c2y+vy;
	}
	else if ( r1>r2 )
	{	r = r1-r2;
		s = gs_tangent ( c2x, c2y, c1x, c1y, r, t1x, t1y, t3x, t3y ); // (p,c,t1):CCW, (p,c,t2):CW
		if ( s<0 ) return -1;
		vx = ((t1x-c1x)/r)*r2;
		vy = ((t1y-c1y)/r)*r2;
		t1x+=vx; t1y+=vy; t2x=c2x+vx; t2y=c2y+vy; // (c1,t1,c2) is CCW
		vx = ((t3x-c1x)/r)*r2;
		vy = ((t3y-c1y)/r)*r2;
		t3x+=vx; t3y+=vy; t4x=c2x+vx; t4y=c2y+vy; // (c1,t3,c2) is CW
	}
	else // r2>r1
	{	r = r2-r1;
		s = gs_tangent ( c1x, c1y, c2x, c2y, r, t4x, t4y, t2x, t2y ); // (p,c,t1):CCW, (p,c,t2):CW
		if ( s<0 ) return -1;
		vx = ((t4x-c2x)/r)*r1;
		vy = ((t4y-c2y)/r)*r1;
		t4x+=vx; t4y+=vy; t3x=c1x+vx; t3y=c1y+vy;
		vx = ((t2x-c2x)/r)*r1;
		vy = ((t2y-c2y)/r)*r1;
		t2x+=vx; t2y+=vy; t1x=c1x+vx; t1y=c1y+vy;
	}
	return s;
}

double gs_internal_tangents ( double c1x, double c1y, double r1, double c2x, double c2y, double r2, 
							  double& t1x, double& t1y, double& t2x, double& t2y,
							  double& t3x, double& t3y, double& t4x, double& t4y )
{
	double s, r, vx, vy;
	r = r1+r2;
	s = gs_tangent ( c2x, c2y, c1x, c1y, r, t1x, t1y, t3x, t3y ); // (p,c,t1):CCW, (p,c,t2):CW
	if ( s<0 ) return -1;
	vx = ((c1x-t1x)/r)*r2;
	vy = ((c1y-t1y)/r)*r2;
	t1x+=vx; t1y+=vy; t2x=c2x+vx; t2y=c2y+vy;
	vx = ((c1x-t3x)/r)*r2;
	vy = ((c1y-t3y)/r)*r2;
	t3x+=vx; t3y+=vy; t4x=c2x+vx; t4y=c2y+vy;
	return s;
}

//=============================== End of File ======================================
