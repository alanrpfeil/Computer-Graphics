/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

/** \file ws_dialog.h
 * Dialog box window 
 */

# ifndef WS_DIALOG_H
# define WS_DIALOG_H 

# include <sigogl/ws_window.h>

class WsWindow;

/*! WsDialog implements a simple window that can be used as a dialog box. */
class WsDialog : public WsWindow
{  public :
	/*! Constructor defines the window label as "Dialog" */
	WsDialog ();
   protected :
	virtual void init ( GlContext* c, int w, int h ) override;
	virtual void draw ( GlRenderer* r ) override;
};

//================================ End of File ======================================

# endif //  WS_DIALOG_H
