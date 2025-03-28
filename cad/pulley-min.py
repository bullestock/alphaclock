from build123d import *
from ocp_vscode import *

outer_dia = 130
thickness = 4.5
hole_dia = 5.8
lip_thickness = 0.75
ridge_outer_dia = 20
ridge_inner_dia = 15
ridge_height = 0.75
large_hole_dia = 17
large_hole_depth = thickness - lip_thickness
slot_dia = 2

bottom = (Align.CENTER, Align.CENTER, Align.MIN)
top = (Align.CENTER, Align.CENTER, Align.MAX)

with BuildPart() as p:
    Cylinder(outer_dia/2, thickness)
    fillet(p.edges().sort_by(Axis.Z)[0], radius=0.25)
    Torus(outer_dia/2, slot_dia/2, mode=Mode.SUBTRACT)
    # though hole
    Cylinder(hole_dia/2, 10, 
             mode=Mode.SUBTRACT)
    # large hole
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1]):
        Circle(large_hole_dia/2)
    extrude(amount=-large_hole_depth, mode=Mode.SUBTRACT)
    # ridge
    with BuildSketch(p.faces().sort_by(Axis.Z)[0]):
        Circle(ridge_outer_dia/2)
    extrude(amount=ridge_height)
    with BuildSketch(p.faces().sort_by(Axis.Z)[0]):
        Circle(ridge_inner_dia/2)
    extrude(amount=-ridge_height, mode=Mode.SUBTRACT)
    

show(p)

export_step(p.part, 'pulley-min.step')
