/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_IMAGE_H
# define GS_IMAGE_H

/** \file gs_image.h 
 * Classes for manipulating images.*/

# include <sig/gs_color.h>
# include <sig/gs_shareable.h>

//=========================== GsImageBase ============================

/*! \class GsImageBase gs_image.h
	\brief Base class for a non-compressed image storage class */
class GsImageBase : public GsShareable
 { protected :
	int _w, _h; // total image size is _w*_h*sizeof bytes
	void* _img; // image data

   protected :
	/*! Constructor for an empty image */
	GsImageBase ();

	/*! Destructor */
   ~GsImageBase ();

	/*! Constructor for an image of the optional given size */
	GsImageBase ( int w, int h, unsigned sizeofx );

	/*! Alloc the desired size in pixels of the image.
		A total of w*h pixels (ie w*h*4 bytes) are allocated.
		Invalid dimensions deletes the image data */
	void init ( int w, int h, unsigned sizeofx );

	/*! Changes the image by appying a vertical mirroring */
	void vertical_mirror ( unsigned sizeofx );

	/*! Saves the image in png, tga or bmp format according to the file extension.
		Returns true in case of success, false otherwise. */
	bool save ( const char* filename, unsigned sizeofx );

	/*! Load a png or bmp image. Returns true in case of success, false otherwise. */
	bool load ( const char* filename, unsigned sizeofx );

	bool load_from_memory ( const gsbyte* buffer, unsigned buflen, unsigned sizeofx );

   public :

	/*! Returns the width in pixels of the image */
	int w () const { return _w; }

	/*! Returns the height in pixels of the image */
	int h () const { return _h; }
 
	/*! Returns the number of bytes in the image, equal to w()*h() */
	int size () const { return _w*_h; }

	/*! Returns the buffer index of the pixel */
	int lcindex ( int l, int c ) const { return l*_w+c; }

	/*! Returns the buffer index in planar xy coordinates */
	int xyindex ( int x, int y ) const { return y*_w+x; }
};

//=========================== GsImage ============================

/*! \class GsImage gs_image.h
	\brief Non compressed 32 bit depth rgba image
	GsImage stores pixel data as a sequence of rgba pixels. */
class GsImage : public GsImageBase
 {  public :
	/*! Constructor for an empty image */
	GsImage () {}

	/*! Constructor for an image of the optional given size */
	GsImage ( int w, int h ) : GsImageBase(w,h,sizeof(GsColor)) {}

	/*! Destructor */
   ~GsImage () {}

	/*! Alloc the desired size in pixels of the image.
		A total of w*h pixels (ie w*h*4 bytes) are allocated.
		Invalid dimensions deletes the image data */
	void init ( int w, int h ) { GsImageBase::init(w,h,sizeof(GsColor)); }

	/*! Changes the image by appying a vertical mirroring */
	void vertical_mirror () { GsImageBase::vertical_mirror(sizeof(GsColor)); }

	/*! Returns a reference to the color of pixel at position (l,c) */
	GsColor& operator() ( int l, int c ) { return pixel(l,c); }

	/*! Returns a reference to the color of pixel at position (l,c) */
	GsColor& pixel ( int l, int c ) { return ((GsColor*)_img)[l*_w+c]; }

	/*! Returns a const reference to the color of pixel at position (l,c) */
	const GsColor& cpixel ( int l, int c ) const { return ((GsColor*)_img)[l*_w+c]; }

	/*! Returns a pointer to the pixel color (4 bytes) at position (l,c) */
	GsColor* ptpixel ( int l, int c ) { return ((GsColor*)_img)+(l*_w+c); }

	/*! Access a pixel in planar xy coordinates (the opposite of line,column) */
	GsColor& xypixel ( int x, int y ) { return ((GsColor*)_img)[y*_w+x]; }

	/*! Const access to a pixel in planar xy coordinates (the opposite of line,column) */
	const GsColor& cxypixel ( int x, int y ) const { return ((GsColor*)_img)[y*_w+x]; }

	/*! Returns a reference to the red component of the pixel at position (l,c) */
	gsbyte& r ( int l, int c ) { return pixel(l,c).r; }

	/*! Returns a reference to the green component of the pixel at position (l,c) */
	gsbyte& g ( int l, int c ) { return pixel(l,c).g; }

	/*! Returns a reference to the blue component of the pixel at position (l,c) */
	gsbyte& b ( int l, int c ) { return pixel(l,c).b; }
 
	/*! Returns a reference to the alpha component of the pixel at position (l,c) */
	gsbyte& a ( int l, int c ) { return pixel(l,c).a; }

	/*! Returns the base pointer of the line l of the image */
	GsColor* line ( int l ) { return ((GsColor*)_img)+(l*_w); }

	/*! Returns the base pointer of the image data */
	GsColor* data () { return ((GsColor*)_img); }

	/*! Returns the base pointer of the image data as a const pointer */
	const GsColor* cdata () const { return ((GsColor*)_img); }

	/*! Saves the image in png, tga or bmp format according to the file extension.
		Returns true in case of success, false otherwise. */
	bool save ( const char* filename ) { return GsImageBase::save(filename,sizeof(GsColor)); }

	/*! Load a png or bmp image. Returns true in case of success, false otherwise. */
	bool load ( const char* filename ) { return GsImageBase::load(filename,sizeof(GsColor)); }
};

//=========================== GsBytemap ============================

/*! \class GsBytemap gs_image.h
	\brief Non compressed 8 bit depth grayscale image */
class GsBytemap : public GsImageBase
 {  public :
	/*! Constructor for an empty image */
	GsBytemap () {}

	/*! Constructor for an image of the optional given size */
	GsBytemap ( int w, int h ) : GsImageBase(w,h,sizeof(gsbyte)) {}

	/*! Destructor */
   ~GsBytemap () {}

	/*! Alloc the desired size in pixels of the image.
		A total of w*h pixels (ie w*h*4 bytes) are allocated.
		Invalid dimensions deletes the image data */
	void init ( int w, int h ) { GsImageBase::init(w,h,sizeof(gsbyte)); }

	/*! Changes the image by appying a vertical mirroring */
	void vertical_mirror () { GsImageBase::vertical_mirror(sizeof(gsbyte)); }

	/*! Returns the width in pixels of the image */
	int w () const { return _w; }

	/*! Returns the height in pixels of the image */
	int h () const { return _h; }
 
	/*! Returns the number of bytes in the image, equal to w()*h() */
	int size () const { return _w*_h; }

	/*! Returns a reference to the value of pixel at position (l,c) */
	gsbyte& operator() ( int l, int c ) { return pixel(l,c); }

	/*! Returns a reference to the value at position i in the internal data array */
	gsbyte& operator[] ( int i ) { return ((gsbyte*)_img)[i]; }

	/*! Returns a reference to the color of pixel at position (l,c) */
	gsbyte& pixel ( int l, int c ) { return ((gsbyte*)_img)[l*_w+c]; }

	/*! Returns a const reference to the color of pixel at position (l,c) */
	gsbyte cpixel ( int l, int c ) const { return ((gsbyte*)_img)[l*_w+c]; }

	/*! Returns the base pointer of the line l of the image */
	gsbyte* line ( int l ) { return ((gsbyte*)_img)+(l*_w); }

	/*! Returns the base pointer of the image data */
	gsbyte* data () { return (gsbyte*)_img; }

	/*! Returns the base pointer of the image data as a const pointer */
	const gsbyte* cdata () const { return (gsbyte*)_img; }

	/*! Saves the image in png, tga or bmp format according to the file extension.
		Returns true in case of success, false otherwise. */
	bool save ( const char* filename ) { return GsImageBase::save(filename,sizeof(gsbyte)); }

	/*! Load an image in 8 bits per pixel format. Returns true in case of success, false otherwise. */
	bool load ( const char* filename ) { return GsImageBase::load(filename,sizeof(gsbyte)); }

	bool load_from_memory ( const gsbyte* buffer, unsigned buflen )
	 { return GsImageBase::load_from_memory(buffer,buflen,sizeof(gsbyte)); }

	/*! Converts bytemap to 24-bit img, placing values in given component: 0:r, 1:g, 2:b, 3:a */
	void convert_to_image ( GsImage& img, int component=0 ) const
	 { img.init(w(),h()); for(int i=0;i<size();i++) img.data()[i].e[component]=cdata()[i]; }
};

//============================= end of file ==========================

# endif // GS_IMAGE_H
