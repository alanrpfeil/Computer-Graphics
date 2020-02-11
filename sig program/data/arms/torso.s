# SIG Skeleton Definition

name Torso

KnSkeleton

skeleton
root body
{ modelrot axis 0 1 0 ang 90 # must come before visgeo/colgeo
  visgeo "body.m"
  colgeo "body.m"
  channel Quat axis 1 1 1 ang -120 frozen

  joint neck
   { offset 0 0 2
     channel Quat axis 1 1 1 ang 120 frozen

     joint head
      { offset 0 2.5 0
        visgeo "head.m"
        colgeo "head.m"
        channel Swing lim 40 120
        channel Twist 0 lim -20 20
      }
   }

  joint lshoulder
   { offset 0 3 1
     modelrot axis 1 0 0 ang 90
     visgeo "link.m"
     colgeo "link.m"
     prerot axis 0 1 1 ang 180
     channel Swing lim 110 150
     channel Twist 0 -120 90

     joint lelbow
      { offset 0 0 9
        modelrot axis 1 0 0 ang 90
        visgeo "link.m"
        colgeo "link.m"
        euler ZY  #  otherwise YXZ is the default
        channel YRot -90 lim -170 0
        channel ZRot 0 lim -35 120

        joint lhand
         { offset 0 0 9
           modelrot axis 1 0 0 ang 90
           visgeo "hand.m"
           colgeo "hand.m"
           channel Swing lim 50 90
         }
      }
   }

  joint rshoulder
   { offset 0 -3 1
     modelrot axis 1 0 0 ang 90
     visgeo "link.m"
     colgeo "link.m"
     prerot axis 1 0 0 ang 90
     channel Swing lim 110 150
     channel Twist 0 -90 120

     joint relbow
      { offset 0 0 9
        modelrot axis 1 0 0 ang 90
        visgeo "link.m"
        colgeo "link.m"
        euler ZY  #  otherwise YXZ is the default
        channel YRot 90 lim 0 170
        channel ZRot 0 lim -120 35

        joint rhand
         { offset 0 0 9
           modelrot axis 1 0 0 ang 90
           visgeo "hand.m"
           colgeo "hand.m"
           channel Swing lim 50 90
         }
      }
   }
}

ik RightArm rhand
ik LeftArm lhand

posture init 0 0 0 0 0 0 -90 0 0 0 0 0 0 90 0 0 0
posture zero 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
posture rup 0 0 0 0 0 0 -90 0 0 0 -0.655937 0.528783 43 90 0 0 0
posture lup 0 0 0 -0.724983 -0.653202 -47.666668 -90 0 0 0 0 0 0 90 0 0 0

