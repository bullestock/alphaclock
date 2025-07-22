from build123d import *
from ocp_vscode import *

outer_dia = 54
hole_dia1 = 6
hole_dia2 = 2.55
slot_dia = 2
thickness = 4.5
stud_dia = 14
stud_height = 2

bottom = (Align.CENTER, Align.CENTER, Align.MIN)

with BuildPart() as p:
    Cylinder(outer_dia/2, thickness)
    fillet(p.edges().sort_by(Axis.Z)[0], radius=0.25)
    Torus(outer_dia/2, slot_dia/2, mode=Mode.SUBTRACT)
    # stud
    with BuildSketch(p.faces().sort_by(Axis.Z)[0]):
        Circle(stud_dia/2)
    extrude(amount=stud_height)
    # large hole
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1]):
        Circle(hole_dia1/2)
    extrude(amount=-2, mode=Mode.SUBTRACT)
    # small hole
    Cylinder(hole_dia2/2, 10, 
             mode=Mode.SUBTRACT)
show(p)

export_step(p.part, 'pulley-sec.step')
