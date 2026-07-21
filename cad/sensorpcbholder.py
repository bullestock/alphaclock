from build123d import *
from ocp_vscode import *
from defs import *
from epilogue import *

pcb_w = 38.5
pcb_h = 38
pcb_th = 1.5 # actually 1.3

width = pcb_w + 15
foot_len = 15
th = 5

with BuildPart() as p:
    with BuildSketch():
        with Locations((0, foot_len/2)):
            RectangleRounded(width, foot_len, 1)
    extrude(amount=3)
    with BuildSketch():
        RectangleRounded(width, th, 1)
    with BuildSketch(Plane.XY.offset(pcb_h)):
        RectangleRounded(pcb_w + 6, th, 1)
    loft()
    with BuildSketch(Plane.XY.offset(3)):
        Rectangle(pcb_w, pcb_th)
    extrude(amount=pcb_h, mode=Mode.SUBTRACT)
    with BuildSketch(Plane.XY.offset(3)):
        Rectangle(pcb_w - 3, th)
    extrude(amount=pcb_h, mode=Mode.SUBTRACT)

epilogue(p)
