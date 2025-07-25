from build123d import *
from ocp_vscode import *
from defs import *

width, length = 44.25, 77.25
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
        with Locations((-(width/2 - 37.65), length/2 - 61.98)):
            Circle(radius=4)
    extrude(amount=-4)
    with BuildSketch(p.faces().sort_by(Axis.Z)[0]) as sk:
        with Locations((-(width/2 - 37.65), length/2 - 61.98)):
            Circle(radius=insert_r - 0.1)
    extrude(amount=-20, mode=Mode.SUBTRACT)
    
show(p)

export_step(p.part, 'pcbmount.step')
