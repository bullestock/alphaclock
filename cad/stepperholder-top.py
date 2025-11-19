# %%
from build123d import *
from ocp_vscode import *
from defs import *

depth = 10
mot_d = 19.7
offset = 2.5
width = 26
hole_cc = motor_bracket_cc
flange_extend = 18
flange_width = width + flange_extend
flange_th = 5
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
    extrude(amount=mot_d+2)
    # fillet
    fillet(o.edges().filter_by(Axis.Z), radius=1)
    fillet(o.edges().sort_by(Axis.Z)[-1], radius=1)
    # screw holes
    with BuildSketch(o.faces().sort_by(Axis.Z)[0]):
        with Locations([(-hole_cc/2, z_offset, 0), (hole_cc/2, z_offset, 0)]):
            RectangleRounded(3.4, 7, 1.65)
    extrude(amount=-6, mode=Mode.SUBTRACT)
    # motor hole
    with BuildSketch(o.faces().sort_by(Axis.Y)[0]) as h:
        with Locations([ (0, -1.5-2) ]):
            Circle(radius=mot_d/2)
    extrude(amount=-depth, mode=Mode.SUBTRACT)
    # wire hole
    with BuildSketch(o.faces().sort_by(Axis.Y)[0]) as h:
        with Locations([ (0, 5) ]):
            RectangleRounded(16, 10, 1)
    extrude(amount=-depth, mode=Mode.SUBTRACT)
    

show(o)    
export_step(o.part, f"stepperholder-top.step")
