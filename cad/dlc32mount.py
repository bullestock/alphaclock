from pathlib import Path
from build123d import *
from ocp_vscode import *
from defs import *

width, length = 70.5, 90.5
th = 2
brim_w = 2
brim_th = 6

bottom = (Align.CENTER, Align.CENTER, Align.MIN)

with BuildPart() as p:
    Box(width + 2*brim_w, length + 2*brim_w, th + brim_th,
        align=bottom)
    fillet(p.edges().filter_by(Axis.Z), radius=3)
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1]) as sk:
        Rectangle(width, length)
    extrude(amount=-brim_th, mode=Mode.SUBTRACT)
    fillet(p.edges().filter_by(Axis.X), radius=.5)
    with BuildSketch(p.faces().sort_by(Axis.Z)[0]) as sk:
        with GridLocations(62, 82, 2, 2):
            Circle(radius=2.9/2)
    extrude(amount=-4)
    
show(p)

export_step(p.part, f'{os.path.basename(__file__)}.step')

