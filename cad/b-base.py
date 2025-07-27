from build123d import *
from ocp_vscode import *
from defs import *

th = 2
stud_r = 3
stud_h = 5

bottom = (Align.CENTER, Align.CENTER, Align.MIN)

with BuildPart() as p:
    # basic shape
    Box(ow, ow, th, align=bottom)
    # insert studs/holes
    with Locations(p.faces().sort_by(Axis.Z)[-1]) as sk:
        with PolarLocations(radius=screw_radius, count=4, start_angle=45):
            Cylinder(radius=stud_r, height=stud_h - th, align=bottom)
        with PolarLocations(radius=screw_radius, count=4, start_angle=45):
            Cylinder(radius=insert_r - 0.1, height=2*stud_h, mode=Mode.SUBTRACT)
    # cutout
    with BuildSketch(p.faces().sort_by(Axis.Z)[0]) as sk:
        with Locations((0, 10, 0)):
            RectangleRounded(ow - 20, ow, 3)
    extrude(amount=-th, mode=Mode.SUBTRACT)
    
show(p)

export_step(p.part, 'b-base.step')
