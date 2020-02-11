 /*=======================================================================
	Copyright (c) 2018-2019 Marcelo Kallmann.
	This software is distributed under the Apache License, Version 2.0.
	All copies must contain the full copyright notice licence.txt located
	at the base folder of the distribution. 
  =======================================================================*/

# pragma once

# include <sig/gs_vec.h>
# include <sig/sn_shape.h>

class GsPolygon; // forward declaration
class GsFontStyle; // forward declaration
class UiLabel; // forward declaration

class SnCircle : public SnShape
{  public :
	GsVec center;
	GsVec normal;
	float radius;
	float linewidth;
	gsuint nvertices;

   public :
	static const char* class_name; //<! Contains string SnLines2
	SN_SHAPE_RENDERER_DECLARATIONS;

   public :

	/* Default constructor. */
	SnCircle ();

	/* Destructor. */
   ~SnCircle ();

	/*! Returns the bounding box, can be empty. */
	virtual void get_bounding_box ( GsBox &b ) const override;
};

/*	The method below has to be called before drawing SnCircle in order to connect SnCircle
	to its renderer. In this example it is automatically called the first time SnCircle is
	used, with a call from SnCircle's constructor. However, if a SnNode is to be used
	independently from its renderer, the connection should be called from another initilization
	function, so that the node does not need to include or be linked with one particular renderer,
	also allowing connections to diferent renderers when/if needed.
	In sig there is a single initializer for all included renderers in the sigogl module. */
void SnCircleRegisterRenderer ();
