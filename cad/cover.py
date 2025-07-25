import cadquery as cq

dia = 150
w = 20
d = 8
th = 3
insert_r = 4.2/2

res = (cq.Workplane("XY")
       .rect(w, dia + 2 * d)
       .extrude(th)
       .edges(">Z or |Z")
       .fillet(1)
       .faces(">Z")
       .rarray(w/2, dia + d, 2, 2)
       .circle(3.2/2)
       .cutThruAll()
       .faces(">Z")
       .circle(2/2)
       .cutThruAll()
       )

show_object(res)

