import cadquery as cq
from cq_gears import BevelGear
import numpy as np
        
module = 1
# teeth
t1 = 40
t2 = 7
# bore holes
d1= 2.55
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

wp1 = (cq.Workplane('XY').
       tag("o").
       gear(gear1).
       workplaneFromTagged("o").
       workplane(2).
       circle(3).
       cutBlind(10)
       )

wp2 = cq.Workplane('XY').transformed(offset=(45, 0, 0)).gear(gear2)
#show_object(wp1)
show_object(wp2)
