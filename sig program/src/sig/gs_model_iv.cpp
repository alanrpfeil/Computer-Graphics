/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_model.h>

//# define GS_USE_TRACE1	// keyword tracking
//# define GS_USE_TRACE2	// data tracking
//# define GS_USE_TRACE3	// more data tracking
# include <sig/gs_trace.h>

static void wcolor ( GsColor c, GsOutput& o )
 {
   float f[4];
   c.get(f);
   o << f[0] << gspc << f[1] << gspc << f[2];
 }

bool GsModel::save_iv ( const char* file )
 {
   int i;
   GsOutput o ( fopen(file,"wt") );
   if ( !o.valid() ) return false;

   // ===== Header =================================
   o << "#VRML V1.0 ascii\n\n";
   o << "PanelSeparator {\n\n";
   o << "  Info { string \"Produced by GsModel iv exporter 0.5, M. Kallmann 2006\" }\n\n";

   // ===== Hints ==================================
   o << "  ShapeHints {\n";
   if ( culling ) o << "   shapeType SOLID\n";
   o << "   vertexOrdering COUNTERCLOCKWISE\n";
   o << "   faceType CONVEX\n";
   o << "  }\n\n";
   
   //===== Materials ===============================
   if ( M.size()==1 )
	{ o << "  Material {\n";
	  o << "  diffuseColor "; wcolor(M[0].diffuse,o); o<<gsnl;
	  o << "  }\n\n";
	}
   else if ( M.size()>1 )
	{ o << "  Material {\n";
	  o << "  diffuseColor [\n";
	  for ( i=0; i<M.size(); i++ )
	   { o<<gstab;
		 wcolor(M[i].diffuse,o);
		 if ( i<M.size()-1 ) o<<",\n"; else o<<"]\n";
	   }
	  o << "  }\n";
	  o << "  MaterialBinding { value PER_FACE_INDEXED }\n\n";
	}
   
   //===== Coordinates ===============================
   o << "  Coordinate3 { point [\n";
   for ( i=0; i<V.size(); i++ )
	{ o << gstab << V[i];
	  if ( i<V.size()-1 ) o<<",\n"; else o<<"]\n";
	}
   o << "  }\n\n";

   //===== Faces ===============================
   o << "  IndexedFaceSet { coordIndex [\n";

   for ( i=0; i<F.size(); i++ )
	{ o << gstab << F[i].a << ", " << F[i].b << ", " << F[i].c << ", -1";
	  if ( i<F.size()-1 ) o<<",\n"; else o<<"]\n";
	}

   GsArray<int> Fm;
   get_materials_per_face ( Fm );
   if ( M.size()>1 )
	{ o << "	materialIndex [\n";
	  for ( i=0; i<Fm.size(); i++ )
	   { o << gstab << Fm[i];
		 if ( i<Fm.size()-1 ) o<<",\n"; else o<<"]\n";
	   }
	}

   o << "  }\n\n";

   //===== End ===============================

   o << "}\n\n";

   GS_TRACE1("Ok!");
   return true;
 }

static void loadmtl ( GsInput& in, GsMaterial& m )
 {
   in.get(); // '{'
   float r, g, b;

   while ( true )
	{ in.get();
	  if ( in.ltoken()[0]=='}' ) break;
	  if ( in.ltoken()[0]=='a' ) { in >> r >> g >> b; m.ambient.set ( r, g, b ); }
	  else if ( in.ltoken()[0]=='d' ) { in >> r >> g >> b; m.diffuse.set ( r, g, b ); }
	  else if ( in.ltoken()[1]=='p' ) { in >> r >> g >> b; m.specular.set ( r, g, b ); }
	  else if ( in.ltoken()[1]=='h' ) { in >> r; m.shininess = gsbyte(r*128); }
	}
 }

class IvDef
{  public :
	GsString key;
	int mi; // only definition of materials are supported by now
};

# define SKIP_SECTION   while (in.ltoken()[0]!='}') in.get();

bool GsModel::load_iv ( const char* fname )
 {
   GsInput in;
   if ( !in.open(fname) ) return false;

   // ensure proper input type
   in.commentchar ( '#' );
   in.lowercase ( false );

   // clear arrays and set culling to true
   init ();

   GsArray<GsMat> stack; // transformation stack
   GsArrayPt<IvDef> defs; // definitions
   GsArray<int> Fm; // materials per face

   int mi=0; // current material
   int vi=0; // latest initial vertex index loaded
   int ni=0; // latest initial vertex index loaded

   GsString s;
   while ( true )
	{ 
	  in.get();
	  if ( in.ltype()==GsInput::End ) break;
  
	  if ( in.ltype()==GsInput::Delimiter && in.ltoken()[0]=='}' )
	   { stack.pop();
		 continue;
	   }

	  s = in.ltoken();
	  GS_TRACE1 ( '[' << s << "] keyword found..." );

	  if ( s=="ShapeHints" ) // skip this one
	   { SKIP_SECTION;
	   }
	  else if ( s=="PanelSeparator" )
	   { in.get(); // '{'
		 stack.push();
		 if ( stack.size()==1 )
		  stack.top().identity();
		 else
		  stack.top() = stack.top(1);
	   }
	  else if ( s=="Transform" )
	   { in.get(); // '{'
		 GsVec trans, rotaxis, scalefac, scaleori;
		 float rotang=0, scaleang=0;
		 in.get();
		 while ( in.ltoken()[0]!='}' )
		  { if ( in.ltoken()=="translation" ) { in>>trans; }
			else if ( in.ltoken()=="rotation" ) { in>>rotaxis; in>>rotang; }
			else if ( in.ltoken()=="scaleFactor" ) { in>>scalefac; }
			else if ( in.ltoken()=="scaleOrientation" ) { in>>scaleori; in>>scaleang; }
			in.get();
		  }
		 GsMat m;
		 if ( rotang!=0 ) m.rot ( rotaxis, rotang );
		 m.setrans ( trans );
		 if ( scalefac!=GsVec::one ) { gsout<<"Scaling not implemented!\n"; }
		 if ( stack.size()==0 ) { gsout<<"Empty stack in Transform!\n"; return false; }
		 GsMat mult = m*stack.top();
		 stack.top()=mult;
		 GS_TRACE3 ( stack.top() );
	   }
	  else if ( s=="MatrixTransform" )
	   { in.get(); // '{'
		 in.get(); // matrix
		 GsMat m; in >> m;
		 in.get(); // '}'
		 if ( stack.size()==0 ) { gsout<<"Empty stack in MatrixTransform!\n"; return false; }
		 if ( in.ltoken()[0]!='}' ) { gsout<<"MatrixTransform type not supported!\n"; return false; }
		 GsMat mult = m*stack.top();
		 stack.top()=mult;
		 GS_TRACE3 ( stack.top() );
	   }
	  else if ( s=="DEF" )
	   { defs.push();
		 in.get(); // key
		 defs.top()->key = in.ltoken();
		 in.get(); // only accepting material
		 if ( in.ltoken()=="Material" )
		  { defs.top()->mi = M.size();
			M.push().init();
			loadmtl ( in, M.top() );
			mi = defs.size()-1;
			GS_TRACE2("DEF mi="<<mi);
		  }
		 else
		  { defs.pop(); SKIP_SECTION; }
	   }
	  else if ( s=="USE" )
	   { in.get(); // key
		 mi=0;
		 for ( int i=0; i<defs.size(); i++ )
		  if ( defs[i]->key==in.ltoken() ) { mi=i; break; }
		 GS_TRACE2("USE mi="<<mi);
	   }
	  else if ( s=="Coordinate3" )
	   { in.get(); // {
		 in.get(); // point
		 in.get(); // [
		 GsVec v;
		 GsMat m = stack.top();
		 vi = V.size(); // save starting point
		 while ( true )
		  { if ( in.ltoken()[0]==']' ) break;
			in >> v;
			V.push() = v * m;
			GS_TRACE3(V.top());
			in.get(); // , or ]
		  }
		 in.get();
		 if ( in.ltoken()[0]!='}' ) { gsout<<"} expected!\n"; }
	   }
	  else if ( s=="Normal" )
	   { in.get(); // {
		 in.get(); // vector
		 in.get(); // [
		 GsVec n;
		 GsMat m = stack.top();
		 m.setrans ( 0, 0, 0 ); // no 
		 ni = N.size(); // save starting point
		 while ( true )
		  { if ( in.ltoken()[0]==']' ) break;
			in >> n;
			N.push() = n * m;
			GS_TRACE3(N.top());
			in.get(); // , or ]
		  }
		 in.get();
		 if ( in.ltoken()[0]!='}' ) { gsout<<"} expected!\n"; }
	   }
	  else if ( s=="IndexedFaceSet" )
	   { in.get(); // {
		 in.get(); // coordIndex
		 GS_TRACE1( "Parsing "<<in.ltoken()<<"..." );
		 in.get(); // [
		 while ( true )
		  { if ( in.ltoken()[0]==']' ) break;
			F.push();
			if ( M.size()>0 ) Fm.push() = defs[mi]->mi;
			F.top().a = in.geti()+vi; in.get(); // ,
			F.top().b = in.geti()+vi; in.get(); // ,
			F.top().c = in.geti()+vi; in.get(); // ,
			if ( in.geti()!=-1 ) { gsout<<"coordIndex -1 expected...\n"; return false; }
			in.get(); // , or ]
		  }
		 in.get(); // normalIndex or '}'
		 if ( in.ltoken()[0]=='n' )
		  {
			GS_TRACE1( "Parsing "<<in.ltoken()<<"..." );
			in.get(); // [
			while ( true )
			 { if ( in.ltoken()[0]==']' ) break;
			   Fn.push();
			   Fn.top().a = in.geti()+ni; in.get(); // ,
			   Fn.top().b = in.geti()+ni; in.get(); // ,
			   Fn.top().c = in.geti()+ni; in.get(); // ,
			   if ( in.geti()!=-1 ) { gsout<<"normalIndex -1 expected...\n"; return false; }
			   in.get(); // , or ]
			 }
			in.get();
		  }
		 if ( in.ltoken()[0]!='}' ) { gsout<<"} expected!\n"; }
	   }
	  else
	   { GS_TRACE1 ( "Skipping keyword ["<<s<<"]..." );
		 SKIP_SECTION;
	   }
	}

   if ( N.size()>0 && Fn.size()!=F.size() ) Fn=F; // security

   define_groups ( Fm ); // make material groups from local Fm array

   GS_TRACE1 ( "OK.\n" );

   return true;
 }

//============================ EOF ===============================
