/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef MAIN_H
# define MAIN_H

# include <sig/sn_lines2.h>
# include <sig/sn_planar_objects.h>
# include <sigogl/gl_texture.h>
# include <sigogl/ui_input.h>
# include <sigogl/ws_viewer.h>

enum ExampleFont
	{	DroidSans_256x190_18_P1OS2,
		DroidSans_128x188_12_P1OS2,
		FontTest
	};

extern ExampleFont CurExFont;

bool makefont ( const GsArray<gsbyte>& ttfdata, int w, int h, int padding, int fontsize, int osamp, GsBytemap& bmap );

// Events:
enum Cmd { CmdLoad, CmdGenerateAA, CmdGenerateDF, CmdTest, CmdHelp, CmdQuit,
		   CmdImgData, CmdRedraw,
		   CmdAny };

class MyWindow : public WsViewer
{  public :
	GlTexture* _texture; 
	SnPlanarObjects* _texturepo;
	SnPlanarObjects* _testpo;
	SnLines2* _testl;
	SnGroup* _testg;
	GsArray<gsbyte> _ttfdata;
	UiInput *_uiw, *_uih, *_uip, *_uios, *_uifs;

   public :
	MyWindow ( int w, int h, const char* l );
	void build_ui ();
	void load_ttf ();
	void generate ();
	void test ();
	virtual int uievent ( int e ) override;
};

# endif // MAIN_H
