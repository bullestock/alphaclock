# %%
from build123d import *
from ocp_vscode import *
from defs import *
from epilogue import *

mot_th = 13
depth = mot_th + 3
mot_d = 19.7
offset = -0.25
width = 24
hole_cc = motor_bracket_cc
flange_extend = 18
flange_width = width + flange_extend
flange_th = 3
factor = 0.8
INSERT_R = 4/2
z_offset = 0 #1.5

with BuildPart() as o:
    with BuildSketch():
        # flange
        Rectangle(flange_width, depth)
    extrude(amount=flange_th)
    with BuildSketch(Plane.XY.offset(flange_th)):
        # basic shape
        Rectangle(width, depth)
    extrude(amount=offset + mot_d * factor - flange_th)
    # fillet
    fillet(o.edges().filter_by(Axis.Z), radius=1)
    fillet(o.edges().sort_by(Axis.Z)[-1], radius=1)
    # motor hole
    with BuildSketch(Plane.XZ.offset(depth/2)):
        with Locations([ (0, mot_d/2+offset, 0) ]):
            Circle(radius=mot_d/2)
    extrude(amount=-mot_th, mode=Mode.SUBTRACT)
    with BuildSketch(Plane.XZ.offset(depth/2)):
        with Locations([ (0, mot_d/2+2, 0) ]):
            RectangleRounded(mot_d/2, mot_d*0.8, mot_d/4-0.01)
    extrude(amount=-depth, mode=Mode.SUBTRACT)
    # screw holes
    with BuildSketch(o.faces().sort_by(Axis.Z)[0]):
        with Locations([(-hole_cc/2, z_offset, 0), (hole_cc/2, z_offset, 0)]):
            RectangleRounded(3.4, 12, 1.65)
    extrude(amount=-6, mode=Mode.SUBTRACT)

epilogue(o)