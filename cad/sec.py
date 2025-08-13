from build123d import *
from ocp_vscode import *
from defs import *

i_dia = 2.1
rr = 0.5

p_dia = 65
slot_dia = 2
p_th = 4.5

int_th = 1

pulley_z = bearing_th + p_th/2 + int_th

bottom = (Align.CENTER, Align.CENTER, Align.MIN)

with BuildPart() as p:
    Cylinder(bearing_id/2 - b_crush/2, bearing_th, align=bottom)
    e = p.edges().sort_by(Axis.Z)
    fillet([e[0]], radius=rr)
    with BuildSketch(p.faces().sort_by(Axis.Z)[0].offset(-rr)) as sk:
        with PolarLocations(radius=bearing_id/2 - b_crush/2, count=10):
            Circle(b_crush)
    extrude(amount=-(bearing_th - 2*rr))
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1]) as sk:
        Circle(bearing_id/2 + 1)
    extrude(amount=int_th)
    with Locations([(0, 0, pulley_z)]):
        Cylinder(p_dia/2, p_th)
        Torus(p_dia/2, slot_dia/2, mode=Mode.SUBTRACT)
    # through hole
    Cylinder(i_dia/2, 20, align=bottom,
             mode=Mode.SUBTRACT)

show(p)

export_step(p.part, 'sec.step')
