from build123d import *
from ocp_vscode import *
from defs import *

# Mount for hours bearing

bearing_od = 27.5
bearing_id = 23.5
bearing_th = 4
b_crush = 0.45
flange_h = 1
z_offset = 4

worm_l, worm_dia, worm_cl = 7, 6, 2*2
gear_dia = 45

bottom = (Align.CENTER, Align.CENTER, Align.MIN)

with BuildPart() as p:
    Box(ow, ow, bearing_th + flange_h, align=bottom)
    # stud holes
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1]) as sk:
        with PolarLocations(radius=screw_radius, count=4, start_angle=45):
            Circle(6.5/2)
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
    
show(p)

export_step(p.part, 'mount-bottom.step')
