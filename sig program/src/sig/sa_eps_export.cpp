/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/sa_eps_export.h>
# include <sig/sa_bbox.h>
# include <sig/sn_shape.h>
# include <sig/sn_lines.h>
# include <sig/sn_lines2.h>
# include <sig/sn_points.h>
# include <sig/sn_polygons.h>

//# define GS_USE_TRACE1 // constructor / destructor
//# define GS_USE_TRACE2 // render
# include <sig/gs_trace.h>

//============================= export functions ====================================

// detect if lines are in 3d or 2d!!?
// make use of current matrix (and camera) transformation

static void export_lines ( SnShape* shape, GsOutput& o, const SaEpsExport* epsexp )
 {
   SnLines& l = *((SnLines*)shape);
   GsArray<GsPnt>&   V = l.V;
   GsArray<GsColor>& C = l.Vc;  // TodoNote: Revision needed for new nodes - was using C intead of Vc
   GsArray<gsuint>&  I = l.I;
   float c[4];

   if ( V.size()<2 ) return;

   shape->material().diffuse.get ( c );
   o << c[0] << gspc << c[1] << gspc << c[2] << " setrgbcolor\n";

   float res = l.line_width(); // 0.5 1.0 1.5
   res *= 0.02f;	// makes resolution 1 be .02 cms
   res *= 28.346f;  // put in unit pts
   res /= epsexp->scale_factor();
   o << res << " setlinewidth\n";

   int pat = (int)l.patfactor;
   float patunit = 0.02f;  // default used lenght for pat==1
   patunit += 0.01f*pat;	// customize lenght with pat
   o << "[ ";
   if ( pat ) o << patunit << gspc;
   o << "] 0 setdash\n";

   int v=0;			   // current V index
   int i;				 // current I index
   int imax = I.size()-1; // max value for i
   int i1=-1, i2 = -1;		 // pair I[i],I[i+1]

   if ( I.size()>1 ) { i=0; i1=I[i]; i2=I[i+1]; }

   while ( v<V.size() )
	{
	  if ( v==i1 )
	   { if ( i2<0 ) // new color
		  {  C[-i2-1].get ( c );
			 o << c[0] << gspc << c[1] << gspc << c[2] << " setrgbcolor\n";
		  }
		 else if ( v<V.size() ) // new polyline
		  { o << "newpath\n";
			o << V[v].x << gspc << V[v].y << " moveto\n";
			v++;
			while ( v<V.size() && v<=i2 )
			 { o << V[v].x << gspc << V[v].y << " lineto\n";
			   v++;
			 }
			//o << "closepath\n";
			o << "stroke\n\n";
		  }

		 i+=2; // update next I information
		 if ( i<imax ) { i1=I[i]; i2=I[i+1]; } else i1=-1;
	   }
	  else
	   { o << "newpath\n";
		 bool move=true;
		 while ( v<V.size() && v!=i1 )
		  { o << V[v].x << gspc << V[v].y;
			v++;
			if ( move )
			 { o<<" moveto\n"; move=false; }
			else
			 { o<<" lineto\n"; move=true; }
		  }
		 //o << "closepath\n";
		 o << "stroke\n\n";
	   }
	}
 }

static void export_points ( SnShape* /*shape*/, GsOutput& /*o*/, const SaEpsExport* /*epsexp */)
 {
/*
   GsPoints& p = ((SnPoints*)shape)->get();

   if ( p.P.size()==0 ) return;

   glDisable ( GL_LIGHTING );
   glColor ( shape->material().diffuse );

   if ( shape->render_mode()==gsRenderModeSmooth )
	{ // render shperes, with resolution as radius?
	}

   glPointSize ( shape->resolution() ); // default is 1.0

   int i;
   glBegin ( GL_POINTS );
   for ( i=0; i<p.P.size(); i++ )
	glVertex ( p.P[i] );
   glEnd ();
*/
 }

static void export_polygon ( GsPolygon& p, gsRenderMode /*rm*/, float res,
							 GsOutput& o, const SaEpsExport* epsexp )
 {
   if ( p.size()==0 ) return;

   int i;

   if ( p.open() )
	{ res *= 0.02f;	// makes resolution 1 be .02 cms
	  res *= 28.346f;  // put in unit pts
	  res /= epsexp->scale_factor();
	  o << res << " setlinewidth\n";
	}

   o << "newpath\n";
   o << p[0] << " moveto\n";
   for ( i=1; i<p.size(); i++ ) o << p[i] << " lineto\n";
   if ( !p.open() ) o << p[0] << " lineto\n";
   o << "closepath\n";

   if ( p.open() )
	{ o << "stroke\n\n";
	}
   else
	{ o << "fill\n\n";
	}
 }

/*
	{ if ( rm==gsRenderModeSmooth || rm==gsRenderModeFlat || rm==gsRenderModeDefault )
	   { GsArray<GsPnt2> tris;
		 p.ear_triangulation ( tris );
		 glBegin ( GL_TRIANGLES );
		 for ( i=0; i<tris.size(); i++ ) glVertex ( tris[i] );
		 glEnd ();
	   }
	}
   if ( rm==gsRenderModePoints || rm==gsRenderModeFlat )
	{ glPointSize ( res*4 );
	  glBegin ( GL_POINTS );
	  for ( i=0; i<p.size(); i++ ) glVertex ( p[i] );
	  glEnd ();
	}
*/

static void export_polygons ( SnShape* shape, GsOutput& o, const SaEpsExport* epsexp )
 {
   SnPolygons& snp = *((SnPolygons*)shape);
   GsPolygons& p = *snp.polygons();

   if ( p.size()==0 ) return;

   float c[4];
   shape->material().diffuse.get ( c );
   o << c[0] << gspc << c[1] << gspc << c[2] << " setrgbcolor\n";

   float resolution = snp.lines()->line_width();

   int i;
   for ( i=0; i<p.size(); i++ ) ::export_polygon ( p[i], shape->render_mode(), resolution, o, epsexp );
 }

//=============================== SaEpsExport ====================================

GsArray<SaEpsExport::RegData> SaEpsExport::_efuncs;

void register_export_function ( const char* class_name, SaEpsExport::export_function efunc ) // friend function
 {
   GsArray<SaEpsExport::RegData>& ef = SaEpsExport::_efuncs;

   int i;
   for ( i=0; i<ef.size(); i++ ) 
	{ if ( gs_compare(ef[i].class_name,class_name)==0 ) break;
	}
   if ( i==ef.size() ) ef.push(); // if not found, push a new position

   ef[i].class_name = class_name;
   ef[i].efunc = efunc;
 }

SaEpsExport::SaEpsExport ( GsOutput& o ) 
			   : _output ( o )
 { 
   GS_TRACE1 ( "Constructor" );

   _page_width = 21.59f;  // == 612 pts
   _page_height = 27.94f; // == 792 pts
   _page_margin = 4.0f;
   _bbox_margin = 0.1f;

   if ( _efuncs.size()==0 ) // no functions registered
	{ //register_export_function ( "SnModel",	export_model );
	  register_export_function ( "SnLines",	export_lines );
	  register_export_function ( "SnPoints",   export_points );
	  //register_export_function ( "SnBox",	  export_box );
	  //register_export_function ( "SnSphere",   export_sphere );
	  register_export_function ( "SnPolygons", export_polygons );
	}
 }

SaEpsExport::~SaEpsExport ()
 {
   GS_TRACE1 ( "Destructor" );
 }

bool SaEpsExport::apply ( SnNode* n )
 {
   // get bounding box
   SaBBox ab;
   ab.apply(n);
   GsBox bbox = ab.get();
   GsVec bboxsize = bbox.size();
   GsVec bboxcenter = bbox.center();

   // PostScript uses pts as default unit: 1inch==72pts, 1cm==28.346pts
   // so we put this transformation constant here:
   const float topts = 28.346f;

   // The required scaling so that the bounding box is inside the page
   // and respecting the desired margin:
   _scale_factor = _page_width / (bboxsize.x+2*_page_margin);
   _scale_factor *= topts;

   // The required translation to make the center of the bounding box
   // be in the middle of the page:
   _translation.set ( _page_width*topts/2  - bboxcenter.x*_scale_factor, 
					  _page_height*topts/2 - bboxcenter.y*_scale_factor );

   // Output eps header
   _output << "%!PS-Adobe-3.0 EPSF-3.0\n";

   // Output the ebounding box:
   // The four arguments of the bounding box comment correspond to the lowerleft
   // and upper-right corners of the bounding box, expressed in the default 
   // PostScript coordinate system (pts) (28.346 converts cms to pts).
   float a = ( bbox.a.x - _bbox_margin ) * _scale_factor + _translation.x;
   float b = ( bbox.a.y + _bbox_margin ) * _scale_factor + _translation.y;
   float c = ( bbox.b.x - _bbox_margin ) * _scale_factor + _translation.x;
   float d = ( bbox.b.y + _bbox_margin ) * _scale_factor + _translation.y;
   _output << "%%BoundingBox: " << int(a) << gspc << int(b) << gspc
								<< int(c) << gspc << int(d) << gsnl;

   // Output other information:
//ww: have a mark to quickly find update places like this
   _output << "%%Creator: SIG EPS Exporter\n";
   _output << "%!GsBoundingBox: " << bbox.a.x << gspc << bbox.a.y << gspc
								  << bbox.b.x << gspc << bbox.b.y << gsnl;

   _output << "%!\n\n";

   // Save the graphics state:
   _output << "gsave\n";

   // Output transformations so to work in cms and to fit the bbox in the page:
   _output << _translation << " translate\n";
   _output << _scale_factor << gspc << _scale_factor << " scale\n";

   // Other settings:
//   _output << "1 setlinewidth\n";
   _output << gsnl;

   // Call the scene graph:
   bool result = SaAction::apply ( n );

   // Finalize:
   _output << "\n";
   _output << "grestore\n";
//   _output << "showpage\n";

   return result;
 }

//==================================== virtuals ====================================

void SaEpsExport::mult_matrix ( const GsMat& /*mat*/ )
 {
   //glMultMatrix ( mat );
 }

void SaEpsExport::push_matrix ()
 {
   //glPushMatrix ();
 }

void SaEpsExport::pop_matrix ()
 {
   //glPopMatrix ();
 }

bool SaEpsExport::shape_apply ( SnShape* s )
 {
   // 1. Search for export function
   int i;
   const char* class_name = s->instance_name ();
   GsArray<SaEpsExport::RegData>& ef = SaEpsExport::_efuncs;
   for ( i=0; i<ef.size(); i++ )
	if ( gs_compare(ef[i].class_name,class_name)==0 ) break;
   if ( i==ef.size() ) return true; // not found: nothing is done

   // 2. Export only if needed
   if ( !s->visible() ) return true;

   // 3. Export
   ef[i].efunc ( s, _output, this );
   return true;
 }

//======================================= PS GUIDE ====================================

/*
http://adela.karlin.mff.cuni.cz/netkarl/prirucky/Flat/paths.html

72 72 scale			 points to inches, 1 pt = 1/72 inches
2.8346 2.8346 scale	 points to mms
1.0 setlinewidth	0 is valid
2 setlinecap		0, 1, 2: butt ends, round ends, square ends
2 setlinejoin		0, 1, 2: miter corners, round corners, bevel corners
0.5 setgray		0 to 1

gsave
grestore


EXAMPLE:
%!

0.025 setlinewidth
72 72 scale
0.8 setgray
newpath
1 1 moveto
7 1 lineto
1 5 lineto
closepath
fill

0 setgray
newpath
1 1 moveto
7 1 lineto
1 5 lineto
closepath
stroke

showpage


*/

//======================================= EOF ====================================

