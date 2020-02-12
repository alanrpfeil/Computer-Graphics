#include "Sn_myTorus.h"
#include "glr_mynode.h"

const char* SnMyTorus::class_name = "SnMyTorus";
SN_SHAPE_RENDERER_DEFINITIONS(SnMyTorus);

SnMyTorus::SnMyTorus() : SnShape(class_name)
{
	r = 0.10;				//"thickness" of torus (cross-sected circle)
	R = 0.05;				//radius is 1 (revolution of cross section)
	n = 30;
	if (!SnMyTorus::renderer_instantiator) SnMyTorusRegisterRenderer();
}

SnMyTorus::~SnMyTorus() {}

static SnShapeRenderer* GlrMyNodeInstantiator()
{
	return new GlrMyNode;
}

void SnMyTorus::get_bounding_box(GsBox& b) const
{
	// the bounding box is needed for camera view_all computations
	b.set(init, init + GsVec(R, r, 0));
}

void SnMyTorusRegisterRenderer()
{
	SnMyTorus::renderer_instantiator = &GlrMyNodeInstantiator;
}