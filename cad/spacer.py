from build123d import *
from ocp_vscode import *
from defs import *

th = 6
cd = 60

with BuildPart() as p:
    Box(ow, ow, th)
    # round cutout
    Cylinder(cd/2 + 2, th, mode=Mode.SUBTRACT)
    # 4 screw holes
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1]) as sk:
        with PolarLocations(radius=screw_radius, count=4, start_angle=45):
            Circle(3.2/2)
    extrude(amount=-th, mode=Mode.SUBTRACT)
    # rectangular cutout
    with BuildSketch(p.faces().sort_by(Axis.Z)[0]) as sk:
        with Locations([(cd/2, 0, 0)]):
            Rectangle(cd, cd-5)
    extrude(amount=-th, mode=Mode.SUBTRACT)
    # side screw holes
    with BuildSketch(p.faces().sort_by(Axis.Y)[0]) as sk:
        with PolarLocations(radius=motor_bracket_cc/2, count=2, start_angle=90):
            Circle(radius=3/2)
    extrude(amount=-20, mode=Mode.SUBTRACT)
    
show(p)

export_step(p.part, 'spacer.step')
