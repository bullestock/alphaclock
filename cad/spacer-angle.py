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
    # bearing cutout
    Cylinder(bearing_od/2 + b_crush/2, th, mode=Mode.SUBTRACT, align=bottom)
    # 4 screw holes
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1]) as sk:
        with PolarLocations(radius=screw_radius, count=4, start_angle=45):
            Circle(3.2/2)
    extrude(amount=-th, mode=Mode.SUBTRACT)
    # crush ribs
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1]) as sk:
        with PolarLocations(radius=bearing_od/2 + b_crush/2, count=10):
            Circle(b_crush)
    extrude(amount=-bearing_th)
    # access holes
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1]) as sk:
        with PolarLocations(radius=ow*0.4, count=4, start_angle=45):
            Circle(20/2)
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1]) as sk:
        with PolarLocations(radius=ow*0.35, count=4):
            Circle(15/2)
    extrude(amount=-th, mode=Mode.SUBTRACT)
    # side screw holes
    with BuildSketch(p.faces().sort_by(Axis.X)[0]) as sk:
        with PolarLocations(radius=motor_bracket_cc/2, count=2, start_angle=90):
            Circle(radius=3/2)
    extrude(amount=-ow, mode=Mode.SUBTRACT)
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

export_step(p.part, 'spacer-angle.step')
