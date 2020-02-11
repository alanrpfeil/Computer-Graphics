# SIG Skeleton Definition

# Signature:
KnSkeleton

# the name of this skeleton:
name OneArm

# starts the skeleton definition
skeleton
root body
{ visgeo "body.m"
  colgeo "body.m"
  channel ZRot -90 lim -120 -60

  joint shoulder
   { offset 0 3 0
     visgeo "link.m"
     colgeo "link.m"
     channel ZRot 90 lim -90 90

     joint elbow
      { offset 0 9 0
	    visgeo "link.m"
	    colgeo "link.m"
        channel ZRot -150 lim -150 150

        joint wrist
         { offset 0 9 0
           visgeo "hand.m"
           colgeo "hand.m"
           channel ZRot 60 lim -90 90

           joint finger # used for the distance function
            { offset 0 3 0
            }
         }
      }
   }
}

# Note: some of the following data must come after the skeleton definition

# Set known pair of joints that do not need to be tested for self collisions:
collision_free_pairs
body     shoulder
shoulder elbow
elbow    wrist
shoulder wrist;

# Set joints to be used in the distance function betweeen postures
dist_func_joints
shoulder elbow wrist finger;

# Pre-defined postures: (Quat and Swing are saved in axis-angle format)
posture initial  -90.0 90.0 -150.0 60.0
posture backup   -60.0 55.7 106.0 -16
posture up       -66.0 71.0 -39.0 -54.0
posture midup    -91.0 62 -58 -5.0
posture mid      -90.0 0.0 0.0 0.0
posture middown  -91.0 -50.0 39.0 14.0
posture down     -120.0 -60.0 39.0 52.0
posture backdown -120.0 -81.0 -71.0 -16

# Any additional data declared here will be loaded and stored in the skeleton:
userdata
{ mydata.draw = wrist blue;  # strings
  drm.grid_bbox = -14.5 -23.5 -1.0 24.9 23.5 1.0; # reals
  rrt.insertion_tries = 5; # integers
}

# end keyword is optional; usefull to stop when other data comes after this point
end
