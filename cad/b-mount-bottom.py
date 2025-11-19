from build123d import *
from ocp_vscode import *
from defs import *

# Mount for hours bearing

bearing_od = 27.5
bearing_id = 23.5
bearing_th = 4
b_crush = 0.6
flange_h = 1
base_h = 2
base_extra_x = 30
base_extra_y = 10
z_offset = 4

worm_l, worm_dia, worm_cl = 7, 6, 2*2
gear_dia = 45

bottom = (Align.CENTER, Align.CENTER, Align.MIN)
top = (Align.CENTER, Align.CENTER, Align.MAX)

with BuildPart() as p:
    Box(ow + 2, ow + 2, bearing_th + flange_h, align=bottom)
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1]) as sk:
        RectangleRounded(ow + base_extra_x, ow + base_extra_y, 5)
    extrude(amount=-base_h)
    # stud holes
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1]) as sk:
        with PolarLocations(radius=screw_radius, count=4, start_angle=45):
            Circle(8.25/2)
    extrude(amount=-(bearing_th + flange_h), mode=Mode.SUBTRACT)
    # bearing hole
    Cylinder(bearing_od/2 + b_crush/2, bearing_th, align=bottom,
             mode=Mode.SUBTRACT)
    # through hole
    Cylinder(bearing_id/2, bearing_th + flange_h, align=bottom,
             mode=Mode.SUBTRACT)
    # crush ribs
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1]) as sk:
        with PolarLocations(radius=bearing_od/2 + b_crush/2, count=10):
            Circle(b_crush)
    extrude(amount=-bearing_th)
    # worm gear cutout
    with BuildSketch():
        with Locations((0, gear_dia/2 + worm_dia/2)):
            RectangleRounded(worm_l + worm_cl, worm_dia + worm_cl, 2)
    extrude(amount=2.5*bearing_th, mode=Mode.SUBTRACT)
    # insert holes
    with BuildSketch():
        with Locations((5, ow/2 - 5), (-5, ow/2 - 5)):
            Circle(insert_r - 0.1)
    extrude(amount=bearing_th + flange_h, mode=Mode.SUBTRACT)
    
show(p)

export_step(p.part, 'mount-bottom.step')
