/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef KN_SCENE_H
# define KN_SCENE_H

//================================ KnScene =================================================

# include <sig/gs_array.h>
# include <sig/sn_group.h>
# include <sig/sn_shape.h>

class SnLines;
class GsModel;
class KnJoint;
class KnSkeleton;
class KnSkinScene;

/*! Maintains a scene graph containing geometries to display a given KnSkeleton */
class KnScene : public SnGroup
{  protected :
	GsArray<SnGroup*> _jgroup;
	float _cradius, _sfactor, _axislen, _avgoffsetlen;
	KnSkeleton* _skeleton;
	KnSkinScene* _skinsc;
	SnGroup* _skelgroup;

   public :
	/*! Constructor. If s is given as non-null s will be connected. */
	KnScene ( KnSkeleton* s=0 );

	/*! Destructor unreferences the associated skeleton */
   ~KnScene ();

	/*! Clears the scene */
	void init ();

   public : //=== virtual methods for functionality extension ===

	/*! Creates a scene graph according to the given skeleton.
		If a skin exists, only the skin is visible at creation time; otherwise,
		the visualization geometry will be set as visible. In all cases method
		set_visibility() can be used to visualize other elements.
		If only skin visualization is need, use lighter KnSkinScene class.
		Null can be passed to clear the scene.
		The skeleton ref()/unref() methods are respected. */
	void connect ( KnSkeleton* s );

	/*! Update the transformations of the scene graph according
		to the joints in the skeleton sent to init().
		The skin and per-joint elements will only be updated if visible. */
	void update ();

	/*! Update the scene transformation relative to the given joint index j,
		of the skeleton sent to init. */
	void update ( int j );

	/*! Rebuild all joints of the current skeleton */
	void rebuild ();

	/*! Rebuild all data relative to the given joint index j */
	void rebuild ( int j );

   public : //=== end of virtual methods section ===

	/*! Returns the connected skeleton, or null if not connected */
	KnSkeleton* skeleton () { return _skeleton; }

	/*! Set the visibility state of the skin, skeleton, the visualization geometry,
		the collision geometry, and joint axis. The integers mean 1:show, 0:hide, and
		-1:does not change the visibility state. If the skin and/or all per-joint
		elements are not visible the non-visible elements will not be updated on
		a call to update() */
	void set_visibility ( int skin, int skel, int visgeo, int colgeo, int axis );

	/*! Same as the other set_visibility() method, but only affecting the given joint */
	void set_visibility ( KnJoint* joint, int skel, int visgeo, int colgeo, int vaxis );

	/*! Change the radius of the cylinders used to draw the skeleton */
	void set_skeleton_radius ( float radius );
	
	/*! Change the color of the spheres representing the skeleton joints */
	void set_skeleton_joint_color ( const GsColor& color );

	/*! Change the color of the sphere representing the given skeleton joint */
	void set_skeleton_joint_color ( KnJoint* joint, const GsColor& color );

	/*! Update the length of the axis to the new value */
	void set_axis_length ( float l );
	
	/*! Mark with the default red color the geometries attached to joint j */
	void mark_geometry ( KnJoint* j , GsColor color=GsColor::red );

	/*! Restore the original color of the geometries attached to joint j */
	void unmark_geometry ( KnJoint* j );

	/*! Restore original colors of all geometries */
	void unmark_all_geometries ();

	/*! Set the model m attached to j to have the given render mode, and be optionally
		marked with red color. Default mode restores the original render mode.
		If alpha!=-1 alpha is set in the diffuse color of geometries materials. */
	void set_geometry_style ( KnJoint* j, GsModel* m, gsRenderMode mode, bool mark, int alpha=-1 );
	
	/*! Get the default values for the skeleton radius and axis length */
	static void get_defaults ( float& gsadius, float& alen );
};

/*! Utility function to draw in snlines a 3D graphical representation of the swing limits */
void draw_swing_limits ( KnJoint* j, SnLines* snlines, float r, GsColor color );

//================================ End of File =================================================

# endif  // KN_SCENE_H
