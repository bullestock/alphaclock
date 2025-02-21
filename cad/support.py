import cadquery as cq

dia = 70
w = 20
d = 6
th = 1
h = 12
insert_r = 4.2/2

part1 = (cq.Workplane("XY")
         .circle(dia/2)
         .extrude(th)
         )

part2 = (cq.Workplane("XY")
         .rect(w, dia + 2 * d)
         .extrude(th)
         .faces(">Z")
         .rarray(1, dia + d, 1, 2)
         .rect(w, d)
         .extrude(h)
         .faces(">Z")
         .workplane()
         .rarray(w/2, dia + d, 2, 2)
         .circle(insert_r)
         .cutBlind(-5)
         
         )

res = part1.union(part2)

res = (res
         .faces(">Z")
         .workplane()
         .circle(17.2/2)
         .cutThruAll()
)
show_object(res)

