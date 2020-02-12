#include "csg_prim.h"

/*!
	Only have to include this header to access functionality of CSG tree 
	This code provides functions to build a CSG object out of primitives

	*/

class CSGTree {

public:
	//operations for CSG Tree
	enum Operation { Union, Difference, Intersection, None };

	Operation op; 
	CSGTree* left;
	CSGTree* right;

	CSGPrimitive* obj;
	GsMaterial mat;

	/*! Constructors
		New primitives are always added to the left */
	CSGTree(CSGPrimitive* prim);
	CSGTree(Operation op, CSGTree* left, CSGTree* right);

	/*! Primary operations*/
	void push(Operation op, CSGPrimitive* left);
	int intersects(const GsVec& orig, const GsVec& dir, GsVec& incident1, GsVec& incident2, GsVec& normal);
	bool contains(GsPnt& p);

	/*! debugging functions, print information relating the corresponding function*/
	void dbg();
	void dbg_intersect(GsVec Lin1, GsVec Lin2, GsVec Rin1, GsVec Rin2, int Lx, int Rx, int op);
};

/*! Constructor with single primitive */
CSGTree::CSGTree(CSGPrimitive* prim) {
	obj = prim;
	op = None;
	left = NULL;
	right = NULL;
}

/*! Constructor that combines two primitives with one operator*/
CSGTree::CSGTree(Operation new_op, CSGTree* leftprim, CSGTree* rightprim) {
	op = new_op;
	left = leftprim;
	right = rightprim;
	obj = NULL;
}

/*! add an operator and another primitive
	for when a tree only contains a single primitive */
void CSGTree::push(Operation new_op, CSGPrimitive* prim) {
	op = new_op;
	right = new CSGTree(obj);
	left = new CSGTree(prim);
	obj = NULL;
}

/*! Traverse the tree and check if point lies inside object while following operations */
bool CSGTree::contains(GsPnt& p) {
	if (op == None) { //base case
		return obj->contains(p);
	}

	int Lc, Rc;
	Lc = left->contains(p);
	Rc = right->contains(p);

	if (op == Union) {
		if (Lc || Rc) return true;
	}

	else if (op == Difference) {
		if (Rc) return false;
		else if (Lc) return true;
	}

	else if (op == Intersection) {
		if (Lc && Rc) return true;
	}

	return false;
}

/*! Traverse the tree and check if a ray intersects the object */
//int CSGTree::intersects(const GsVec& orig, const GsVec& dir, GsVec& incident1, GsVec& incident2) {
//	GsVec Lin1, Lin2, Rin1, Rin2;
//	int Lx, Rx;
//	
//	//check primitive for interesction, and store incident point if true
//	if (op == None){
//		return obj->intersects(orig, dir, incident1, incident2);
//	}
//
//	Lx = left->intersects(orig, dir, Lin1, Lin2);
//	Rx = right->intersects(orig, dir, Rin1, Rin2);
//	
//	//check values of intersection tests
//	//dbg_intersect(Lin1, Rin1, Lin2, Rin2, Lx, Rx, op);
//
//	if (op == Union){ //tested at least once
//		if (!Lx && !Rx){
//			return 0;
//		}
//		
//		else if (Lx && !Rx){
//			incident1 = Lin1;
//			incident2 = Lin2;
//			return 2;
//		}
//		
//		else if (!Lx && Rx){
//			incident1 = Rin1;
//			incident2 = Rin2;
//			return 2;
//		}
//		
//		else { //ray intersects both prims, use the closer incident
//			if (dist(orig, Lin1) < dist(orig, Rin1) ) incident1 = Lin1;
//			else incident1 = Rin1;
//
//			if (dist(orig, Lin2) > dist(orig, Rin2) ) incident2 = Lin2;
//			else incident2 = Rin2;
//
//			return 2;
//		}
//	}
//
//	else if (op == Difference){ //left - right //tested at least twice
//		if (!Lx) return 0;
//
//		else if (right->contains(Lin1) && right->contains(Lin2)) return 0;
//		
//		else if (Lx && !Rx){
//			incident1 = Lin1; 
//			incident2 = Lin2; 
//			return 1;
//		}
//		
//		else if (dist(orig, Lin1) < dist(orig, Rin1)){
//			incident1 = Lin1;
//			if (left->contains(Rin1)) incident2 = Rin1;
//			else incident2 = Lin2;
//			return 2;
//		}
//		
//		else if (dist(orig, Lin1) > dist(orig, Rin1)){
//			if (right->contains(Lin1) && left->contains(Rin2)){
//				incident1 = Rin2;
//				incident2 = Lin2;
//			}
//			else {
//				incident1 = Lin1;
//				incident2 = Lin2;
//			}
//			return 2;
//		}  
//
//	}
//
//	else if (op == Intersection) { //tested at least once
//		if (!Lx || !Rx){
//			return 0;
//		}
//		else if (Lx && Rx){
//			incident1 = (!left->contains(Rin1)) ? Lin1 : Rin1;
//			incident2 = (left->contains(Rin2)) ? Rin2 : Lin2;
//			return 2;
//		}
//	}
//
//	gsout << "invalid operation" << gsnl; return false;
//}

int CSGTree::intersects(const GsVec& orig, const GsVec& dir, GsVec& incident1, GsVec& incident2, GsVec& normal) {
	GsVec Lin1, Lin2, Rin1, Rin2, lnorm, rnorm;
	int Lx, Rx;

	//base case
	//check primitive for interesction, and store incident point and normal vector if true
	if (op == None) {
		Lx = obj->intersects(orig, dir, incident1, incident2); //intersection function returns incident point needed to calculate normal
		normal = obj->get_normal(orig, dir, incident1);
		return Lx;
	}

	Lx = left->intersects(orig, dir, Lin1, Lin2, lnorm);
	Rx = right->intersects(orig, dir, Rin1, Rin2, rnorm);

	//check values of intersection tests
	//dbg_intersect(Lin1, Rin1, Lin2, Rin2, Lx, Rx, op);

	if (op == Union) { //tested at least once
		if (!Lx && !Rx) {
			return 0;
		}

		else if (Lx && !Rx) {
			incident1 = Lin1;
			incident2 = Lin2;
			normal = lnorm;
		}

		else if (!Lx && Rx) {
			incident1 = Rin1;
			incident2 = Rin2;
			normal = rnorm;
		}

		else { //ray intersects both prims, use the closer incident (for normal too)
			if (dist(orig, Lin1) < dist(orig, Rin1)) { 
				incident1 = Lin1; normal = lnorm; 
			}
			else incident1 = Rin1; normal = rnorm;

			if (dist(orig, Lin2) > dist(orig, Rin2)) {
				incident2 = Lin2;
			}
			else incident2 = Rin2;
		}

		return 2;
	}

	else if (op == Difference) { //left - right //tested at least twice
		if (!Lx) return 0;

		else if (right->contains(Lin1) && right->contains(Lin2)) return 0;

		else if (Lx && !Rx) {
			incident1 = Lin1;
			incident2 = Lin2;
			normal = rnorm;
			return 1;
		}

		else if (dist(orig, Lin1) < dist(orig, Rin1)) {
			incident1 = Lin1;
			normal = rnorm;
			if (left->contains(Rin1)) incident2 = Rin1;
			else incident2 = Lin2;
			return 2;
		}

		else if (dist(orig, Lin1) > dist(orig, Rin1)) {
			if (right->contains(Lin1) && left->contains(Rin2)) {
				incident1 = Rin2;
				incident2 = Lin2;
				normal = lnorm;
			}
			else {
				incident1 = Lin1;
				incident2 = Lin2;
				normal = rnorm;
			}
			return 2;
		}

	}

	else if (op == Intersection) { //tested at least once
		if (!Lx || !Rx) {
			return 0;
		}

		else if (Lx && Rx) {
			if (left->contains(Rin1) && right->contains(Lin2)) {
				incident1 = Rin1;
				incident2 = Lin2;
				normal = rnorm;
				return 2;
			}

			if (right->contains(Lin1) && left->contains(Rin2)) {
				incident1 = Lin1;
				incident2 = Rin2;
				normal = lnorm;
				return 2;
			}
		}

		return 0;
	}

	gsout << "invalid operation" << gsnl; return false;
}

void CSGTree::dbg() {
	gsout << op << gsnl;
	gsout << left << gsnl;
	gsout << right << gsnl;
	gsout << obj << gsnl;
}
void CSGTree::dbg_intersect(GsVec Lin1, GsVec Lin2, GsVec Rin1, GsVec Rin2, int Lx, int Rx, int op) {
	gsout << "operation: " << op << gsnl;
	gsout << "Lin1: " << Lin1 << gsnl;
	gsout << "Rin1: " << Rin1 << gsnl;
	gsout << "Lin2: " << Lin2 << gsnl;
	gsout << "Rin2: " << Rin2 << gsnl;
	gsout << "Lx: " << Lx << gsnl;
	gsout << "Rx: " << Rx << gsnl;
}
