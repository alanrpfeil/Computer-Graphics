/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef MAIN_H
# define MAIN_H

# include <sigogl/ui_output.h>
# include <sigogl/ws_window.h>

// Some event enumerators:
enum Cmd {	CmdPanels, CmdElements,
			CmdColorDlg, CmdChoiceDlg, CmdConfirmDlg, CmdAskDlg, CmdMessageDlg, CmdInputDlg, 
						 CmdOpenFileDlg, CmdSaveFileDlg, CmdFolderDlg,
						 CmdWindowDlg, CmdPanelDlg,
			CmdSubtle, CmdLight, CmdGray, CmdDark,
			CmdClear,
			CmdAbout,
			CmdQuit,
			CmdAny };

class MyWindow : public WsWindow
{  private :
	UiPanel* _mainmenu;
	UiOutput* _messagebar;
   public :
	MyWindow ( int x, int y, int w, int h, const char* l );
	void build_ui ();
   public : // derived virtual methods
	void draw ( GlRenderer *r ) override;
	int uievent ( int e ) override;
   public : // method for testing ui elements
	void test_panels ();
	void test_elements ();
};

# endif // MAIN_H
