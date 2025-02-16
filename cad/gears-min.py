import cadquery as cq
from cq_gears import BevelGear
import numpy as np
        
module = 1
# teeth
t1 = 55
t2 = 20
# bore holes
d1 = 7
d2 = 1.55

aa_sin = np.sin(np.radians(90))
aa_cos = np.cos(np.radians(90))
delta1 = np.arctan(aa_sin / (t2/t1 + aa_cos))
delta2 = np.arctan(aa_sin / (t1/t2 + aa_cos))

pressure_angle=20.0

gear1 = BevelGear(module, t1, np.degrees(delta1),
                  face_width=6.0, pressure_angle=pressure_angle, helix_angle=42.0,
                  bore_d=9*2)

gear2 = BevelGear(module, t2, np.degrees(delta2), 
                  face_width=6.0, pressure_angle=pressure_angle, helix_angle=-42.0,
                  bore_d=d2)

g1 = (cq.Workplane('XY').
      gear(gear1))

wp1 = (cq.Workplane('XY').
       circle(54/2).
       extrude(-5).
       tag("o").
       workplaneFromTagged("o").
       workplane(2.5).
       circle(7/2).
       cutThruAll()
       )

wp1 = wp1 + g1
#wp2 = cq.Workplane('XY').transformed(offset=(45, 0, 0)).gear(gear2)
show_object(wp1)
#show_object(wp2)
