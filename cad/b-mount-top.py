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
x_size = 80

b_offset = -7.5

bottom = (Align.CENTER, Align.CENTER, Align.MIN)
top = (Align.CENTER, Align.CENTER, Align.MAX)

with BuildPart() as p:
    with BuildSketch(Plane.XY):
        RectangleRounded(x_size, 35, 5)
    extrude(amount=bearing_th + flange_h)
    sd = 35
    with BuildSketch(Plane.XY) as sk:
        # screw holes
        with Locations((sd, 12.5), (sd, -12.5)):
            Circle(3.2/2)
        with Locations((sd, 0)):
            RectangleRounded(6, 3.2, 1.5)
    extrude(amount=(bearing_th + flange_h), mode=Mode.SUBTRACT)
    # bearing hole
    with BuildSketch(Plane.XY) as sk:
        with Locations((b_offset, 0)):
            Circle(bearing_od/2 + b_crush/2)
    extrude(amount=bearing_th, mode=Mode.SUBTRACT)
    # through hole
    with BuildSketch(Plane.XY):
        with Locations((b_offset, 0)):
            Circle(bearing_id/2)
    extrude(amount=bearing_th + flange_h, mode=Mode.SUBTRACT)
    # crush ribs
    with BuildSketch(Plane.XY) as sk:
        with Locations((b_offset, 0)):
            with PolarLocations(radius=bearing_od/2 + b_crush/2, count=10):
                Circle(b_crush)
    extrude(amount=bearing_th)
    # worm gear cutout
    with BuildSketch():
        with Locations((gear_dia/2 + worm_dia/2 + b_offset, 0)):
            RectangleRounded(worm_l + worm_cl, worm_dia + worm_cl, 2)
    extrude(amount=2*bearing_th, mode=Mode.SUBTRACT)
    # insert holes
    with BuildSketch():
        with Locations((ow/2 - 5 + b_offset, 5), (ow/2 - 5 + b_offset, -5)):
            Circle(insert_r)
    extrude(amount=bearing_th + flange_h, mode=Mode.SUBTRACT)
    
show(p)

export_step(p.part, 'mount-middle.step')
