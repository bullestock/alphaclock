from build123d import *
from ocp_vscode import *
from defs import *

h = 15
r1 = 2/2
r2 = 1.5/2
r3 = 3
slit_w = 1.5
slit_h = 7.5

with BuildPart() as p:
    with BuildSketch():
        Circle(r3)
    extrude(amount=h)
    with BuildSketch():
        Circle(r1)
    extrude(amount=h/2, mode=Mode.SUBTRACT)
    with BuildSketch():
        Circle(r2)
    extrude(amount=h, mode=Mode.SUBTRACT)
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1].offset(-(h - slit_h)/2)):
        Rectangle(2*r3, slit_w)
        Rectangle(slit_w, 2*r3)
    extrude(amount=-slit_h, mode=Mode.SUBTRACT)
    
show(p)

export_step(p.part, f'coupler{h}.step')
