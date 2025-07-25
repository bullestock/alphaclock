from build123d import *
from ocp_vscode import *

d = 3.7 + 0.2
h = 3
hole_r = 3.2/2

with BuildPart() as p:
    Cylinder(d/2, h)
    Cylinder(hole_r, h, mode=Mode.SUBTRACT)
    
show(p)

export_step(p.part, 'min-adapter.step')
