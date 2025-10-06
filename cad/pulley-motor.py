from build123d import *
from ocp_vscode import *

outer_dia = 3.5
hole_dia1 = 2
hole_dia2 = 0.6 # 1.5 - reduced for JLC
slot_dia = 1.5
thickness = 4.5

bottom = (Align.CENTER, Align.CENTER, Align.MAX)

with BuildPart() as p:
    Cylinder(outer_dia/2, thickness)
    e = p.edges().sort_by(Axis.Z)
    #fillet([ e[0], e[2] ], radius=0.25)
    Torus(outer_dia/2, slot_dia/2, mode=Mode.SUBTRACT)
    # make longer for JLC
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1].offset(0)):
        Circle(1.5)
    extrude(amount=6)
    # hole
    Cylinder(hole_dia2/2, 20, 
             mode=Mode.SUBTRACT)
show(p)

export_step(p.part, 'pulley-motor.step')
