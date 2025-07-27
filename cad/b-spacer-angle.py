from build123d import *
from ocp_vscode import *
from defs import *

th = 5
cd = 60
height = th + 7
width = ow
z_offset = -3 # insert holes

bottom = (Align.CENTER, Align.CENTER, Align.MIN)

with BuildPart() as p:
    Box(width, width, th, align=bottom)
    # round cutout
    Cylinder(cd/2 + 2, th, mode=Mode.SUBTRACT, align=bottom)
    # 4 screw holes
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1]) as sk:
        with PolarLocations(radius=screw_radius, count=4, start_angle=45):
            Circle(3.2/2)
    extrude(amount=-th, mode=Mode.SUBTRACT)
    # flange
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1].offset(-th)):
        with Locations((0, width/2 + th/2)):
            Rectangle(width, th)
    extrude(amount=height)
    # insert holes
    with BuildSketch(p.faces().sort_by(Axis.Y)[-1]) as sk:
        with Locations([(motor_bracket_cc/2, z_offset),
                        (-motor_bracket_cc/2, z_offset)]):
            Circle(radius=insert_r-0.2)
    extrude(amount=-5, mode=Mode.SUBTRACT)
    
show(p)

export_step(p.part, 'b-spacer-angle.step')
