# include "sn_mynode.h"

const char* SnMyNode::class_name = "SnMyNode"; // static
SN_SHAPE_RENDERER_DEFINITIONS(SnMyNode);

//===== SnMyNode =====

SnMyNode::SnMyNode () : SnShape ( class_name )
{
	r = 0.15;				//"thickness" of torus (cross-sected circle)
	R = 0.05;				//radius is 1 (revolution of cross section)
	n = 30;
	if (!SnMyNode::renderer_instantiator) SnMyNodeRegisterRenderer();
}

SnMyNode::~SnMyNode ()
{
}

void SnMyNode::get_bounding_box ( GsBox& b ) const
{
	// the bounding box is needed for camera view_all computations
	b.set ( init, init+GsVec(R,r,0) );
}

//===== Renderer Instantiator =====

# include "glr_mynode.h"

static SnShapeRenderer* GlrMyNodeInstantiator ()
{
	return new GlrMyNode;
}

void SnMyNodeRegisterRenderer ()
{
	SnMyNode::renderer_instantiator = &GlrMyNodeInstantiator;
}
