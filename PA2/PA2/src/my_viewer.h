# pragma once

# include <sig/sn_poly_editor.h>
# include <sig/sn_lines2.h>

# include <sigogl/ui_button.h>
# include <sigogl/ws_viewer.h>

# include <vector>

// Viewer for this example:
class MyViewer : public WsViewer
{  protected :
	enum MenuEv { EvNormals, EvAnimate, EvExit };
	UiCheckButton* _nbut;
	SnGroup* _gHands;
	SnGroup* _gLight;
	SnTransform* _tShadow1;
	SnTransform* _tShadow2;
	bool _animating;
	float totSecDegrees = 0, totMinDegrees = 0;
	GsVec lightPos = GsVec(0,5,5);
	float sh_Length = 2.0f;
	float mh_Length = 1.8f;
	float dialR = 2.5f;
   public :
	MyViewer ( int x, int y, int w, int h, const char* l );
	void build_ui ();
	void add_model ( SnGroup* parentg, SnShape* s, GsVec p );
	void build_scene ();
	void show_normals ( bool b );
	void run_animation ();
	void update_shadow ();
	virtual int handle_keyboard ( const GsEvent &e ) override;
	virtual int uievent ( int e ) override;
};

