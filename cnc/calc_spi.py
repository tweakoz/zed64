#!/usr/bin/python

teeth_per_rot = 20.0
mm_per_tooth = 2.0
mm_per_rot = teeth_per_rot*mm_per_tooth
steps_per_rot = 200.0

mm_per_step = mm_per_rot/steps_per_rot 
in_per_step = mm_per_step/25.4
steps_per_in = 1.0 / in_per_step
steps_per_mm = 1.0 / mm_per_step

print "teeth_per_rot<%f>" % teeth_per_rot
print "mm_per_tooth<%f>" % mm_per_tooth
print "steps_per_rot<%f>" % steps_per_rot
print "mm_per_rot<%f>" % mm_per_rot
print "mm_per_step<%f>" % mm_per_step
print "in_per_step<%f>" % in_per_step
print "steps_per_mm<%f>" % steps_per_mm
print "steps_per_in<%f>" % steps_per_in
print "halfsteps_per_in<%f>" % (steps_per_in*2.0)
