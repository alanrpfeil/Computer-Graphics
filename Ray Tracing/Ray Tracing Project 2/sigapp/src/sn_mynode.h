# pragma once

# include <sig/gs_vec.h>
# include <sig/sn_shape.h>

class GsPolygon; // forward declaration
class GsFontStyle; // forward declaration
class UiLabel; // forward declaration

class SnMyNode : public SnShape
{  public :
	GsVec init;
	float width; // default is 0.5f
	float height; // default is 1.0f
	bool multicolor; // default is false

   public :
	static const char* class_name; //<! Contains string SnLines2
	SN_SHAPE_RENDERER_DECLARATIONS;

   public :

	/* Default constructor. */
	SnMyNode ();

	/* Destructor. */
   ~SnMyNode ();

	/*! Returns the bounding box, can be empty. */
	virtual void get_bounding_box ( GsBox &b ) const override;
};

/*	The method below has to be called before drawing SnMyNode in order to connect SnMyNode
	to its renderer. In this example it is automatically called the first time SnMyNode is
	used, with a call from SnMyNode's constructor. However, if a SnNode is to be used
	independently from its renderer, the connection should be called from another initilization
	function, so that the node does not need to include or be linked with one particular renderer,
	also allowing connections to diferent renderers when/if needed.
	In sig there is a single initializer for all included renderers in the sigogl module. */
void SnMyNodeRegisterRenderer ();
