from build123d import *
from ocp_vscode import *

th = 5
cd = 25

with BuildPart() as p:
    Box(40, 40, th)
    Cylinder(cd/2 + 2, th, mode=Mode.SUBTRACT)
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1]) as sk:
        with PolarLocations(radius=22, count=4, start_angle=45):
            Circle(3.2/2)
    extrude(amount=-th, mode=Mode.SUBTRACT)
    with BuildSketch(p.faces().sort_by(Axis.Z)[0]) as sk:
        with Locations([(cd/2, 0, 0)]):
            Rectangle(cd, cd)
    extrude(amount=-th, mode=Mode.SUBTRACT)
    with BuildSketch(p.faces().sort_by(Axis.Y)[0]) as sk:
        with PolarLocations(radius=12.5, count=2, start_angle=90):
            Circle(radius=3/2)
    extrude(amount=-20, mode=Mode.SUBTRACT)
    
show(p)

export_step(p.part, 'b-spacer.step')
