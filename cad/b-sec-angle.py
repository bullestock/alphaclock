from build123d import *
from ocp_vscode import *
from defs import *
import math

th = 3
cd = 24
height = 15
width = 40
depth = 30
z_offset = th - 6

motor_bracket_cc = 23

bottom = (Align.CENTER, Align.CENTER, Align.MIN)

with BuildPart() as p:
    Box(width, depth, th)
    # flange
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1].offset(-th)):
        with Locations((0, depth/2)):
            Rectangle(width, th)
    extrude(amount=height)
    # slots
    with BuildSketch(p.faces().sort_by(Axis.Z)[0]) as sk:
        with Locations((22*math.cos(math.radians(45)), 3, 0),
                       (-22*math.cos(math.radians(45)), 3, 0)):
            RectangleRounded(3.2, 30, 3.1/2)
        with Locations((0, 10, 0)):
            RectangleRounded(cd, 60, cd/2 - 0.01)
    extrude(amount=-th, mode=Mode.SUBTRACT)
    # insert holes
    with BuildSketch(p.faces().sort_by(Axis.Y)[-1]) as sk:
        with Locations([(motor_bracket_cc/2, z_offset),
                        (-motor_bracket_cc/2, z_offset)]):
            Circle(radius=3.5/2)
    extrude(amount=-20, mode=Mode.SUBTRACT)
    
show(p)

export_step(p.part, 'b-sec-angle.step')
