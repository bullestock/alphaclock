from build123d import *
from ocp_vscode import *
from defs import *

bearing_od = 27.25
bearing_th = 4
b_crush = 0.45
flange_w = 5
flange_h = 22
z_offset = 4

bottom = (Align.CENTER, Align.CENTER, Align.MIN)

with BuildPart() as p:
    Box(ow, ow, bearing_th, align=bottom)
    Cylinder(bearing_od/2 + b_crush/2, bearing_th, align=bottom,
             mode=Mode.SUBTRACT)
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1]) as sk:
        with PolarLocations(radius=bearing_od/2 + b_crush/2, count=10):
            Circle(b_crush)
    extrude(amount=-bearing_th)
    # stud holes
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1]) as sk:
        with PolarLocations(radius=screw_radius, count=4, start_angle=45):
            Circle(6.5/2)
    extrude(amount=-bearing_th, mode=Mode.SUBTRACT)
    with BuildSketch():
        with Locations((0, (ow + flange_w)/2)):
            Rectangle(ow, flange_w)
    extrude(amount=bearing_th)
    with BuildSketch():
        with Locations((0, ow/2 + flange_w)):
            Rectangle(ow, bearing_th)
    extrude(amount=flange_h)
    with BuildSketch():
        with Locations((0, ow/2 + flange_w)):
            Rectangle(ow/2, bearing_th*3)
    extrude(amount=2.5*bearing_th, mode=Mode.SUBTRACT)
    # insert holes
    with BuildSketch(p.faces().sort_by(Axis.Y)[-1]) as sk:
        with Locations([(motor_bracket_cc/2, -z_offset),
                        (-motor_bracket_cc/2, -z_offset)]):
            Circle(radius=insert_r-0.2)
    extrude(amount=-5, mode=Mode.SUBTRACT)
    
show(p)

export_step(p.part, 'mount-bottom.step')
