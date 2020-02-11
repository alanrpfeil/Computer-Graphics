# SIG Skeleton Definition

# Signature:
KnSkeleton

# the name of this skeleton:
name TwoLinks84

# starts the skeleton definition
skeleton
root joint1
{ visgeo "link.m"
  colgeo "link.m"
  modelrot 0 0 1 -90 # to orient the link in x direction
  channel ZRot 0 lim -180 180

  joint joint2
   { offset 8 0 0
     visgeo "link4.m"
     colgeo "link4.m"
     modelrot 0 0 1 -90 # to orient the link in x direction
     channel ZRot 0 lim -180 180

     joint wrist
      { offset 4 0 0
        channel ZRot 0 lim -180 180

        joint hand
         { visgeo "hand.m"
           colgeo "hand.m"
           modelrot 0 0 1 -90 # to orient the hand in x direction
           channel ZRot 0 lim 0 0
         }
      }
   }
}

# end is optional; usefull to stop when other data comes after this point
end
