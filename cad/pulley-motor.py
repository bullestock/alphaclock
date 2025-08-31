from build123d import *
from ocp_vscode import *

outer_dia = 4
hole_dia1 = 2
hole_dia2 = 1.6
slot_dia = 2
thickness = 4.5

bottom = (Align.CENTER, Align.CENTER, Align.MIN)

with BuildPart() as p:
    Cylinder(outer_dia/2, thickness)
    e = p.edges().sort_by(Axis.Z)
    fillet([ e[0], e[2] ], radius=0.25)
    Torus(outer_dia/2, slot_dia/2, mode=Mode.SUBTRACT)
    # hole
    Cylinder(hole_dia2/2, 10, 
             mode=Mode.SUBTRACT)
show(p)

export_step(p.part, 'pulley-motor.step')
