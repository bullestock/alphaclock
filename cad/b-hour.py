from build123d import *
from ocp_vscode import *

bearing_id = 20
bearing_th = 4
stem_h = 15
o_dia = 5.9
i_dia = 3.1
b_crush = 0.45
h_crush = 0.2
rr = 0.5

p_dia = 15
slot_dia = 2
p_th = 4.5

pulley_z = bearing_th + p_th/2


bottom = (Align.CENTER, Align.CENTER, Align.MIN)

with BuildPart() as p:
    Cylinder(o_dia/2 - h_crush/2, stem_h, align=bottom)
    Cylinder(bearing_id/2 - b_crush/2, bearing_th, align=bottom)
    e = p.edges().sort_by(Axis.Z)
    fillet([e[0], e[2], e[5]], radius=rr)
    with BuildSketch(p.faces().sort_by(Axis.Z)[0].offset(-rr)) as sk:
        with PolarLocations(radius=bearing_id/2 - b_crush/2, count=10):
            Circle(b_crush)
    extrude(amount=-(bearing_th - 2*rr))
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1].offset(-rr)) as sk:
        with PolarLocations(radius=o_dia/2 - h_crush/2, count=10):
            Circle(h_crush)
    extrude(amount=-2)
    with Locations([(0, 0, pulley_z)]):
        Cylinder(p_dia/2, p_th)
        Torus(p_dia/2, slot_dia/2, mode=Mode.SUBTRACT)
    Cylinder(i_dia/2, stem_h*2, align=bottom,
             mode=Mode.SUBTRACT)
    
show(p)

export_step(p.part, 'b-hour.step')
