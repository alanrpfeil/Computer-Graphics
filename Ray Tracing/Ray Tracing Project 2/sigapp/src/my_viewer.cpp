
# include "my_viewer.h"

# include <sigogl/ui_button.h>
# include <sigogl/ui_radio_button.h>
# include <sig/sn_primitive.h>
# include <sig/sn_transform.h>
# include <sig/sn_manipulator.h>
# include "sn_mynode.h"
# include "csg_prim.h"
# include <fstream> 
# include <sigogl/ws_run.h>
# include <sigogl/gl_tools.h>
# include <iostream> 
# include <cstdlib> 
# include <cstdio> 
# include <cmath> 
# include <cassert> 
# include <iostream>
# include <string>

#define PI 3.141592653589793

void dbg_csgprim() {
	CSGPrimitive* prim =  (CSGPrimitive*)new GsPrimitive();
	prim->sphere(5); //make sphere prim with radius = 5
}

SnModel* add_csgprim() {

	//create a new empty GsModel
	GsModel* G = new GsModel();

	//create a CSGPrimitive with ra (radius) = 5; centered at (0,0,0).
	CSGPrimitive prim; prim.ra = 5; prim.center = GsVec(0, 0, 0);

	//make the GsModel represent the CSGPrimitive
	G->make_primitive(prim);

	//make the SnModel represent the GsModel
	SnModel* M = new SnModel(G);
	return M;
}

MyViewer::MyViewer ( int x, int y, int w, int h, const char* l ) : WsViewer(x,y,w,h,l)
{
	add_ui ();
	//add_mynode ();
	build_scene();
	//trace(WsViewer::w, WsViewer::h, 2);
	camera().eye = GsVec(0, 0, 1);
	camera().center = GsVec(0, 0, 0);

	render_pixel(640, 480, 0);	//640x480 res and bounces given
}

void MyViewer::add_ui ()
{
	UiPanel *p;
	UiManager* uim = WsWindow::uim();
	p = uim->add_panel ( "", UiPanel::HorizLeft );
	//p->add ( new UiButton ( "Add", EvAdd ) );
	p->add ( new UiButton ( "OpenGL", EvInfo ) );
	p->add ( new UiButton ( "Exit", EvExit ) );
}

void MyViewer::add_mynode ( int n )
{
	SnMyNode* c;

	float r=0.15f; // position range
	while ( n-->0 )
	{
		c = new SnMyNode;
		if ( mcolorbut->value() )
			c->multicolor = true;
		else
			c->color ( GsColor::random() );

		c->init.set ( gs_random(-r,r), gs_random(-r,r), gs_random(-r,r) );
		c->width = gs_random(0.001f,r);
		c->height = gs_random(0.001f,r*2);

		// Example how to print/debug your generated data:
		// gsout<<n<<": "<<c->color()<<gsnl;

		rootg()->add(c);
	}
}

void MyViewer::add_model(SnGroup* parentg, SnShape* s, GsVec p)
{
	SnGroup* g = new SnGroup; // group will have: transf, shape, lines
	SnTransform* t = new SnTransform;
	SnLines* l = new SnLines;
	l->color(GsColor::orange);
	g->add(t);
	g->add(s);
	g->add(l);
	g->separator(true);
	parentg->add(g);
}

GsVec get_rgb(GsMaterial mat) {
		float r, g, b;
		r = mat.ambient.r;
		g = mat.ambient.g;
		b = mat.ambient.b;
		return GsVec(r, g, b);
	}

GsVec MyViewer::trace(GsVec pixeleyeray, GsVec pixeleyeraydirection, GsArray<CSGPrimitive> Shapes, int depth) {
	GsVec color = GsVec(0, 0, 0);	//RGB
	//float illuminationequation;
	bool intersection = false;
	float alpha = 1.0;

	bool illumination = true; //Is a point in shadow?

	GsVec ray;
	GsVec direction;
	GsVec incidentpoint;
	GsVec incidentpointdirection;
	GsVec shapenormal;
	GsVec incidentpoint2;

	ray = pixeleyeray;
	direction = pixeleyeraydirection;

	GsArray<GsVec> LightSources;
	LightSources.push(lightPos);

	float minimumdistance = 999;
	int k = 0;  //shape index   

	//gsout << "def here\n";

	for (int i = 0; i < Shapes.size(); i++) { //Primary ray intersection
		if (Shapes[i].intersects(ray, direction, incidentpoint, GsVec(0, 0, 0)) == 1 || Shapes[i].intersects(ray, direction, incidentpoint, incidentpoint2)) {
			//gsout << "def here maybe\n";
			if (minimumdistance > dist(ray, incidentpoint)) {
				minimumdistance = dist(ray, incidentpoint);
				k = i;
				intersection = true;
				break;
			}
		}
	}
	if (intersection) {
		ray = incidentpoint;
		intersection = false;
		minimumdistance = 999;
		GsVec incidentLight = GsVec(0, 0, 0);

		for (int i = 0; i < LightSources.size(); i++) { // Shadow Feelers - not working for multiple light sources
			direction = LightSources[i] - ray;    //mathematics unclear
			direction.normalize();
			for (int j = 0; j < Shapes.size(); j++) {
				if (j == k) {
					continue;
				}
				if ((Shapes[j].intersects(ray, direction, incidentLight, GsVec(0, 0, 0)) == 1) || (Shapes[j].intersects(ray, direction, incidentLight, GsVec(0, 0, 0)) == 2)) { //Last GsVec is used by intersects function to be modified. Passing empty/default GsVec.
					intersection = true;
				}
			}

			if (!intersection) {
				//gsout << "finally here\n";
				color += alpha * get_rgb(Shapes[k].material); //adds ambient color of intersected object. Alpha is a fraction [0, 1] that grabs a part of the color
			}

			//Reflection Section
			//calculate reflection ray
			if (depth != 0) {
				GsVec reflectiondirection;

				reflectiondirection = direction - shapenormal * 2 * dot(direction, shapenormal);
				reflectiondirection.normalize();
				color += trace(ray + shapenormal, reflectiondirection, Shapes, --depth);  //compute ray point
			}

		}
	}

	return color;
}

GsVec get_rgbColor() {
	float r, g, b;
	r = GsColor::random().r;
	g = GsColor::random().g;
	b = GsColor::random().b;
	return GsVec(r, g, b);
}

void MyViewer::render_pixel(int width, int height, int bounces) {
	
	GsVec evalPixel, rayDir;
	pixels = new GsVec[width * height];

	GsVec* image = new GsVec[width * height];
	float invWidth = 1 / float(width), invHeight = 1 / float(height);
	float fov = 60, aspectratio = width / float(height);
	float angle = tan(PI * 0.5 * fov / 180.);

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {

			evalPixel.x = (2 * ((j + 0.5) * invWidth) - 1) * angle * aspectratio;
			evalPixel.y = (1 - 2 * ((i + 0.5) * invHeight)) * angle;

			rayDir = evalPixel - camera().eye;
			rayDir.normalize();
			pixels[(i * width) + j] = trace(evalPixel, rayDir, prims, bounces);

		}
	}

	std::ofstream ofs("./untitled.ppm", std::ios::out | std::ios::binary);
	ofs << "P6\n" << width << " " << height << "\n255\n";
	for (int i = 0; i < (width * height); i++) {
		ofs << (unsigned char)(std::fmin(float(1), pixels[i].x) * 255) <<
			(unsigned char)(std::fmin(float(1), pixels[i].y) * 255) <<
			(unsigned char)(std::fmin(float(1), pixels[i].z) * 255);
	}
	ofs.close();

	delete[] image;
}

void MyViewer::build_scene() {

	//Light
	SnPrimitive* light_s = new SnPrimitive();
	light_s->prim().sphere(0.2f);
	light_s->prim().nfaces = 20;
	light_s->prim().material.diffuse = GsColor::yellow;
	SnGroup* gLight = new SnGroup(new SnTransform, light_s, true);
	gLight->get<SnTransform>(0)->get().setrans(lightPos);

	//Background (for bouncing)
	SnGroup* backdrop = new SnGroup();

	GsBox back; back.a = GsVec(-3.0f, 0, -1.0f); back.b = GsVec(3.0f, 3, -1.2f);
	CSGPrimitive backCSG; backCSG.box().a = GsVec(-3.0f, 0, -1.0f); backCSG.box().b = GsVec(3.0f, 3, -1.2f);
	backCSG.type = CSGPrimitive::Box;
	SnPrimitive* backpiece = new SnPrimitive();
	//backpiece->prim().box(back);

	GsBox left; left.a = GsVec(-3.0f, 0, -1.2f); left.b = GsVec(-3.0f, 3, 2.0f);
	CSGPrimitive leftCSG; leftCSG.box().a = GsVec(-3.0f, 0, -1.2f); leftCSG.box().b = GsVec(-3.0f, 3, 2.0f);
	leftCSG.type = CSGPrimitive::Box;
	SnPrimitive* leftpiece = new SnPrimitive();
	//leftpiece->prim().box(left);

	GsBox right; right.a = GsVec(3.0f, 0, -1.2f); right.b = GsVec(3.0f, 3, 2.0f);
	CSGPrimitive rightCSG; rightCSG.box().a = GsVec(3.0f, 0, -1.2f); rightCSG.box().b = GsVec(3.0f, 3, 2.0f);
	rightCSG.type = CSGPrimitive::Box;
	SnPrimitive* rightpiece = new SnPrimitive();
	//rightpiece->prim().box(right);
	
	GsBox top; top.a = GsVec(-3.0f, 3, -1.0f); top.b = GsVec(3.0f, 3, 2.0f);
	CSGPrimitive topCSG; topCSG.box().a = GsVec(-3.0f, 3, -1.0f); topCSG.box().b = GsVec(3.0f, 3, 2.0f);
	rightCSG.type = CSGPrimitive::Box;
	SnPrimitive* toppiece = new SnPrimitive();
	//toppiece->prim().box(top);

	GsBox bottom; bottom.a = GsVec(-3.0f, 0, -1.2f); bottom.b = GsVec(3.0f, 0, 2.0f);
	CSGPrimitive bottomCSG; bottomCSG.box().a = GsVec(-3.0f, 0, -1.2f); bottomCSG.box().b = GsVec(3.0f, 0, 2.0f);
	bottomCSG.type = CSGPrimitive::Box;
	SnPrimitive* bottompiece = new SnPrimitive();
	//bottompiece->prim().box(bottom);

	backpiece->prim().material.diffuse = GsColor::darkgray;
	toppiece->prim().material.diffuse = GsColor::darkgray;
	leftpiece->prim().material.diffuse = GsColor::darkgray;
	rightpiece->prim().material.diffuse = GsColor::darkgray;
	bottompiece->prim().material.diffuse = GsColor::darkgray;

	/*
	add_model(backdrop, backpiece, GsVec(0, 0, 0));
	add_model(backdrop, toppiece, GsVec(0, 0, 0));
	add_model(backdrop, leftpiece, GsVec(0, 0, 0));
	add_model(backdrop, rightpiece, GsVec(0, 0, 0));
	add_model(backdrop, bottompiece, GsVec(0, 0, 0));
	*/
	
	/*
	prims.push(backCSG);
	prims.push(leftCSG);
	prims.push(rightCSG);
	prims.push(topCSG);
	prims.push(bottomCSG);
	*/

	// Cylinder
	/*
	SnPrimitive* p = new SnPrimitive();
	SnGroup* gCylinder = new SnGroup();
	p->prim().cylinder(GsVec(0, 0.01f, 0), GsVec(0, 2.0f, 0), 1.0f);
	p->prim().material.diffuse = GsColor::blue;
	add_model(gCylinder, p, GsVec(0, 10.0f, 0));
	*/

	//Sphere
	SnPrimitive* sphere1 = new SnPrimitive();
	GsMat dSphere = GsMat(1, 0, 0, 1.2f, 0, 1, 0, 1.2f, 0, 0, 1, 1.5f, 0, 0, 0, 1);
	SnTransform* displaceSphere = new SnTransform(dSphere);
	GsMat dSphereInv = GsMat(1, 0, 0, -1.2f, 0, 1, 0, -1.2f, 0, 0, 1, -1.5f, 0, 0, 0, 1);
	SnTransform* displaceSphereInv = new SnTransform(dSphereInv);
	SnGroup* gSphere = new SnGroup();
	gSphere->separator(true);
	sphere1->prim().sphere(0.8f);
	sphere1->prim().nfaces = 60;
	sphere1->prim().material.ambient = GsColor::lightgray;
	sphere1->prim().material.diffuse = GsColor::white;
	sphere1->prim().material.specular = GsColor::white;
	//add_model(gSphere, sphere1, GsVec(10.0f, 0, 4.0f));

	CSGPrimitive sphereCSG2;
	sphereCSG2.sphere(0.65f);
	sphereCSG2.nfaces = 60;
	sphereCSG2.material.ambient = GsColor::blue;
	sphereCSG2.material.diffuse = GsColor::blue;
	sphereCSG2.material.specular = GsColor::white;
	sphereCSG2.center = GsVec(0.2f, 0.5f, (float)-96);
	prims.push(sphereCSG2);

	CSGPrimitive sphereCSG3;
	sphereCSG3.sphere(0.5f);
	sphereCSG3.nfaces = 60;
	sphereCSG3.material.ambient = GsColor::green;
	sphereCSG3.material.diffuse = GsColor::green;
	sphereCSG3.material.specular = GsColor::white;
	sphereCSG3.center = GsVec(-1.2f, -0.5f, (float)-98);
	prims.push(sphereCSG3);

	CSGPrimitive sphereCSG;
	sphereCSG.sphere(0.8f);
	sphereCSG.nfaces = 60;
	sphereCSG.material.ambient = GsColor::red;
	sphereCSG.material.diffuse = GsColor::darkred;
	sphereCSG.material.specular = GsColor::white;
	sphereCSG.center = GsVec(0, 0, -100);
	prims.push(sphereCSG);

	//Box
	/*
	SnPrimitive* cube = new SnPrimitive();
	cube->prim().box().a = GsPnt(-1.2f, 0, 2.0f);
	cube->prim().box().b = GsPnt(-0.2f, 0.7f, 0.8f);
	SnGroup* gcube = new SnGroup();
	gcube->separator(true);
	GsMat dbox = GsMat(1, 0, 0, -1.2f, 0, 1, 0, 0, 0, 0, 1, 2.0f, 0, 0, 0, 1);
	SnTransform* displaceBox = new SnTransform(dbox);
	add_model(gcube, cube, GsVec(0, 0, 0));
	*/

	//rootg()->add(backdrop);
	//rootg()->add(gLight);
	//rootg()->add(gCylinder);

	//rootg()->add(displaceSphere);
	//rootg()->add(gSphere);
	//rootg()->add(displaceSphereInv);

	//rootg()->add(displaceBox);
	//rootg()->add(gcube);
}

int MyViewer::handle_keyboard ( const GsEvent &e )
{
	int ret = WsViewer::handle_keyboard ( e ); // 1st let system check events
	if ( ret ) return ret;

	switch ( e.key )
	{	case GsEvent::KeyEsc : gs_exit(); return 1;
		case GsEvent::KeyLeft: gsout<<"Left\n"; return 1;
		// etc
		default: gsout<<"Key pressed: "<<e.key<<gsnl;
	}

	return 0;
}

int MyViewer::uievent ( int e )
{
	switch ( e )
	{	case EvAdd: add_mynode(1); return 1;

		case EvInfo:
		{	if ( output().len()>0 ) { output(""); return 1; }
			output_pos(0,30);
			activate_ogl_context(); // we need an active context
			GsOutput o; o.init(output()); gl_print_info(&o); // print info to viewer
			return 1;
		}

		case EvExit: gs_exit();
	}
	return WsViewer::uievent(e);
}
