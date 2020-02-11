/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef MV_WINDOW_H
# define MV_WINDOW_H

# include <sigogl/ws_viewer.h>
# include "mv_scene.h"

class UiPanel;
class UiCheckButton;

class MvWindow : public WsViewer
 { private :
	float _last_crease_angle_deg;
	float _last_normals_prec_deg;
	GsString _lastfile;
	UiCheckButton* _viewbut[4];
	UiPanel* _spanel;
	int _spanelcmds;

   public :
	MvWindow ( int x, int y, int w, int h, const char *label=0 );
   ~MvWindow ();
	void build_ui ();
	bool open ( const char *file=0 ) { return 0; }
	//bool save ( int i=-1 ); // -1 is current one
	void add_model ( SnModel* sm );
	void load_model ( const char* fname=0 ); // if 0 will open file section dialog
	MvScene* root() { return (MvScene*)WsViewer::root(); }
	void select_top ();
	void update_selection ( int val ); // 1:sel all, 0:unselect, -1:upd scene sel from menu
	void update_visibility ();
//	SnModel* cur_smodel () const { return 0; }
//	GsModel* cur_model () const { return 0; }
//	void cur_close ();
//	void open_numbered ( const char* first_file=0 );

//	void edit_primitive ( SnModel* m=0 );

//	void export_skeleton ( const char* file=0, const char* name=0 );

   public :
   public :
	virtual int uievent ( int cmd ) override;
	virtual int handle_keyboard ( const GsEvent& e ) override;
	virtual int handle_scene_event ( const GsEvent& e ) override;
};

# endif // MV_WINDOW_H
