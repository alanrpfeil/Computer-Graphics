/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef KN_SKIN_H
# define KN_SKIN_H

//================================ KnSkin ===================================

# include <sig/gs_array.h>

class SnLines;
class GsModel;
class KnJoint;
class KnSkeleton;

/*! Maintains a model and skinning weights.
	This class is usually owned (via sharing) by a KnSkeleton. */
class KnSkin : public GsShareable
{  public :
	struct Influence
	{	KnJoint* j; float w; GsVec v; //GsQuat q;
		//void init ( KnJoint* ja, float wa, GsVec va, GsQuat qa ) { j=ja; w=wa; v=va; q=qa; }
		void init ( KnJoint* ja, float wa, GsVec va ) { j=ja; w=wa; v=va; }
	};
   protected :
	GsArrayPt<GsArray<Influence>> _infs;
	GsArray<int> _infstart; // starting indices for each model, same size as _models
	GsArrayRef<GsModel> _models;
	KnSkeleton* _skeleton;

   public :
	/*! Constructor  */
	KnSkin ();

	/*! Destructor */
   ~KnSkin ();

	/*! Deletes weights and initializes the reference model*/
	void init ();

	void compress () {}

	/*! Returns the associated skeleton, or null if none */
	KnSkeleton* skeleton () const { return _skeleton; }

	/*! Returns the number of models in the skin */
	int models () const { return _models.size(); }

	/*! Returns the referenced model at given index i, which must be from 0 to models()-1 */
	GsModel* model ( int i ) { return _models[i]; }

	/*! Returns the influences of vertex index v in model index i. No range checks are made. */
	GsArray<Influence>& influences ( int m, int v ) { return *_infs[_infstart[m]+v]; }

	/*! Adds a skin model connected to sk and returns the index of the new model in the internal arrays.
		The given model will be referenced by KnSkin and can thus be shared with other objects.
		The internal arrays of influences are updated with entries for each vertex in m->V. */
	int add_model ( KnSkeleton* sk, GsModel* m );

	/*! Normalizes the weights influencing each vertex */
	void normalize_weights ();

	/*! Init by loading a skin mesh file (.m or .obj) and a skinning weight file (.w).
		Returns false if some error is encountered, otherwise true is returned.
		Parameter basedir can specify where to search for the skin mesh file, 
		and if not given, it is extracted from filename. */
	//bool load ( KnSkeleton* skel, const char* filename, const char* basedir=0 );

	/*! Computes the positions of all vertices of the skin according to the weights.
		Will only update if the skin mesh is visible, otherwise nothing is done. */
	void update ();
};

//================================ End of File ======================================

# endif  // KN_SKIN_H
