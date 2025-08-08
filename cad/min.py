from build123d import *
from ocp_vscode import *
from defs import *

stem_h = 14.5
o_dia = 5.9
i_dia = 4.1
b_crush = 0.45
h_crush = 0.2
rr = 0.5

p_dia = 40
slot_dia = 2
p_th = 4.5

bearing_clearance = 1

bearing_z = p_th

bottom = (Align.CENTER, Align.CENTER, Align.MIN)

with BuildPart() as p:
    # pulley
    Cylinder(p_dia/2, p_th)
    Torus(p_dia/2, slot_dia/2, mode=Mode.SUBTRACT)
    # spacer
    with Locations([(0, 0, bearing_z/2)]):
        Cylinder(bearing_id/2 + 0.5, bearing_clearance)
    # part that fits inside bearing
    with Locations([(0, 0, bearing_z)]):
        Cylinder(bearing_id/2 - b_crush/2, bearing_th)
    # bearing crush ribs
    with BuildSketch(p.faces().sort_by(Axis.Z)[0].offset(-p_th-rr)) as sk:
        with PolarLocations(radius=bearing_id/2 - b_crush/2 - 0.1, count=10):
            Circle(b_crush)
    extrude(amount=-(bearing_th - 2*rr))
    # through hole
    Cylinder(i_dia/2, 50,
             mode=Mode.SUBTRACT)
    
show(p)

export_step(p.part, 'min.step')
