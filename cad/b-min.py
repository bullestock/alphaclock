from build123d import *
from ocp_vscode import *

bearing_id = 20
bearing_th = 4
stem_h = 15
o_dia = 3.6
i_dia = 1.1
b_crunch = 0.45
h_crunch = 0.2
rr = 0.5

bottom = (Align.CENTER, Align.CENTER, Align.MIN)

with BuildPart() as p:
    Cylinder(o_dia/2 - h_crunch/2, stem_h, align=bottom)
    Cylinder(bearing_id/2 - b_crunch/2, bearing_th, align=bottom)
    e = p.edges().sort_by(Axis.Z)
    fillet([e[0], e[2], e[5]], radius=rr)
    Cylinder(i_dia/2, stem_h*2, align=bottom,
             mode=Mode.SUBTRACT)
    with BuildSketch(p.faces().sort_by(Axis.Z)[0].offset(-rr)) as sk:
        with PolarLocations(radius=bearing_id/2 - b_crunch/2, count=10):
            Circle(b_crunch)
    extrude(amount=-(bearing_th - 2*rr))
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1].offset(-rr)) as sk:
        with PolarLocations(radius=o_dia/2 - h_crunch/2, count=10):
            Circle(h_crunch)
    extrude(amount=-2)
show(p)

export_step(p.part, 'b-min.step')
