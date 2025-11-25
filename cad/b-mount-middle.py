from build123d import *
from ocp_vscode import *
from defs import *

# Mount for hours bearing

bearing_od = 27.5
bearing_id = 23.5
bearing_th = 4
b_crush = 0.6
flange_h = 1
hole_radius = 30

worm_l, worm_dia, worm_cl = 7, 6, 2*2
gear_dia = 45

b_offset = -7.5

bottom = (Align.CENTER, Align.CENTER, Align.MIN)
top = (Align.CENTER, Align.CENTER, Align.MAX)

with BuildPart() as p:
    with BuildSketch():
        RectangleRounded(80, 35, 5)
    extrude(amount=bearing_th + flange_h)
    # screw holes
    sd = 35
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1]) as sk:
        with Locations((-sd, 12.5), (sd, 12.5), (-sd, -12.5), (sd, -12.5)):
            Circle(3.2/2)
    extrude(amount=-(bearing_th + flange_h), mode=Mode.SUBTRACT)
    # bearing hole
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1]) as sk:
        with Locations((b_offset, 0)):
            Circle(bearing_od/2 + b_crush/2)
    extrude(amount=-bearing_th, mode=Mode.SUBTRACT)
    # through hole
    with BuildSketch():
        with Locations((b_offset, 0)):
            Circle(bearing_id/2)
    extrude(amount=bearing_th + flange_h, mode=Mode.SUBTRACT)
    # crush ribs
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1]) as sk:
        with Locations((b_offset - 2.25, 0)): # why?
            with PolarLocations(radius=bearing_od/2 + b_crush/2, count=10):
                Circle(b_crush)
    extrude(amount=-bearing_th)
    # worm gear cutout
    with BuildSketch():
        with Locations((gear_dia/2 + worm_dia/2 + b_offset, 0)):
            RectangleRounded(worm_l + worm_cl, worm_dia + worm_cl, 2)
    extrude(amount=2*bearing_th, mode=Mode.SUBTRACT)
    # insert holes
    with BuildSketch():
        with Locations((ow/2 - 5 + b_offset, 5), (ow/2 - 5 + b_offset, -5)):
            Circle(insert_r - 0.1)
    extrude(amount=bearing_th + flange_h, mode=Mode.SUBTRACT)
    
show(p)

export_step(p.part, 'mount-middle.step')
