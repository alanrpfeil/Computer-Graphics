/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef UI_FILE_BROWSER_H
# define UI_FILE_BROWSER_H

/** \file ui_file_browser.h 
 * simple file browser
 */

# include <sigogl/ui_panel.h>
# include <sigogl/ui_input.h>
# include <sigogl/ui_output.h>

//================================ UiFileChooser ====================================

/*! \class UiFileChooser sn_sphere.h
	\brief a file browser
*/
class UiFileChooser : public UiPanel
 { protected:
	UiInput* _inp;  // for filename input
	UiOutput* _out; // for display of files in current directory
	GsString _curext;

   public :

	/*! Constructs as a sphere centered at (0,0,0) with radius 1 */
	UiFileChooser ( const char* fname, int ev, int x=0, int y=0, int mw=220, int mh=120 );

	const char* value () const { return _inp->value(); }

	const GsString& curext () const { return _curext; }
	void update_file_list ( const char* fstring );

   public :
	virtual int handle ( const GsEvent& e, UiManager* uim ) override;
};


//================================ End of File =================================================

# endif // UI_FILE_BROWSER_H
