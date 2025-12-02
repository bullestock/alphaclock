from build123d import *
from ocp_vscode import *
from defs import *

h = 13.5 + 8 - 5
r1 = 8/2
r2 = 3.2/2
cc = 25

with BuildPart() as p:
    with BuildSketch(Plane.XY):
        with Locations((-cc/2, 0), (cc/2, 0)):
            Circle(r1)
        Rectangle(cc - r2, r1)
    extrude(amount=h)
    with BuildSketch(Plane.XY):
        with Locations((-cc/2, 0), (cc/2, 0)):
            Circle(r2)
    extrude(amount=h, mode=Mode.SUBTRACT)
    fillet(p.edges().filter_by(Axis.Z), radius=3)
    
    
show(p)

export_step(p.part, 'secspacer.step')
