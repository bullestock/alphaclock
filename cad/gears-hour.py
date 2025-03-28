import cadquery as cq
from cq_gears import BevelGear
import math
import numpy as np
        
module = 1
# teeth
t1 = 68
t2 = 12
# bore holes
d1 = 5.85
d2 = 1.55

aa_sin = np.sin(np.radians(90))
aa_cos = np.cos(np.radians(90))
delta1 = np.arctan(aa_sin / (t2/t1 + aa_cos))
delta2 = np.arctan(aa_sin / (t1/t2 + aa_cos))

pressure_angle=20.0

gear1 = BevelGear(module, t1, np.degrees(delta1),
                  face_width=6.0, pressure_angle=pressure_angle, helix_angle=42.0,
                  bore_d=d1)

gear2 = BevelGear(module, t2, np.degrees(delta2), 
                  face_width=6.0, pressure_angle=pressure_angle, helix_angle=-42.0,
                  bore_d=d2)

g1 = (cq.Workplane('XY').
      gear(gear1))

wp1 = (cq.Workplane('XY').
       tag('o').
       circle(66/2).
       extrude(-0.8).
       circle(d1/2).
       cutThruAll())

wp1 = wp1 + g1

theta = 10
r = 30
for i in range(0, 16):
    theta = math.radians(360*i/16)
    wp1 = (wp1.
           workplaneFromTagged('o').
           transformed(offset=(r*math.cos(theta), 
                               r*math.sin(theta), -0.5)).
           sphere(1)
           )

wp2 = cq.Workplane('XY').transformed(offset=(45, 0, 0)).gear(gear2)
show_object(wp1)
#show_object(wp2)
