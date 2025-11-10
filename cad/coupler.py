from build123d import *
from ocp_vscode import *
from defs import *

h = 10
r1 = 2/2
r2 = 1.5/2
r3 = 3

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
    
show(p)

export_step(p.part, 'coupler.step')
