#pragma once

# include <sig/gs_vec.h>
# include <sig/sn_shape.h>

class GsPolygon; // forward declaration
class GsFontStyle; // forward declaration
class UiLabel; // forward declaration

class SnMyTorus : public SnShape
{
public:

	GsVec init;
	float r, R;
	int n;		//resolution

	static const char* class_name; //<! Contains string SnLines2
	SN_SHAPE_RENDERER_DECLARATIONS;

	SnMyTorus();

	~SnMyTorus();

	virtual void get_bounding_box(GsBox& b) const override;

};

void SnMyTorusRegisterRenderer();