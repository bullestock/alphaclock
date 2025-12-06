# %%
from build123d import *
from ocp_vscode import *
from defs import *

th = 5

dx1 = 12
dy1 = 67
yo1 = 16
dx2 = 12.5
dy2 = 22
mmdx = 26
mmdy = 15
mount_h = 20
hole_cc = 25
hole_x = 0
hole_y = 0
motor_y = 42
motor_x = -17
motor_z = 13
mot_d = 19.7

with BuildPart() as p:
    # basic shape
    with BuildSketch(Plane.XY):
        with Locations((0, yo1)):
            RectangleRounded(dx1, dy1, 4)
        with Locations((-3, 38.5)):
            RectangleRounded(dx2, dy2, 2)
    extrude(amount=th)
    # screw holes
    with BuildSketch(Plane.XY):
        with Locations([(hole_x, hole_y - hole_cc/2), (hole_x, hole_y + hole_cc/2)]):
            RectangleRounded(5, 3.2, 1.5)
        # insert hole
        with Locations([(hole_x, hole_y)]):
            Circle(radius=insert_r)
    extrude(amount=th, mode=Mode.SUBTRACT)
    # motor mount
    with BuildSketch(Plane.XY):
        with Locations((motor_x, motor_y)):
            RectangleRounded(mmdx, mmdy, 2)
    extrude(amount=mount_h)
    # motor hole
    with BuildSketch(Plane.XZ):
        with Locations([ (motor_x, motor_z) ]):
            Circle(radius=mot_d/2)
    extrude(amount=-100, mode=Mode.SUBTRACT)
    

show(p)
export_step(p.part, f"minstepperholder.step")
