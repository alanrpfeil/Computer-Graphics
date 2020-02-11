# SIG Skeleton Definition

KnSkeleton

name TwoArm

skeleton
root body
{ modelmat 0 1 0 0   -1 0 0 0   0 0 1 0   0 0 0 1
  visgeo "body.m"
  colgeo "body.m"
  channel ZRot 0 -35 35

  joint lshoulder
   { offset 0 3.5 0
     visgeo "link.m"
     colgeo "link.m"
     channel ZRot 0 -110 90

     joint lelbow
      { offset 0 9 0
	    visgeo "link.m"
	    colgeo "link.m"
        channel ZRot -90 -150 0

        joint lhand
         { offset 0 9 0
           visgeo "hand.m"
           colgeo "hand.m"
           channel ZRot 0 -90 90
 
           joint lfinger # needed for the distance function
            { offset 0 3 0
            }
        }
      }
   }

  joint rshoulder
   { offset 0 -3.5 0
     modelmat -1 0 0 0   0 -1 0 0   0 0 1 0   0 0 0 1
     visgeo "link.m"
     colgeo "link.m"
     channel ZRot 0 -90 110

     joint relbow
      { offset 0 -9 0
        modelmat -1 0 0 0   0 -1 0 0   0 0 1 0   0 0 0 1
	    visgeo "link.m"
	    colgeo "link.m"
        channel ZRot 90 0 150

        joint rhand
         { offset 0 -9 0
	       modelmat -1 0 0 0   0 -1 0 0   0 0 1 0   0 0 0 1
           visgeo "hand.m"
           colgeo "hand.m"
           channel ZRot 0 -90 90

           joint rfinger # needed for the distance function
            { offset 0 -3 0
            }
         }
      }
   }
}

posture zero 0.0 0.0 0.0 0.0 0.0 0.0 0.0
posture init 0.0 0.0 -90.0 0.0 0.0 90.0 0.0

collision_free_pairs
body lshoulder     body rshoulder   lshoulder lelbow
rshoulder relbow   lelbow lhand     relbow rhand;

dist_func_joints
lshoulder lelbow lhand lfinger rshoulder relbow rhand rfinger;

