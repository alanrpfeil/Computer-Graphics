# include "sn_mynode.h"

const char* SnMyNode::class_name = "SnMyNode"; // static
SN_SHAPE_RENDERER_DEFINITIONS(SnMyNode);

//===== SnMyNode =====

#define PI 3.14157

SnMyNode::SnMyNode () : SnShape ( class_name )
{
	r = 0.005;				//"thickness" of torus (cross-sected circle)
	R = 0.50;				//radius is 1 (revolution of cross section)
	n = 30;

	if (!SnMyNode::renderer_instantiator) SnMyNodeRegisterRenderer();
}

SnMyNode::~SnMyNode ()
{
}

GsModel* SnMyNode::modelize() {	//INCOMPLETE?

	GsModel* t = new GsModel();
	GsVec o = init;

	float x, y, z, xa, xb, ya, yb, za, zb, xc, yc, zc;

	for (int i = 0; i < n; i++) {		//for every ring
		for (int j = 0; j <= n; j++) {		//for every point

			x = (R + (r * cos(2 * PI / n * i))) * cos(2 * PI / n * j);
			y = (R + (r * cos(2 * PI / n * i))) * sin(2 * PI / n * j);
			z = r * sin(2 * PI / n * i);

			xa = (R + (r * cos(2 * PI / n * (i + 1)))) * cos(2 * PI / n * j);		//getting next i iteration to connect to
			ya = (R + (r * cos(2 * PI / n * (i + 1)))) * sin(2 * PI / n * j);
			za = r * sin(2 * PI / n * (i + 1));

			xb = (R + (r * cos(2 * PI / n * i))) * cos(2 * PI / n * (j + 1));	//getting next j iteration to connect to
			yb = (R + (r * cos(2 * PI / n * i))) * sin(2 * PI / n * (j + 1));
			zb = r * sin(2 * PI / n * i);

			xc = (R + (r * cos(2 * PI / n * (i + 1)))) * cos(2 * PI / n * (j + 1));	//getting next j+i iteration to connect to
			yc = (R + (r * cos(2 * PI / n * (i + 1)))) * sin(2 * PI / n * (j + 1));
			zc = r * sin(2 * PI / n * (i + 1));

			//VERTICES
			GsVec temp = GsVec(x, y, z);
			GsVec temp2 = GsVec(xa, ya, za);
			GsVec temp3 = GsVec(xb, yb, zb);
			GsVec temp4 = GsVec(xc, yc, zc);
			GsVec temp5 = GsVec(xb, yb, zb);
			GsVec temp6 = GsVec(xa, ya, za);

			//VERTICES 1
			t->V.push() = o + temp;
			t->V.push() = o + temp2;
			t->V.push() = o + temp3;
			//FACE 1
			t->F.push() = GsModel::Face(i*6 + j*6, i*6 + j*6 + 1, i*6 + j*6 + 2);

			//VERTICES 2
			t->V.push() = o + temp4;
			t->V.push() = o + temp5;
			t->V.push() = o + temp6;
			//FACE 1
			t->F.push() = GsModel::Face(i*6 + j*6 + 3, i*6 + j*6 + 4, i*6 + j*6 + 5);

			//NORMALS:
			t->N.push() = cross(temp, temp2);
			t->N.push() = cross(temp2, temp3);
			t->N.push() = cross(temp3, temp);
			t->N.push() = cross(temp4, temp5);
			t->N.push() = cross(temp5, temp6);
			t->N.push() = cross(temp4, temp6);

		}
	}

	return t;
}

void SnMyNode::get_bounding_box ( GsBox& b ) const
{
	// the bounding box is needed for camera view_all computations
	b.set(init + GsVec(-R,-R, 0) , init + GsVec(R, R, 0));
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