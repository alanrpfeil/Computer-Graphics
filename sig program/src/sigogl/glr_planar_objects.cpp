/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/sn_planar_objects.h>

# include <sigogl/gl_core.h>
# include <sigogl/gl_context.h>
# include <sigogl/gl_font.h>
# include <sigogl/gl_texture.h>
# include <sigogl/gl_resources.h>
# include <sigogl/glr_planar_objects.h>

//# define GS_USE_TRACE1 // Constructor and Destructor
//# define GS_USE_TRACE2 // Render
# include <sig/gs_trace.h>

//======================================= SrLines ====================================

GlrPlanarObjects::GlrPlanarObjects()
{
	GS_TRACE1("Constructor");
	_esize = 0;
}

GlrPlanarObjects::~GlrPlanarObjects()
{
	GS_TRACE1("Destructor");
}

static const GlProgram* Prog = 0;

void GlrPlanarObjects::init ( SnShape* s )
{
	GS_TRACE2("Generating program objects");
	if (!Prog) Prog = GlResources::get_program("2dtextured");
	_glo.gen_vertex_arrays(1);
	_glo.gen_buffers(3);
}

void GlrPlanarObjects::render ( SnShape* s, GlContext* c )
{
	GS_TRACE2("GL4 Render " << s->instance_name());
	SnPlanarObjects& o = *((SnPlanarObjects*)s);

	// 2. Set buffer data if node has been changed:
	if ( s->changed()&SnShape::Changed ) // flags are: Unchanged, RenderModeChanged, MaterialChanged, ResolutionChanged, Changed
	{	_esize = o.I.size() / 3;
		glBindVertexArray(_glo.va[0]);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, _glo.buf[0]);
		glBufferData(GL_ARRAY_BUFFER, o.P.sizeofarray(), o.P.pt(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, _glo.buf[1]);
		glBufferData(GL_ARRAY_BUFFER, o.C.sizeofarray(), o.C.pt(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0);
		if (o.T.size()) // has texture coordinates
		{	glEnableVertexAttribArray(2);
			glBindBuffer(GL_ARRAY_BUFFER, _glo.buf[2]);
			glBufferData(GL_ARRAY_BUFFER, o.T.sizeofarray(), o.T.pt(), GL_STATIC_DRAW);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
		}
	}

	if (!_esize) return; // nothing to do

	// 3. Enable/bind needed elements and draw:
	c->use_program(Prog);
	glActiveTexture(GL_TEXTURE0 + 0); // Only using texture unit 0
	glUniformMatrix4fv(Prog->uniloc[0], 1, GLTRANSPMAT, c->projection()->e);
	glUniformMatrix4fv(Prog->uniloc[1], 1, GLTRANSPMAT, c->modelview()->e);
	glUniform1f(Prog->uniloc[2], o.zcoordinate);
	glBindVertexArray(_glo.va[0]);
    GsArray<SnPlanarObjects::Group>& G=o.G;
	const int gsize = G.size();
	int* indices = o.I.pt();
	if ( !gsize )
	{	glUniform1i ( Prog->uniloc[3], SnPlanarObjects::Colored ); // Mode
		glDrawElements ( GL_TRIANGLES, o.I.size(), GL_UNSIGNED_INT, indices );
	}
	else
	{	G.push().ind=o.I.size();
		glUniform1i ( Prog->uniloc[4], 0 ); // Tell to use sampler for texture unit 0
		for ( int i=0; i<gsize; i++ )
		{	glUniform1i ( Prog->uniloc[3], o.G[i].type ); // Mode
			if ( o.G[i].type!=SnPlanarObjects::Colored )
			{	glBindTexture ( GL_TEXTURE_2D, o.G[i].texid ); // Bind image
			}
			const int numi = G[i+1].ind-G[i].ind;
			glDrawElements ( GL_TRIANGLES, numi, GL_UNSIGNED_INT, indices );
			indices += numi;
		}
		G.pop();
	}
	glBindVertexArray(0); // break the existing vertex array object binding.
}

//================================ EOF ======================================
