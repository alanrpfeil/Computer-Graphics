/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef SN_TEXT_H
# define SN_TEXT_H

/** \file sn_text.h 
 * node to display text at 3D locations
 */

# include <sig/gs_font.h>
# include <sig/sn_planar_objects.h>

class SnPlanarObjects;

/*! \class SnText sn_text.h
	\brief node to display text at 3D locations
	SnText draws text from strings automatically definining textured objects
	to display the text at the given locations. It derives SnPlanarObjects
	in order to build textured quads for the text at display time.
	ImprNote: A more generic SnGlyph class could be implemented based on SnText. */
class SnText : public SnPlanarObjects
{ public :
	struct Entry
	{	GsFontStyle fs;
		GsColor color;
		GsColor* e3c; // optional extra 3 colors
		GsCharPt text;
		GsPnt2 position;
		float xmax;
		Entry () { e3c=0; color=GsColor::black; xmax=0; }
	   ~Entry () { delete[] e3c; }
	};

	static const char* class_name; //<! Contains string SnText
	SN_SHAPE_RENDERER_DECLARATIONS;

  private :
	GsArrayPt<Entry> _entries;
	float _scaling;
	Entry& cur () { if(empty()) add_entry(); return *_entries.top(); }

  public :
	/* Default Constructor. */
	SnText ();

	/* Destructor */
   ~SnText ();

	/*! Set the sizes of internal arrays and number of entries to zero and calls touch(). */
	void init ();

	/*! Returns the number of text entries currently defined. */
	int entries () const { return _entries.size(); }

	/*! Returns the entry of index i, which must be already defined. */
	const Entry* entry ( int i ) const { return _entries[i]; }

	/*! Set SnText ready to receive a new text entry. */
	void add_entry ();

	/*! Returns true if P array is empty; false otherwise. */
	bool empty () const { return _entries.empty(); }

	/*! Compress internal arrays. */
	void compress ();

	/*! Set font style */
	void font_style ( const GsFontStyle& fs );

	/*! Access font style for changing parameters, touch() is automatically called. */
	GsFontStyle& font_style ();

	/*! Set the z coordinate. */
	void zcoordinate ( float z );

	/*! Returns the z coordinate. */
	float zcoordinate () const { return SnPlanarObjects::zcoordinate; }

	/*! Set a scaling factor to be multiplied to the coordinates of the textured text quads. */
	void scaling ( float s );

	/*! Returns the coordinate scaling factor. */
	float scaling () const { return _scaling; }

	/*! Set text position in windows coordinates */
	void pos ( float x, float y );
	void pos ( const GsVec2& p ) { pos(p.x,p.y); }
	void pos ( const GsVec& p ) { pos(p.x,p.y); zcoordinate(p.z); }
	void pos ( float x, float y, float z ) { pos(x,y); zcoordinate(z); }

	/*! Set text to display. */
	void set ( const char* st );
	void append ( const char* st );
	void set ( float x, float y, const char* st ) { pos(x,y); set(st); }
	void set ( const GsVec2& p, const char* st ) { pos(p); set(st); }
	void set ( const GsVec& p, const char* st ) { pos(p); set(st); }

	/*! Set text color of current entry. */
	void color ( GsColor c );

	/*! Set the maximum x limit, if value>0. */
	void xmax ( float xmax );

	/*! Set four colors (top-left, top-right, bottom-left, bottom-right) per character. */
	void multicolor ( GsColor c1, GsColor c2, GsColor c3, GsColor c4 );

	/*! Access const text of any entry, if default -1 index is given, the top entry is returned.
		The returned pointer may be null if no string has been defined */
	const char* ctext ( int i=-1 );
};

//================================ End of File =================================================

# endif // SN_TEXT_H
