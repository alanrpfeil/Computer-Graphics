/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# pragma once

# include <sigogl/ws_viewer.h>

class ShapeViewer : public WsViewer
{  public :
	enum Cmd { CmdClear, CmdPoints, CmdLines, CmdLines2, CmdText, CmdTriangles, CmdPolygons, 
			   CmdColorSurf, CmdPrimitive, CmdQuit };
	float MaxCoord;
   public :
	ShapeViewer ( int x, int y, int w, int h, const char* l );
	void add ( SnNode* n, GsPnt p );
	void new_primitive ();
	void new_points ();
	void new_lines ();
	void new_lines2 ();
	void new_text ();
	void new_triangles ();
	void new_polygons ();
	void new_colorsurf ();
	virtual int handle_keyboard ( const GsEvent &e ) override;
	virtual int uievent ( int e ) override;
};
