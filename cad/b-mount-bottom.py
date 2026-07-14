from build123d import *
from ocp_vscode import *
from defs import *
from epilogue import *

# Mount for hours bearing

bearing_od = 27.5 # bearing dimensions
bearing_id = 23.5
bearing_th = 4
b_crush = 0.6     # crush ribs radius
flange_h = 1      # bearing retaining flange height
total_h = bearing_th + flange_h
motor_z_offset = 6.5 # dual gears

# worm gear dimensions
worm_l, worm_dia, worm_cl = 7, 6, 2*2
# gear wheel diameter
gear_dia = 45

motor_y = gear_dia/2 + worm_dia/2
motor_x = -45

bottom = (Align.CENTER, Align.CENTER, Align.MIN)

with BuildPart() as p:
    with BuildSketch(Plane.XY):
        with Locations((0, 7.5)):
            RectangleRounded(110, ow + 2, 2)
    extrude(amount=total_h)

    # stud holes
    sxcd = 30
    sycd = 41
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1]) as sk:
        with Locations((sxcd, sycd), (-sxcd, sycd),
                       (sxcd, -sycd), (-sxcd, -sycd)):
            Circle(8.25/2)
    extrude(amount=-(total_h), mode=Mode.SUBTRACT)

    # bearing hole
    Cylinder(bearing_od/2 + b_crush/2, bearing_th, align=bottom,
             mode=Mode.SUBTRACT)
    # through hole
    Cylinder(bearing_id/2, total_h, align=bottom,
             mode=Mode.SUBTRACT)
    # crush ribs
    with BuildSketch(Plane.XY) as sk:
        with PolarLocations(radius=bearing_od/2 + b_crush/2, count=10):
            Circle(b_crush)
    extrude(amount=bearing_th)
    # insert holes, middle
    mx = 7.5
    my = 0
    sdx = 35
    sdy = 12.5
    with BuildSketch(Plane.XY) as sk:
        with Locations((mx - sdx, my - sdy), (mx - sdx, my + sdy), 
                       (mx + sdx, my + sdy), (mx + sdx, my - sdy)):
            Circle(insert_r)
    extrude(amount=10, mode=Mode.SUBTRACT)
    # insert holes, top
    mx = 0
    my = 7.5
    with BuildSketch(Plane.XY) as sk:
        with Locations((mx - sdy, my - sdx), (mx - sdy, my + sdx), 
                       (mx + sdy, my + sdx), (mx + sdy, my - sdx)):
            Circle(insert_r)
    extrude(amount=10, mode=Mode.SUBTRACT)

    # worm gear cutout
    with BuildSketch():
        with Locations((0, motor_y)):
            RectangleRounded(worm_l + worm_cl, worm_dia + worm_cl, 2)
    extrude(amount=2.5*bearing_th, mode=Mode.SUBTRACT)
    # insert holes for worm gear holder
    with BuildSketch():
        with Locations((5, ow/2 - 5), (-5, ow/2 - 5)):
            Circle(insert_r - 0.1)
    extrude(amount=10, mode=Mode.SUBTRACT)
    # motor mount holes
    with BuildSketch():
        with Locations([(motor_x, motor_y-motor_bracket_cc/2, 0), (motor_x, motor_y+motor_bracket_cc/2, 0)]):
            Circle(insert_r)
    extrude(amount=total_h, mode=Mode.SUBTRACT)
    # sensor cutout
    with BuildSketch():
        with Locations((gear_inner_dia/2+sensor_h/2, 0)):
            RectangleRounded(2*sensor_h, sensor_w, 2)
    extrude(amount=sensor_th, mode=Mode.SUBTRACT)

cutout_x = -50+2.5

with BuildPart() as cutout:
    # groove for gear
    with BuildSketch(Plane.YZ.offset(-40)):
        with Locations((25, 0)):
            Circle(6)
    extrude(amount=10)

p.part -= cutout.part

epilogue(p)