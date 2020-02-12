/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_dirs.h>
# include <sig/gs_image.h>

# include <sigogl/gl_core.h>
# include <sigogl/gl_texture.h>
# include <sigogl/gl_context.h>
# include <sigogl/gl_resources.h>

# include <sig/sn_model.h>
# include <sigogl/glr_model.h>

//# define GS_USE_TRACE1 // Constructor and Destructor
//# define GS_USE_TRACE2 // Render
//# define GS_USE_TRACE3 // Model stats
//# define GS_USE_TRACE4 // Render messages
# include <sig/gs_trace.h>

//======================================= GlrModel ====================================

GlrModel::GlrModel ()
{
	GS_TRACE1 ( "Constructor" );
	_normalspervertex = false;
}

GlrModel::~GlrModel ()
{
	GS_TRACE1 ( "Destructor" );
}

static const GlProgram* pFlat=0;
static const GlProgram* pGour=0;
static const GlProgram* pPhong=0;
static const GlProgram* pText=0;
static const GlProgram* pPhongMC=0;
static const GlProgram* pColored=0;

void GlrModel::init ( SnShape* s )
{
	GS_TRACE2 ( "Generating program objects" );
	if ( !pFlat )
	{	pFlat = GlResources::get_program("3dflat");
		pGour = GlResources::get_program("3dgouraud");
		pText = GlResources::get_program("3dtextured");
		pPhong = GlResources::get_program("3dphong");
		// pPhongMC and pColored are not as used and are later loaded only when/if needed 
	}
	_glo.gen_vertex_arrays ( 1 );
	_glo.gen_buffers ( 3 ); // it will need 2 or 3 buffers
}

static inline void _add_texture ( GsModel::Group& G, const GsModel& m )
{	
	G.dmap->id = GlResources::declare_texture(G.dmap->fname,G.dmap->fname);
	GsString path = m.filename;
	if ( path.len() )
	{	remove_filename ( path );
		GlResources::dirs().push(path);
	}
}

void GlrModel::render (  SnShape* s, GlContext* c )
{
	const GsModel& m = *((const SnModel*)s)->cmodel();

	GS_TRACE2 ( "Start rendering "<<s->instance_name()<<" ["<<m.name<<"]" );

	GS_TRACE3 ( "Faces     : "<<m.F.size() );
	GS_TRACE3 ( "Normals   : "<<m.N.size() );
	GS_TRACE3 ( "Vertices  : "<<m.V.size() );
	GS_TRACE3 ( "TexCoords : "<<m.T.size() );
	GS_TRACE3 ( "Materials : "<<m.M.size() );
	GS_TRACE3 ( "Groups    : "<<m.G.size() );

	if ( m.empty() ) return;

	const GlProgram* p=pGour;

	c->cull_face ( m.culling==1 ); // every node has to set its desired back face culling state

	// 1. Set programs based on rendering mode
	gsRenderMode rm = s->render_mode();
	switch ( rm )
	{	case gsRenderModeDefault: p=pGour; c->polygon_mode_fill(); GS_TRACE4("Prog: Gouraud"); break;
		case gsRenderModePhong  : p=pPhong; c->polygon_mode_fill(); GS_TRACE4("Prog: Phong"); break;
		case gsRenderModeGouraud: p=pGour; c->polygon_mode_fill(); GS_TRACE4("Prog: Gouraud"); break;
		case gsRenderModeFlat	: p=pFlat; c->polygon_mode_fill(); GS_TRACE4("Prog: Flat"); break;
		case gsRenderModeLines  : p=pGour; c->polygon_mode_line(); GS_TRACE4("Prog: Gouraud"); break;
		case gsRenderModePoints : p=pFlat; c->polygon_mode_point(); GS_TRACE4("Prog: Flat"); break;
	}

	gscbool textured = m.textured;
	GsModel::MtlMode mtlmode = m.mtlmode();
	if ( s->material_is_overriden() ) { textured=0; mtlmode=GsModel::NoMtl; } // SgDev: color override not completed/tested

	// Textured objects have to be defined using the group structure
	// SnColorSurf will use 2 possible modes: Smooth-PerVertexMtl or Faces-PerVertexColor
	if ( textured )
	{	GS_TRACE4 ( "Textured..." );
		p=pText;
	}
	else if ( mtlmode==GsModel::PerVertexMtl || mtlmode==GsModel::PerFaceMtl )
	{	GS_TRACE4 ( "MtlMode: PerVertexMtl or PerFaceMtl..." );
		if ( !pPhongMC ) pPhongMC = GlResources::get_program("3dphongmc");
		p = pPhongMC;
	}
	else if ( mtlmode==GsModel::PerVertexColor )
	{	GS_TRACE4 ( "MtlMode: PerVertexColor..." );
		if ( !pColored ) pColored = GlResources::get_program("3dsmooth");
		p = pColored;
	}
	else
	{	GS_TRACE4 ( "MtlMode: NoMtl or PerGroupMtl..." );
	}

	// 2. Set buffer data if node has been changed (flags are: Unchanged, RenderModeChanged, MaterialChanged, Changed)
	if ( s->changed()&SnShape::Changed )
	{	glBindVertexArray ( _glo.va[0] );

		if ( p==pColored ) // colors per vertex, no illumination, only declare vertices
		{	GS_TRACE4 ( "Defining V buffer..." );
			_normalspervertex = false;
			glEnableVertexAttribArray ( 0 );
			glBindBuffer ( GL_ARRAY_BUFFER, _glo.buf[0] );
			glBufferData ( GL_ARRAY_BUFFER, m.V.sizeofarray(), m.V.pt(), GL_STATIC_DRAW );
			glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
		}
		else if ( m.geomode()==GsModel::Smooth && p!=pFlat ) // normals per vertex, or no normals smooth mode
		{	GS_TRACE4 ( "Defining V,N buffers per vertex..." );
			_normalspervertex = true;
			// Vertices:
			glEnableVertexAttribArray ( 0 );
			glBindBuffer ( GL_ARRAY_BUFFER, _glo.buf[0] );
			glBufferData ( GL_ARRAY_BUFFER, m.V.sizeofarray(), m.V.pt(), GL_STATIC_DRAW );
			glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
			// Normals:
			glEnableVertexAttribArray ( 1 );
			glBindBuffer ( GL_ARRAY_BUFFER, _glo.buf[1] );
			glBufferData ( GL_ARRAY_BUFFER, m.N.sizeofarray(), m.N.pt(), GL_STATIC_DRAW );
			glVertexAttribPointer ( 1, 3, GL_FLOAT, GL_FALSE, 0, 0 ); // false means no normalization
			// Tx coordinates:
			if ( textured )
			{	GS_TRACE4 ( "Including texture coordinates..." );
				glEnableVertexAttribArray ( 2 );
				glBindBuffer ( GL_ARRAY_BUFFER, _glo.buf[2] );
				glBufferData ( GL_ARRAY_BUFFER, m.T.sizeofarray(), m.T.pt(), GL_STATIC_DRAW );
				glVertexAttribPointer ( 2, 2, GL_FLOAT, GL_FALSE, 0, 0 ); // false means no normalization
			}
		}
		else
		{	GS_TRACE4 ( "Defining V,N buffers per face..." );
			_normalspervertex = false;
			GsArray<GsVec> va;
			unsigned bufsize = m.F.size()*9*sizeof(float);
			// Vertices:
			m.get_vertices_per_face ( va );
			glEnableVertexAttribArray ( 0 );
			glBindBuffer ( GL_ARRAY_BUFFER, _glo.buf[0] );
			glBufferData ( GL_ARRAY_BUFFER, bufsize, va.pt(), GL_STATIC_DRAW );
			glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
			// Normals:
			if ( p==pFlat )
			{	GS_TRACE4 ( "Computing flat normals..." );
				m.get_flat_normals_per_face(va,3);
			}
			else
			{	GS_TRACE4 ( "Retrieving normals..." );
				m.get_normals_per_face(va);
			}
			glEnableVertexAttribArray ( 1 );
			glBindBuffer ( GL_ARRAY_BUFFER, _glo.buf[1] );
			glBufferData ( GL_ARRAY_BUFFER, bufsize, va.pt(), GL_STATIC_DRAW );
			glVertexAttribPointer ( 1, 3, GL_FLOAT, GL_FALSE, 0, 0 ); // false means no normalization
			// Tx coordinates:
			if ( textured )
			{	GS_TRACE4 ( "Including texture coordinates..." );
				GsArray<GsVec2> tca;
				m.get_texcoords_per_face ( tca );
				glEnableVertexAttribArray ( 2 );
				glBindBuffer ( GL_ARRAY_BUFFER, _glo.buf[2] );
				glBufferData ( GL_ARRAY_BUFFER, tca.sizeofarray(), tca.pt(), GL_STATIC_DRAW );
				glVertexAttribPointer ( 2, 2, GL_FLOAT, GL_FALSE, 0, 0 ); // false means no normalization
			}
		}

		if ( mtlmode>=GsModel::PerVertexMtl ) // define color buffer
		{	GsArray<GsColor> C;
			if ( mtlmode==GsModel::PerFaceMtl )
			{	GS_TRACE4 ( "Defining C per face buffer..." );
				C.capacity ( m.M.size()*3 );
				for ( int i=0, s=m.M.size(); i<s; i++ ) { C.push()=m.M[i].diffuse; C.push_top(); C.push_top(); }
			}
			else
			{	GS_TRACE4 ( "Defining C per vertex buffer..." );
				C.size ( m.V.size() );
				for ( int i=0, s=C.size(); i<s; i++ ) C[i]=m.M[i].diffuse;
			}
			gsuint bufid = m.mtlmode()==GsModel::PerVertexColor? 1:2;
			GS_TRACE4 ( "Defining bufferid "<<bufid );
			glEnableVertexAttribArray ( bufid );
			glBindBuffer ( GL_ARRAY_BUFFER, _glo.buf[bufid] );
			glBufferData ( GL_ARRAY_BUFFER, C.sizeofarray(), C.pt(), GL_STATIC_DRAW );
			glVertexAttribPointer ( bufid, 4, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0 );
		}
	}

	// 3. Enable/bind needed elements and draw:
	c->use_program ( p->id );
	glBindVertexArray ( _glo.va[0] );

	float buf[36]; // LightDev: 36 is needed to encode 4 lights, otherwise buf[12] is enought
	glUniformMatrix4fv ( p->uniloc[0], 1, GLTRANSPMAT, c->projection()->e );
	glUniformMatrix4fv ( p->uniloc[1], 1, GLTRANSPMAT, c->modelview()->e );

	// LightDev: here is where light information is passed to the shaders:
	// Sending 4x full set of parameters could be optimized by better matching GsLight with the uniforms
	int NumL=c->num_lights();
	GsLight* lights = c->light;
	
	# define DEFINE_LIGHT0(L)		glUniform3fv ( p->uniloc[2], 1, L[0].position.e ); \
									glUniformMatrix3fv ( p->uniloc[3], 1, GL_FALSE, L[0].encode_intensities(buf) )
	# define DEFINE_NLIGHTS(L)		encode_positions(L,NumL,buf); glUniform3fv ( p->uniloc[2], NumL, buf ); \
									encode_intensities(L,NumL,buf); glUniformMatrix3fv ( p->uniloc[3], 1, GL_FALSE, buf )
	# define DEFINE_LIGHT(L)		if ( NumL==1 ) { DEFINE_LIGHT0(L); } else { DEFINE_NLIGHTS(L); } \
									glUniform1i ( p->uniloc[4], NumL );

	# define DEFINE_MATERIAL(M)		glUniform3fv ( p->uniloc[5], 4, M.encode_colors(buf) ); \
									glUniform1fv ( p->uniloc[6], 2, M.encode_params(buf) )

	# define DRAW_GROUP_ELEMENTS(G) glDrawElements ( GL_TRIANGLES, G.fn*3, GL_UNSIGNED_INT, &m.F[G.fi].a )
	# define DRAW_GROUP_ARRAYS(G)	glDrawArrays ( GL_TRIANGLES, G.fi*3, G.fn*3 )
	# define DRAW_GROUP(G,npv) 		if (npv) DRAW_GROUP_ELEMENTS(G); else DRAW_GROUP_ARRAYS(G)

	if ( mtlmode==GsModel::NoMtl )
	{	DEFINE_LIGHT ( lights );
		DEFINE_MATERIAL ( s->material() );
		if ( _normalspervertex )
		{	GS_TRACE4 ( "Drawing per-vertex smooth, default material" );
			glDrawElements ( GL_TRIANGLES, m.F.size()*3, GL_UNSIGNED_INT, m.F.pt() );
		}
		else 
		{	GS_TRACE4 ( "Drawing per-face shading, default material" );
			glDrawArrays ( GL_TRIANGLES, 0, m.F.size()*3 );
		}
	}
	else if ( mtlmode==GsModel::PerGroupMtl )
	{	DEFINE_LIGHT ( lights );
		const int gsize = m.G.size();
		if ( textured ) // per-group with textures
		{	GS_TRACE4 ( "Drawing "<<(_normalspervertex?"pre-vertex":"per-face")<<" shading, grouped materials with textures" );
			glUniform1i ( p->uniloc[7], 0 ); // Mode 0 is with texture
			glUniform1i ( p->uniloc[8], 0 ); // Tell to use sampler for texture unit 0
			for ( int g=0; g<gsize; g++ )
			{	GsModel::Group& G=*m.G[g];
				GsMaterial& M=m.M[g];
				if ( G.dmap ) // has diffuse texture
				{	if ( !G.dmap->oglid )
					{	if ( G.dmap->id<0 ) _add_texture(G,m); // need to declare texture
						const GlTexture* t = GlResources::get_texture ( G.dmap->id );
						G.dmap->oglid = t->id;
					}
					glActiveTexture ( GL_TEXTURE0 + 0 );	// Only using texture unit 0
					glBindTexture ( GL_TEXTURE_2D, G.dmap->oglid ); // Bind image
					DEFINE_MATERIAL(M);
					DRAW_GROUP(G,_normalspervertex);
				}
				else
				{	glUniform1i ( p->uniloc[6], 1 ); // set mode to no texture
					DEFINE_MATERIAL(M);
					DRAW_GROUP(G,_normalspervertex);
					glUniform1i ( p->uniloc[6], 0 ); // set mode back to textured
				}
			}
			glBindTexture ( GL_TEXTURE_2D, 0 );
		}
		else if ( _normalspervertex ) // per-group no textures
		{	GS_TRACE4 ( "Drawing per-vertex smooth, grouped materials" );
			for ( int g=0; g<gsize; g++ )
			{	GsModel::Group& G=*m.G[g];
				GsMaterial& M=m.M[g];
				DEFINE_MATERIAL(M);
				DRAW_GROUP_ELEMENTS(G);
			}
		}
		else // per-group no textures
		{	GS_TRACE4 ( "Drawing per-face shading, grouped materials" );
			for ( int g=0; g<gsize; g++ )
			{	GsModel::Group& G=*m.G[g];
				GsMaterial& M=m.M[g];
				DEFINE_MATERIAL(M);
				DRAW_GROUP_ARRAYS(G);
			}
		}
	}
	else if ( m.mtlmode()==GsModel::PerVertexMtl || m.mtlmode()==GsModel::PerFaceMtl )
	{	DEFINE_LIGHT ( lights );
		DEFINE_MATERIAL ( m.M[0] );
		if ( _normalspervertex )
		{	GS_TRACE4 ( "Drawing per-vertex materials, per-vertex normals" );
			glDrawElements ( GL_TRIANGLES, m.F.size()*3, GL_UNSIGNED_INT, m.F.pt() );
		}
		else 
		{	GS_TRACE4 ( "Drawing per-vertex materials, per-face normals" );
			glDrawArrays ( GL_TRIANGLES, 0, m.F.size()*3 );
		}
	}
	else // GsModel::PerVertexColor
	{	GS_TRACE4 ( "Drawing without shading, only per-vertex colors" );
		glDrawElements ( GL_TRIANGLES, m.F.size()*3, GL_UNSIGNED_INT, m.F.pt() );
	}

	glBindVertexArray ( 0 );
	c->polygon_mode_fill();

	GS_TRACE2 ( "End rendering "<<s->instance_name()<<" ["<<m.name<<"]" );
}

/*Notes:
  - MultiDrawArrays() requires indices and is not faster than DrawArrays() multiple times
  - glPolygonMode remains in version 4.5: opengl.org/sdk/docs/man4/html/glPolygonMode.xhtml
*/

//================================ EOF ========================================
