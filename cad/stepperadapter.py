import cadquery as cq

res = (cq.Workplane("XY")
       .circle(3/2)
       .extrude(5)
       #.faces("<Z")
       #.circle(1.5/2)
       #.cutThruAll()
      )

show_object(res)

