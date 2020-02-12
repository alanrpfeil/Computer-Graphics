# pragma once

# include <sig/sn_poly_editor.h>
# include <sig/sn_lines2.h>

# include <sigogl/ui_check_button.h>
# include <sigogl/ui_slider.h>
# include <sigogl/ws_viewer.h>

// Viewer for this example:
class MyViewer : public WsViewer
{  protected :
	enum MenuEv { EvExit, EvViewCurveA, EvViewCurveB, EvViewCurveC, EvViewCurveD, EvDeltaT };
	UiCheckButton *_viewA, *_viewB, *_viewC, *_viewD;
	UiSlider* _slider;
	SnPolyEditor* _polyed;
	SnLines2 *_curveA, *_curveB, *_curveC, *_curveD;

   public :
	MyViewer ( int x, int y, int w, int h, const char* l );
	void build_ui ();
	void update_scene ();
	virtual int uievent ( int e ) override;
};

