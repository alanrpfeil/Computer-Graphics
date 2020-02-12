/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sigogl/glr_base.h>

# include <sig/sn_model.h>
# include <sigogl/glr_model.h>
static SnShapeRenderer* GlrModelInstantiator () { return new GlrModel; }

# include <sig/sn_lines.h>
# include <sigogl/glr_lines.h>
static SnShapeRenderer* GlrLinesInstantiator () { return new GlrLines; }

# include <sig/sn_lines2.h>
# include <sigogl/glr_lines2.h>
static SnShapeRenderer* GlrLines2Instantiator () { return new GlrLines2; }

# include <sig/sn_planar_objects.h>
# include <sigogl/glr_planar_objects.h>
static SnShapeRenderer* GlrPlanarObjectsInstantiator () { return new GlrPlanarObjects; }

# include <sig/sn_points.h>
# include <sigogl/glr_points.h>
static SnShapeRenderer* GlrPointsInstantiator () { return new GlrPoints; }

# include <sig/sn_text.h>
# include <sigogl/glr_text.h>
static SnShapeRenderer* GlrTextInstantiator () { return new GlrText; }

void GlrBase::init ()
{
	SnModel::renderer_instantiator = &GlrModelInstantiator;
	SnLines::renderer_instantiator = &GlrLinesInstantiator;
	SnLines2::renderer_instantiator = &GlrLines2Instantiator;
	SnPlanarObjects::renderer_instantiator = &GlrPlanarObjectsInstantiator;
	SnPoints::renderer_instantiator = &GlrPointsInstantiator;
	SnText::renderer_instantiator = &GlrTextInstantiator;
}
