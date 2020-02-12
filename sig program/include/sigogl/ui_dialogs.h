/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

/** \file ui_dialog.h
 * Provides common dialog boxes.
 */

# ifndef UI_DIALOGS_H
# define UI_DIALOGS_H

# include <sig/gs.h>

class GsColor;
class GsStrings;
class WsWindow;
template <typename X> class GsArray;

//================================ dialog functions =================================================

/*! By default dialogs are built in a OS-allocated independent window. This function will set 
    dialogs to appear as panels inside an existing window w. Calling it with a null w returns
    to default behavior. */
void ui_set_dialog_parent_window ( WsWindow* w );

/*! Set a default label to be used in the window title bar of OS dialogs. If set to 0 (default),
	the label defined by WsDialog is used. Pointer title must be to an always-valid static string. */
void ui_set_dialog_default_title ( const char* title );

/*! Opens a color selection dialog. Returns true if the ok button was pressed and false
	otherwise. If the ok button was pressed the selected color is returned in selection. */
bool ui_color ( const char* msg, GsColor& selection, const char* wintitle=0 );

/*! Opens a dialog with selection buttons for each entry in parameter choices. 
	Returns the index of the chosen selection, or -1 if the cancel button was called.
	The label of the cancel button will be replaced by butlabel if not null. */
int ui_choice ( const char* msg, const GsStrings& choices, const char* butlabel=0, const char* wintitle=0 );

/*! Opens a dialog with ok and cancel buttons, and showing the given message.
	It will return true if ok is pressed and false otherwise. */
bool ui_confirm ( const char* msg, const char* wintitle=0 );

/*! Opens a dialog with yes and no buttons, and showing the given message.
	It will return true if yes is pressed and false otherwise. */
bool ui_ask ( const char* msg, const char* wintitle=0 );

/*! Opens a message dialog and returns after the user press the ok button */
void ui_message ( const char* msg, const char* wintitle=0 );

/*! Opens a text input dialog with ok and cancel buttons, and showing the given message.
	It will return the string if ok is pressed and null otherwise. 
	The returned string is an internal static string which may be reallocated when ui_input_dialog() is called.
	If parameter input is left as null, the previously entered input is displayed to be edited. */
const char* ui_input ( const char* msg, const char* input=0, const char* wintitle=0 );

/*! Opens a file browser. File filter format follows this example: "*.txt;*.log".
	It returns a string if ok is pressed and null otherwise. 
	The returned string points to an internal buffer with global lifetime.
    If pointer multif is non-null, multiple files may be selected and will be returned in multif.
	In that case, the returned string is the common folder (without ending '\'), and strings in
	multif contain only filenames, which are stored (concatenated) in the same internal buffer.
    If a single file is selected its full description is returned and multif will be of size 0. */
const char* ui_select_file ( const char* msg, const char* fname=0, const char* filter=0, GsArray<const char*>* multif=0 );

/*! Opens a file input dialog which allows entering a file name that does not exist.
	The optional parameters work in the same way as in ui_open_file() */
const char* ui_input_file ( const char* msg, const char* fname=0, const char* filter=0 );

/*! Convert multiple files in multif as returned by ui_file_chooser() to one full-path file per string in fullfiels */
void ui_get_full_names ( const char* dir, const GsArray<const char*>& multif, GsStrings& fullfiles );

/*! Opens a dialog for selection of folders */
const char* ui_select_folder ( const char* msg, const char* folder=0 );

//================================ End of File =================================================

# endif // UI_DIALOGS_H
