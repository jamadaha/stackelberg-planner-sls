


(define (problem logistics-c3-s5-p9-a1)
(:domain logistics)
(:objects a0 
          c0 c1 c2 
          t0 t1 t2 
          l00 l01 l02 l03 l04 l10 l11 l12 l13 l14 l20 l21 l22 l23 l24 
          p0 p1 p2 p3 p4 p5 p6 p7 p8 
)
(:init
(allowed_to_remove l00 l01)
(allowed_to_remove l01 l04)
(allowed_to_remove l02 l04)
(allowed_to_remove l13 l14)
(allowed_to_remove l20 l23)
    (AIRPLANE a0)
    (CITY c0)
    (CITY c1)
    (CITY c2)
    (TRUCK t0)
    (TRUCK t1)
    (TRUCK t2)
    (LOCATION l00)
    (in-city  l00 c0)
    (LOCATION l01)
    (in-city  l01 c0)
    (LOCATION l02)
    (in-city  l02 c0)
    (LOCATION l03)
    (in-city  l03 c0)
    (LOCATION l04)
    (in-city  l04 c0)
    (LOCATION l10)
    (in-city  l10 c1)
    (LOCATION l11)
    (in-city  l11 c1)
    (LOCATION l12)
    (in-city  l12 c1)
    (LOCATION l13)
    (in-city  l13 c1)
    (LOCATION l14)
    (in-city  l14 c1)
    (LOCATION l20)
    (in-city  l20 c2)
    (LOCATION l21)
    (in-city  l21 c2)
    (LOCATION l22)
    (in-city  l22 c2)
    (LOCATION l23)
    (in-city  l23 c2)
    (LOCATION l24)
    (in-city  l24 c2)
    (AIRPORT l00)
    (AIRPORT l10)
    (AIRPORT l20)
    (PACKAGE p0)
    (PACKAGE p1)
    (PACKAGE p2)
    (PACKAGE p3)
    (PACKAGE p4)
    (PACKAGE p5)
    (PACKAGE p6)
    (PACKAGE p7)
    (PACKAGE p8)
    (at t0 l03)
    (at t1 l11)
    (at t2 l22)
    (at p0 l13)
    (at p1 l11)
    (at p2 l04)
    (at p3 l12)
    (at p4 l10)
    (at p5 l13)
    (at p6 l10)
    (at p7 l02)
    (at p8 l11)
    (at a0 l10)
)
(:goal
    (and
        (at p0 l22)
        (at p1 l02)
        (at p2 l22)
        (at p3 l12)
        (at p4 l14)
        (at p5 l02)
        (at p6 l04)
        (at p7 l03)
        (at p8 l11)
    )
)
)


