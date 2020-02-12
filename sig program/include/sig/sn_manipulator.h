/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef SN_MANIPULATOR_H
# define SN_MANIPULATOR_H

/** \file sn_manipulator.h 
 * scene manipulator
 */

# include <sig/gs_quat.h>
# include <sig/gs_plane.h>
# include <sig/sn_editor.h>
# include <sig/sn_lines.h>

//==================================== SnManipulator ====================================

/*! \class SnManipulator sn_manipulator.h
	\brief scene manipulator

	SnManipulator puts a bounding box around the child object and lets
	the user manipulate the box, updating the transformation matrix
	accordingly. */
class SnManipulator : public SnEditor
{  public :
	enum Mode { ModeIdle, ModeTranslating, ModeRotating };
	static const char* class_name; //<! Contains string SnManipulator

   private :
	Mode _mode;
	GsBox _box;
	GsVec _gr;
	SnLines* _snbox;
	SnLines* _dragsel;
	GsPnt _firstp;		// 1st pt selected (center of the blue cross)
	int _corner;
	GsPnt _bside[4];
	GsPlane _plane;
	GsMat _initmat;
	GsQuat _rotation;   // local rot after _initmat
	GsVec _translation; // local transl after _initmat
	GsPnt _center;
	float _radius;
	float _precision_in_pixels;
	void (*_usercb) ( SnManipulator*, const GsEvent&, void* );
	void* _userdata;
	gscbool  _translationray;
	gscbool  _drawbox;

   protected :
	/*! Destructor only accessible through unref() */
	virtual ~SnManipulator ();
	GsBox& box() { return _box; }

   public :
	/*! Constructor */
	SnManipulator ();

	/*! Defines the child node */
	void child ( SnNode* sn );

	/*! Retrieves the child node */
	SnNode* child () const { return SnEditor::child(); }

	/*! Template version of child() with typecast to given type */
	template <class Node> Node* child () { return (Node*)SnEditor::child(); }

	/*! Get a reference to the manipulator matrix. */
	GsMat& mat () { return SnEditor::mat(); }

	/*! Set a initial matrix to the manipulator. This matrix will be saved
		and all manipulations performed will be combined to it. 
		All other transformations are set to identity.
		Note: the final transformation is obtained with mat() */
	void initial_mat ( const GsMat& m );

	/*! Put into waiting mode, ie, no selections appearing */
	void init ();

	/*! To be called to update the manipulation box */
	void update ();

	/*! Makes the manipulator box to grow by given amounts per axis. */
	void grow_box ( float dx, float dy, float dz );

	/*! Set if the manipulator box is to be rendered or not */
	void draw_box ( bool b );

	/*! Returns if the manipulator box is rendered or not */
	bool draw_box () const { return _drawbox!=0; }

	/*! Set a user callback that is called each time an event is processed
		by the manipulator */
	void callback ( void(*cb)(SnManipulator*,const GsEvent&,void*), void* udata )
		{ _usercb=cb; _userdata=udata; }

	/*! Returns the associated callback data */
	void* userdata () const { return _userdata; }

	/*! Set the translation DOFs */
	void translation ( const GsVec& t );

	/*! Returns the translation DOFs.
		Note: if initial_mat is set, it is not considered here. */
	GsVec translation () const { return _translation; }

	/*!set rotation DOFs*/
	void rotation ( const GsQuat& q );

	/*! Returns the Quat.
		Note: if initial_mat is set, it is not considered here. */
	GsQuat rotation () const { return _rotation; }

	/*! Handles mouse drag events, and few keys:
		qawsed = rotation around xyz axis (shift/ctrl/alt change the step)
		esc: set the manipulator transformation to be identity
		x: switch to/from the "translation ray" mode
		p: print the current global matrix of the manipulator
		(ps: this help is available via the FlViewer help)
		(make sure your gui has focus to dispach key events to the manipulator) */
	virtual int handle_event ( const GsEvent &e, float t );

	/*! checks if event can be handled */
	int check_event ( const GsEvent& e, float& t );
  
	/* will check and update the size of the bounding box if needed */
	virtual void post_child_render () override;

   protected :
	void _transform ( const GsPnt& p, const GsVec& r, char type );
	void _set_drag_selection ( const GsPnt& p );
};

//================================ End of File =================================================

# endif // SN_MANIPULATOR_H

