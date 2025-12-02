# %%
from build123d import *
from ocp_vscode import *
from defs import *

th = 5

dx1 = 27.5
dy1 = 40
dx2 = 10
dy2 = 57
mmdx = 10
mmdy = 26
mount_h = 20
hole_cc = 25
hole_x = 5
hole_y = -dy1/2
motor_y = -dy1/2 - hole_cc/2 -11.5
motor_z = 16.75
mot_d = 19.7
spacer_h = 8

with BuildPart() as p:
    # basic shape
    with BuildSketch(Plane.XY):
        with Locations((dx1/2, -dy1/2)):
            RectangleRounded(dx1, dy1, 2)
        with Locations((dx2/2 + 17.5, -dy2/2)):
            RectangleRounded(dx2, dy2, 2)
    extrude(amount=th)
    # spacers
    with BuildSketch(Plane.XY):
        with Locations([(hole_x, hole_y - hole_cc/2), (hole_x, hole_y + hole_cc/2)]):
            RectangleRounded(8, 12, 3)
    extrude(amount=spacer_h)
    # insert holes
    with BuildSketch(Plane.XY):
        with Locations([(hole_x, hole_y - hole_cc/2), (hole_x, hole_y + hole_cc/2)]):
            Circle(radius=insert_r)
    extrude(amount=spacer_h, mode=Mode.SUBTRACT)
    # motor mount
    with BuildSketch(Plane.XY):
        with Locations((dx1 - mmdx/2, motor_y)):
            RectangleRounded(mmdx, mmdy, 2)
    extrude(amount=mount_h)
    # motor hole
    with BuildSketch(Plane.YZ):
        with Locations([ (motor_y, motor_z) ]):
            Circle(radius=mot_d/2)
    extrude(amount=50, mode=Mode.SUBTRACT)
    # clearance
    with BuildSketch(Plane.XY):
        with Locations([ (-45/2 + 3, -dy1/2) ]):
            Circle(radius=45/2)
    extrude(amount=50, mode=Mode.SUBTRACT)
    

show(p)
export_step(p.part, f"minstepperholder.step")
