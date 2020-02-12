
# include "my_viewer.h"

# include <sigogl/ui_button.h>
# include <sigogl/ui_radio_button.h>
# include <sig/sn_primitive.h>
# include <sig/sn_transform.h>
# include <sig/sn_manipulator.h>

# include <sigogl/ws_run.h>

static void my_polyed_callback ( SnPolyEditor* pe, enum SnPolyEditor::Event e, int pid )
{
	MyViewer* v = (MyViewer*)pe->userdata();
	if ( e==SnPolyEditor::PostMovement || e==SnPolyEditor::PostEdition || e==SnPolyEditor::PostInsertion )
	{	v->update_scene ();
	}
}

MyViewer::MyViewer ( int x, int y, int w, int h, const char* l ) : WsViewer(x,y,w,h,l)
{
	rootg()->add ( _polyed = new SnPolyEditor );
	rootg()->add ( _curveA = new SnLines2 );
	rootg()->add ( _curveB = new SnLines2 );
	rootg()->add(_curveC = new SnLines2);
	rootg()->add(_curveD = new SnLines2);

	_curveA->color ( GsColor(20,200,25) );
	_curveA->line_width ( 2.0f );

	// you may add new curves here

	_curveB->color(GsColor(200, 20, 25));
	_curveB->line_width(2.0f);

	_curveC->color(GsColor(50, 20, 205));
	_curveC->line_width(2.0f);

	_curveD->color(GsColor(200, 100, 200));
	_curveD->line_width(2.0f);

	// set initial control polygon:
	_polyed->callback ( my_polyed_callback, this );
	_polyed->max_polygons (1);
	_polyed->solid_drawing (0);
	GsPolygon& P = _polyed->polygons()->push();
	P.setpoly ( "-2 -2  -1 1  1 0  2 -2" );
	P.open(true);

	// start:
	build_ui ();
	update_scene ();
	message() = "Click on polygon to edit, use Esc to switch edition mode, Del deletes selected points. Enjoy!";
}

void MyViewer::build_ui ()
{
	UiPanel *p;
	p = uim()->add_panel ( "", UiPanel::HorizLeft );

	p->add ( _viewA=new UiCheckButton ( "CurveA", EvViewCurveA, true ) );
	p->add ( _viewB=new UiCheckButton ( "CurveB", EvViewCurveB, true ) );
	p->add( _viewC = new UiCheckButton("CurveC", EvViewCurveC, true));
	p->add( _viewD = new UiCheckButton("CurveD", EvViewCurveD, true));

	p->add ( _slider=new UiSlider ( " dt:", EvDeltaT, 0,0,150 ) );
	_slider->separate();
	_slider->range ( 0.01f, 0.25f );

	p->add ( new UiButton ( "Exit", EvExit ) ); p->top()->separate();
}

// Use one function for each curve type. Feel free to update/adapt parameter list etc.
static GsPnt2 eval_curveX ( float t, const GsArray<GsPnt2>& P )	//bezier
{
	GsArray<GsPnt2> answer;
	answer = P;

		int i = answer.size() - 1;
		while (i > 0) {
			for (int k = 0; k < i; k++) {
				answer[k].x = answer[k].x + t * (answer[k + 1].x - answer[k].x);
				answer[k].y = answer[k].y + t * (answer[k + 1].y - answer[k].y);
			}
			i--;
		}
		return answer[0];
}

static GsPnt2 eval_curveY (int k, int degree, int i, float t, float* knots, const GsArray<GsPnt2>& P)	//bspline
{

	if (k == 0) {
		return P[i];
	} else {
		double alpha = (t - knots[i]) / (knots[i + degree + 1 - k] - knots[i]);
		return (eval_curveY(k - 1, degree, i - 1, t, knots, P) * (1 - alpha) + eval_curveY(k - 1, degree, i, t, knots, P) * alpha);
	}

}

static GsPnt2 eval_curveC( float t, const GsArray<GsPnt2>& P)	
{
		float t2 = t * t;
		float t3 = t * t * t;
		GsPnt2 pnt;
		GsPnt2 p1, p2, p3, p4;

		p1 = P[0];
		p2 = P[1];
		p3 = P[2];
		p4 = P[3];

		pnt.x = ((-t3 + 2 * t2 - t) * (p1.x) + (3 * t3 - 5 * t2 + 2) * (p2.x) + (-3 * t3 + 4 * t2 + t) * (p3.x) + (t3 - t2) * (p4.x)) / 2;
		pnt.y = ((-t3 + 2 * t2 - t) * (p1.y) + (3 * t3 - 5 * t2 + 2) * (p2.y) + (-3 * t3 + 4 * t2 + t) * (p3.y) + (t3 - t2) * (p4.y)) / 2;

		return pnt;
}

static GsPnt2 eval_curveD(float t, const GsArray<GsPnt2>& P)
{
	GsArray<GsPnt2> answer;
	answer = P;

	

	return answer[0];
}

void MyViewer::update_scene ()
{
	// Initializations:
	_curveA->init();
	_curveB->init();
	_curveC->init();
	_curveD->init();

	// Access the control polygon:
	GsPolygon& P = _polyed->polygon(0);
	float deltat = _slider->value();
	int k = 3;
	float knotvals[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
	int i = P.size();

	if ( _viewA->value() ) // show curve
	{
		// Add your curves below and select the correct one(s) to be displayed.
		_curveA->begin_polyline();
		for ( float t=0; t<1.0f; t+=deltat ) // note: the t range may change according to the curve
		{	_curveA->push ( eval_curveX ( t, P ) );
		}
		_curveA->push ( P.top() ); // ensure final point is there
		_curveA->end_polyline();
	}
	
	if ( _viewB->value() ) // show curve
	{
		_curveB->begin_polyline();

		for (float t = 0; t < 1.0f; t += deltat) // note: the t range may change according to the curve
		{
			_curveB->push(eval_curveY(k, k-1, i, t, knotvals, P));
		}
		_curveB->push(P.top()); // ensure final point is there
		_curveB->end_polyline();
	}
	
	if ( _viewC->value() ) // show curve
	{
		// Add your curves below and select the correct one(s) to be displayed.
		_curveC->begin_polyline();
		for (float t = 0; t < 1.0f; t += deltat) // note: the t range may change according to the curve
		{
			_curveC->push(eval_curveC(t, P));
		}
		_curveC->push(P.top()); // ensure final point is there
		_curveC->end_polyline();
	}

	/*
	if (_viewD->value()) // show curve
	{
		// Add your curves below and select the correct one(s) to be displayed.
		_curveD->begin_polyline();
		for (float t = 0; t < 1.0f; t += deltat) // note: the t range may change according to the curve
		{
			_curveD->push(eval_curveD(t, P));
		}
		_curveD->push(P.top()); // ensure final point is there
		_curveD->end_polyline();
	}
	*/

}

int MyViewer::uievent ( int e )
{
	switch ( e )
	{
		case EvViewCurveA:
		case EvViewCurveB:	update_scene(); return 1;
		case EvViewCurveC:	update_scene(); return 1;
		case EvViewCurveD:	update_scene(); return 1;
		case EvDeltaT:		update_scene(); return 1;
		case EvExit: gs_exit();
	}
	return WsViewer::uievent(e);
}
