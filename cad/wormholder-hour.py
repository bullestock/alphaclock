from build123d import *
from ocp_vscode import *
from defs import *
from epilogue import *

worm_l = 7
spacer_l = 5
th = 2.5
stud_h = 3.5
stud_d = 5
stud_w = 5
w = 2*stud_w + worm_l + 2*spacer_l
d = 16
screwhole_y = 4.5
stud_y = -5.5
axle_y = -5.5
axle_z = 3.5
arm_l = 50
arm_w = 8
extra_len = 20
rr = 1

bottom = (Align.CENTER, Align.CENTER, Align.MIN)

with BuildPart() as p:
    # basic shape
    with BuildSketch():
        RectangleRounded(w, d, rr)
    extrude(amount=th)
    # extension
    with BuildSketch():
        with Locations((extra_len/2+w/2-2*rr, -d/2+stud_d/2)):
            RectangleRounded(extra_len+2*rr, stud_d, rr)
    extrude(amount=th+stud_h)
    # screw holes
    with BuildSketch():
        #with Locations(p.faces().sort_by(Axis.Z)[-1]) as sk:
            with Locations((-5, screwhole_y), (5, screwhole_y)):
                RectangleRounded(3.2, 5, 1.5)
    extrude(amount=th, mode=Mode.SUBTRACT)
    # worm cutout
    with BuildSketch():
        with Locations((0, -5, 0)):
            RectangleRounded(worm_l + 2*spacer_l, 10, 1)
    extrude(amount=th, mode=Mode.SUBTRACT)
    # studs
    with BuildSketch(Plane.XY.offset(th)):
        with Locations((-(worm_l/2 + stud_w/2 + spacer_l), stud_y, 0), 
                       ((worm_l/2 + stud_w/2 + spacer_l), stud_y, 0)):
            RectangleRounded(stud_w, stud_d, 1)
    extrude(amount=stud_h)
    # axle hole
    with BuildSketch(Plane.YZ.offset(-w)):
        with Locations((axle_y, axle_z)):
            Circle(2.1/2)
    extrude(amount=3*w, mode=Mode.SUBTRACT)
    #e = p.edges().sort_by(SortBy.LENGTH)
    #fillet([e[9], e[14], e[-1]], radius=0.15)
    #with BuildSketch(p.faces().sort_by(Axis.Z)[0]):
    #    with Locations((w/2 + arm_l/2 - 5, d/2 - stud_d/2 + 0.125)):
    #        RectangleRounded(arm_l, arm_w, 1)
    #extrude(amount=-th)
    
epilogue(p)