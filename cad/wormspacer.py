from build123d import *
from ocp_vscode import *
from defs import *

h = 2
r1 = 4/2
r2 = 2/2

with BuildPart() as p:
    with BuildSketch():
        Circle(r1)
    extrude(amount=h)
    with BuildSketch():
        Circle(r2)
    extrude(amount=h, mode=Mode.SUBTRACT)
    
show(p)

export_step(p.part, 'wormspacer.step')
