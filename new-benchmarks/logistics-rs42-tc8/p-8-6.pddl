


(define (problem logistics-c3-s8-p6-a1)
(:domain logistics)
(:objects a0 
          c0 c1 c2 
          t0 t1 t2 
          l00 l01 l02 l03 l04 l05 l06 l07 l10 l11 l12 l13 l14 l15 l16 l17 l20 l21 l22 l23 l24 l25 l26 l27 
          p0 p1 p2 p3 p4 p5 
)
(:init
(allowed_to_remove l00 l03)
(allowed_to_remove l00 l07)
(allowed_to_remove l03 l04)
(allowed_to_remove l04 l05)
(allowed_to_remove l15 l16)
(allowed_to_remove l20 l23)
(allowed_to_remove l22 l23)
(allowed_to_remove l26 l27)
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
    (LOCATION l05)
    (in-city  l05 c0)
    (LOCATION l06)
    (in-city  l06 c0)
    (LOCATION l07)
    (in-city  l07 c0)
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
    (LOCATION l15)
    (in-city  l15 c1)
    (LOCATION l16)
    (in-city  l16 c1)
    (LOCATION l17)
    (in-city  l17 c1)
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
    (LOCATION l25)
    (in-city  l25 c2)
    (LOCATION l26)
    (in-city  l26 c2)
    (LOCATION l27)
    (in-city  l27 c2)
    (AIRPORT l00)
    (AIRPORT l10)
    (AIRPORT l20)
    (PACKAGE p0)
    (PACKAGE p1)
    (PACKAGE p2)
    (PACKAGE p3)
    (PACKAGE p4)
    (PACKAGE p5)
    (at t0 l07)
    (at t1 l16)
    (at t2 l21)
    (at p0 l11)
    (at p1 l12)
    (at p2 l01)
    (at p3 l12)
    (at p4 l12)
    (at p5 l13)
    (at a0 l10)
)
(:goal
    (and
        (at p0 l14)
        (at p1 l04)
        (at p2 l13)
        (at p3 l27)
        (at p4 l06)
        (at p5 l26)
    )
)
)


