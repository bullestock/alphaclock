from build123d import *
from ocp_vscode import *
from defs import *
from epilogue import *

pcb_w, pcb_l = 21, 32
pcb_th = 1.7
solder_th = 2.5

dist = 5
brim_w = 2

width = 2*pcb_w + dist + 2*brim_w
length = pcb_l + 2*brim_w

th = 3.5

with BuildPart() as p:
    # base
    with BuildSketch():
        RectangleRounded(width, length, 1)
    extrude(amount=th+pcb_th+solder_th)
    # pcb cutouts
    with BuildSketch():
        with GridLocations(pcb_w + dist, 1, 2, 1):
            Rectangle(pcb_w, pcb_l)
    extrude(amount=pcb_th+solder_th, mode=Mode.SUBTRACT)
    # screw hole
    with BuildSketch():
        Circle(radius=2.7/2)
    extrude(amount=th+pcb_th, mode=Mode.SUBTRACT)
    # lips
    with BuildSketch(first_z(p)):
        with GridLocations(2*pcb_w + dist + 1, 1, 2, 1):
            Rectangle(brim_w+1, 5)
    extrude(amount=2)
    # pcb supports
    with BuildSketch(last_z(p).offset(-th)):
        with GridLocations(pcb_w + dist, 1, 2, 1):
            with GridLocations(pcb_w, 16, 2, 2):
                Rectangle(4, 2)
    extrude(amount=-solder_th)
epilogue(p)
