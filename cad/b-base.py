from build123d import *
from ocp_vscode import *

th = 5
cd = 25
insert_r = 4.2/2

with BuildPart() as p:
    Box(40, 40, th)
    Cylinder(cd/2 + 2, th, mode=Mode.SUBTRACT)
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1]) as sk:
        with PolarLocations(radius=22, count=4, start_angle=45):
            Circle(insert_r)
    extrude(amount=-th, mode=Mode.SUBTRACT)
    
show(p)

export_step(p.part, 'b-base.step')
