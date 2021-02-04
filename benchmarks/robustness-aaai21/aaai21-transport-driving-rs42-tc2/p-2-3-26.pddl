; Transport two-cities-sequential-26nodes-1000size-5degree-100mindistance-2trucks-3packages-42seed

(define (problem transport-two-cities-sequential-26nodes-1000size-5degree-100mindistance-2trucks-3packages-42seed)
 (:domain transport)
 (:objects
  city-1-loc-1 - location
  city-2-loc-1 - location
  city-1-loc-2 - location
  city-2-loc-2 - location
  city-1-loc-3 - location
  city-2-loc-3 - location
  city-1-loc-4 - location
  city-2-loc-4 - location
  city-1-loc-5 - location
  city-2-loc-5 - location
  city-1-loc-6 - location
  city-2-loc-6 - location
  city-1-loc-7 - location
  city-2-loc-7 - location
  city-1-loc-8 - location
  city-2-loc-8 - location
  city-1-loc-9 - location
  city-2-loc-9 - location
  city-1-loc-10 - location
  city-2-loc-10 - location
  city-1-loc-11 - location
  city-2-loc-11 - location
  city-1-loc-12 - location
  city-2-loc-12 - location
  city-1-loc-13 - location
  city-2-loc-13 - location
  city-1-loc-14 - location
  city-2-loc-14 - location
  city-1-loc-15 - location
  city-2-loc-15 - location
  city-1-loc-16 - location
  city-2-loc-16 - location
  city-1-loc-17 - location
  city-2-loc-17 - location
  city-1-loc-18 - location
  city-2-loc-18 - location
  city-1-loc-19 - location
  city-2-loc-19 - location
  city-1-loc-20 - location
  city-2-loc-20 - location
  city-1-loc-21 - location
  city-2-loc-21 - location
  city-1-loc-22 - location
  city-2-loc-22 - location
  city-1-loc-23 - location
  city-2-loc-23 - location
  city-1-loc-24 - location
  city-2-loc-24 - location
  city-1-loc-25 - location
  city-2-loc-25 - location
  city-1-loc-26 - location
  city-2-loc-26 - location
  truck-1 - vehicle
  truck-2 - vehicle
  package-1 - package
  package-2 - package
  package-3 - package
  capacity-0 - capacity-number
  capacity-1 - capacity-number
  capacity-2 - capacity-number
  capacity-3 - capacity-number
  capacity-4 - capacity-number
  truck_f - fix_vehicle
 )
 (:init
(allowed_to_remove city-1-loc-1 city-1-loc-9)
(allowed_to_remove city-2-loc-12 city-2-loc-8)
  (= (total-cost) 0)
  (capacity-predecessor capacity-0 capacity-1)
  (capacity-predecessor capacity-1 capacity-2)
  (capacity-predecessor capacity-2 capacity-3)
  (capacity-predecessor capacity-3 capacity-4)
  ; 228,142 -> 281,250
  (road city-1-loc-4 city-1-loc-3)
  (= (road-length city-1-loc-4 city-1-loc-3) 12)
  ; 281,250 -> 228,142
  (road city-1-loc-3 city-1-loc-4)
  (= (road-length city-1-loc-3 city-1-loc-4) 12)
  ; 754,104 -> 654,114
  (road city-1-loc-5 city-1-loc-1)
  (= (road-length city-1-loc-5 city-1-loc-1) 10)
  ; 654,114 -> 754,104
  (road city-1-loc-1 city-1-loc-5)
  (= (road-length city-1-loc-1 city-1-loc-5) 10)
  ; 89,604 -> 25,759
  (road city-1-loc-8 city-1-loc-2)
  (= (road-length city-1-loc-8 city-1-loc-2) 17)
  ; 25,759 -> 89,604
  (road city-1-loc-2 city-1-loc-8)
  (= (road-length city-1-loc-2 city-1-loc-8) 17)
  ; 432,32 -> 654,114
  (road city-1-loc-9 city-1-loc-1)
  (= (road-length city-1-loc-9 city-1-loc-1) 24)
  ; 654,114 -> 432,32
  (road city-1-loc-1 city-1-loc-9)
  (= (road-length city-1-loc-1 city-1-loc-9) 24)
  ; 432,32 -> 281,250
  (road city-1-loc-9 city-1-loc-3)
  (= (road-length city-1-loc-9 city-1-loc-3) 27)
  ; 281,250 -> 432,32
  (road city-1-loc-3 city-1-loc-9)
  (= (road-length city-1-loc-3 city-1-loc-9) 27)
  ; 432,32 -> 228,142
  (road city-1-loc-9 city-1-loc-4)
  (= (road-length city-1-loc-9 city-1-loc-4) 24)
  ; 228,142 -> 432,32
  (road city-1-loc-4 city-1-loc-9)
  (= (road-length city-1-loc-4 city-1-loc-9) 24)
  ; 30,95 -> 281,250
  (road city-1-loc-10 city-1-loc-3)
  (= (road-length city-1-loc-10 city-1-loc-3) 30)
  ; 281,250 -> 30,95
  (road city-1-loc-3 city-1-loc-10)
  (= (road-length city-1-loc-3 city-1-loc-10) 30)
  ; 30,95 -> 228,142
  (road city-1-loc-10 city-1-loc-4)
  (= (road-length city-1-loc-10 city-1-loc-4) 21)
  ; 228,142 -> 30,95
  (road city-1-loc-4 city-1-loc-10)
  (= (road-length city-1-loc-4 city-1-loc-10) 21)
  ; 517,616 -> 692,758
  (road city-1-loc-11 city-1-loc-6)
  (= (road-length city-1-loc-11 city-1-loc-6) 23)
  ; 692,758 -> 517,616
  (road city-1-loc-6 city-1-loc-11)
  (= (road-length city-1-loc-6 city-1-loc-11) 23)
  ; 203,733 -> 25,759
  (road city-1-loc-12 city-1-loc-2)
  (= (road-length city-1-loc-12 city-1-loc-2) 18)
  ; 25,759 -> 203,733
  (road city-1-loc-2 city-1-loc-12)
  (= (road-length city-1-loc-2 city-1-loc-12) 18)
  ; 203,733 -> 89,604
  (road city-1-loc-12 city-1-loc-8)
  (= (road-length city-1-loc-12 city-1-loc-8) 18)
  ; 89,604 -> 203,733
  (road city-1-loc-8 city-1-loc-12)
  (= (road-length city-1-loc-8 city-1-loc-12) 18)
  ; 558,429 -> 517,616
  (road city-1-loc-13 city-1-loc-11)
  (= (road-length city-1-loc-13 city-1-loc-11) 20)
  ; 517,616 -> 558,429
  (road city-1-loc-11 city-1-loc-13)
  (= (road-length city-1-loc-11 city-1-loc-13) 20)
  ; 225,459 -> 281,250
  (road city-1-loc-14 city-1-loc-3)
  (= (road-length city-1-loc-14 city-1-loc-3) 22)
  ; 281,250 -> 225,459
  (road city-1-loc-3 city-1-loc-14)
  (= (road-length city-1-loc-3 city-1-loc-14) 22)
  ; 225,459 -> 89,604
  (road city-1-loc-14 city-1-loc-8)
  (= (road-length city-1-loc-14 city-1-loc-8) 20)
  ; 89,604 -> 225,459
  (road city-1-loc-8 city-1-loc-14)
  (= (road-length city-1-loc-8 city-1-loc-14) 20)
  ; 225,459 -> 203,733
  (road city-1-loc-14 city-1-loc-12)
  (= (road-length city-1-loc-14 city-1-loc-12) 28)
  ; 203,733 -> 225,459
  (road city-1-loc-12 city-1-loc-14)
  (= (road-length city-1-loc-12 city-1-loc-14) 28)
  ; 603,284 -> 654,114
  (road city-1-loc-15 city-1-loc-1)
  (= (road-length city-1-loc-15 city-1-loc-1) 18)
  ; 654,114 -> 603,284
  (road city-1-loc-1 city-1-loc-15)
  (= (road-length city-1-loc-1 city-1-loc-15) 18)
  ; 603,284 -> 754,104
  (road city-1-loc-15 city-1-loc-5)
  (= (road-length city-1-loc-15 city-1-loc-5) 24)
  ; 754,104 -> 603,284
  (road city-1-loc-5 city-1-loc-15)
  (= (road-length city-1-loc-5 city-1-loc-15) 24)
  ; 603,284 -> 558,429
  (road city-1-loc-15 city-1-loc-13)
  (= (road-length city-1-loc-15 city-1-loc-13) 16)
  ; 558,429 -> 603,284
  (road city-1-loc-13 city-1-loc-15)
  (= (road-length city-1-loc-13 city-1-loc-15) 16)
  ; 828,890 -> 692,758
  (road city-1-loc-16 city-1-loc-6)
  (= (road-length city-1-loc-16 city-1-loc-6) 19)
  ; 692,758 -> 828,890
  (road city-1-loc-6 city-1-loc-16)
  (= (road-length city-1-loc-6 city-1-loc-16) 19)
  ; 714,432 -> 913,558
  (road city-1-loc-17 city-1-loc-7)
  (= (road-length city-1-loc-17 city-1-loc-7) 24)
  ; 913,558 -> 714,432
  (road city-1-loc-7 city-1-loc-17)
  (= (road-length city-1-loc-7 city-1-loc-17) 24)
  ; 714,432 -> 517,616
  (road city-1-loc-17 city-1-loc-11)
  (= (road-length city-1-loc-17 city-1-loc-11) 27)
  ; 517,616 -> 714,432
  (road city-1-loc-11 city-1-loc-17)
  (= (road-length city-1-loc-11 city-1-loc-17) 27)
  ; 714,432 -> 558,429
  (road city-1-loc-17 city-1-loc-13)
  (= (road-length city-1-loc-17 city-1-loc-13) 16)
  ; 558,429 -> 714,432
  (road city-1-loc-13 city-1-loc-17)
  (= (road-length city-1-loc-13 city-1-loc-17) 16)
  ; 714,432 -> 603,284
  (road city-1-loc-17 city-1-loc-15)
  (= (road-length city-1-loc-17 city-1-loc-15) 19)
  ; 603,284 -> 714,432
  (road city-1-loc-15 city-1-loc-17)
  (= (road-length city-1-loc-15 city-1-loc-17) 19)
  ; 159,220 -> 281,250
  (road city-1-loc-18 city-1-loc-3)
  (= (road-length city-1-loc-18 city-1-loc-3) 13)
  ; 281,250 -> 159,220
  (road city-1-loc-3 city-1-loc-18)
  (= (road-length city-1-loc-3 city-1-loc-18) 13)
  ; 159,220 -> 228,142
  (road city-1-loc-18 city-1-loc-4)
  (= (road-length city-1-loc-18 city-1-loc-4) 11)
  ; 228,142 -> 159,220
  (road city-1-loc-4 city-1-loc-18)
  (= (road-length city-1-loc-4 city-1-loc-18) 11)
  ; 159,220 -> 30,95
  (road city-1-loc-18 city-1-loc-10)
  (= (road-length city-1-loc-18 city-1-loc-10) 18)
  ; 30,95 -> 159,220
  (road city-1-loc-10 city-1-loc-18)
  (= (road-length city-1-loc-10 city-1-loc-18) 18)
  ; 159,220 -> 225,459
  (road city-1-loc-18 city-1-loc-14)
  (= (road-length city-1-loc-18 city-1-loc-14) 25)
  ; 225,459 -> 159,220
  (road city-1-loc-14 city-1-loc-18)
  (= (road-length city-1-loc-14 city-1-loc-18) 25)
  ; 980,781 -> 692,758
  (road city-1-loc-19 city-1-loc-6)
  (= (road-length city-1-loc-19 city-1-loc-6) 29)
  ; 692,758 -> 980,781
  (road city-1-loc-6 city-1-loc-19)
  (= (road-length city-1-loc-6 city-1-loc-19) 29)
  ; 980,781 -> 913,558
  (road city-1-loc-19 city-1-loc-7)
  (= (road-length city-1-loc-19 city-1-loc-7) 24)
  ; 913,558 -> 980,781
  (road city-1-loc-7 city-1-loc-19)
  (= (road-length city-1-loc-7 city-1-loc-19) 24)
  ; 980,781 -> 828,890
  (road city-1-loc-19 city-1-loc-16)
  (= (road-length city-1-loc-19 city-1-loc-16) 19)
  ; 828,890 -> 980,781
  (road city-1-loc-16 city-1-loc-19)
  (= (road-length city-1-loc-16 city-1-loc-19) 19)
  ; 344,104 -> 281,250
  (road city-1-loc-20 city-1-loc-3)
  (= (road-length city-1-loc-20 city-1-loc-3) 16)
  ; 281,250 -> 344,104
  (road city-1-loc-3 city-1-loc-20)
  (= (road-length city-1-loc-3 city-1-loc-20) 16)
  ; 344,104 -> 228,142
  (road city-1-loc-20 city-1-loc-4)
  (= (road-length city-1-loc-20 city-1-loc-4) 13)
  ; 228,142 -> 344,104
  (road city-1-loc-4 city-1-loc-20)
  (= (road-length city-1-loc-4 city-1-loc-20) 13)
  ; 344,104 -> 432,32
  (road city-1-loc-20 city-1-loc-9)
  (= (road-length city-1-loc-20 city-1-loc-9) 12)
  ; 432,32 -> 344,104
  (road city-1-loc-9 city-1-loc-20)
  (= (road-length city-1-loc-9 city-1-loc-20) 12)
  ; 344,104 -> 159,220
  (road city-1-loc-20 city-1-loc-18)
  (= (road-length city-1-loc-20 city-1-loc-18) 22)
  ; 159,220 -> 344,104
  (road city-1-loc-18 city-1-loc-20)
  (= (road-length city-1-loc-18 city-1-loc-20) 22)
  ; 94,389 -> 281,250
  (road city-1-loc-21 city-1-loc-3)
  (= (road-length city-1-loc-21 city-1-loc-3) 24)
  ; 281,250 -> 94,389
  (road city-1-loc-3 city-1-loc-21)
  (= (road-length city-1-loc-3 city-1-loc-21) 24)
  ; 94,389 -> 228,142
  (road city-1-loc-21 city-1-loc-4)
  (= (road-length city-1-loc-21 city-1-loc-4) 29)
  ; 228,142 -> 94,389
  (road city-1-loc-4 city-1-loc-21)
  (= (road-length city-1-loc-4 city-1-loc-21) 29)
  ; 94,389 -> 89,604
  (road city-1-loc-21 city-1-loc-8)
  (= (road-length city-1-loc-21 city-1-loc-8) 22)
  ; 89,604 -> 94,389
  (road city-1-loc-8 city-1-loc-21)
  (= (road-length city-1-loc-8 city-1-loc-21) 22)
  ; 94,389 -> 225,459
  (road city-1-loc-21 city-1-loc-14)
  (= (road-length city-1-loc-21 city-1-loc-14) 15)
  ; 225,459 -> 94,389
  (road city-1-loc-14 city-1-loc-21)
  (= (road-length city-1-loc-14 city-1-loc-21) 15)
  ; 94,389 -> 159,220
  (road city-1-loc-21 city-1-loc-18)
  (= (road-length city-1-loc-21 city-1-loc-18) 19)
  ; 159,220 -> 94,389
  (road city-1-loc-18 city-1-loc-21)
  (= (road-length city-1-loc-18 city-1-loc-21) 19)
  ; 867,352 -> 754,104
  (road city-1-loc-22 city-1-loc-5)
  (= (road-length city-1-loc-22 city-1-loc-5) 28)
  ; 754,104 -> 867,352
  (road city-1-loc-5 city-1-loc-22)
  (= (road-length city-1-loc-5 city-1-loc-22) 28)
  ; 867,352 -> 913,558
  (road city-1-loc-22 city-1-loc-7)
  (= (road-length city-1-loc-22 city-1-loc-7) 22)
  ; 913,558 -> 867,352
  (road city-1-loc-7 city-1-loc-22)
  (= (road-length city-1-loc-7 city-1-loc-22) 22)
  ; 867,352 -> 603,284
  (road city-1-loc-22 city-1-loc-15)
  (= (road-length city-1-loc-22 city-1-loc-15) 28)
  ; 603,284 -> 867,352
  (road city-1-loc-15 city-1-loc-22)
  (= (road-length city-1-loc-15 city-1-loc-22) 28)
  ; 867,352 -> 714,432
  (road city-1-loc-22 city-1-loc-17)
  (= (road-length city-1-loc-22 city-1-loc-17) 18)
  ; 714,432 -> 867,352
  (road city-1-loc-17 city-1-loc-22)
  (= (road-length city-1-loc-17 city-1-loc-22) 18)
  ; 549,127 -> 654,114
  (road city-1-loc-23 city-1-loc-1)
  (= (road-length city-1-loc-23 city-1-loc-1) 11)
  ; 654,114 -> 549,127
  (road city-1-loc-1 city-1-loc-23)
  (= (road-length city-1-loc-1 city-1-loc-23) 11)
  ; 549,127 -> 281,250
  (road city-1-loc-23 city-1-loc-3)
  (= (road-length city-1-loc-23 city-1-loc-3) 30)
  ; 281,250 -> 549,127
  (road city-1-loc-3 city-1-loc-23)
  (= (road-length city-1-loc-3 city-1-loc-23) 30)
  ; 549,127 -> 754,104
  (road city-1-loc-23 city-1-loc-5)
  (= (road-length city-1-loc-23 city-1-loc-5) 21)
  ; 754,104 -> 549,127
  (road city-1-loc-5 city-1-loc-23)
  (= (road-length city-1-loc-5 city-1-loc-23) 21)
  ; 549,127 -> 432,32
  (road city-1-loc-23 city-1-loc-9)
  (= (road-length city-1-loc-23 city-1-loc-9) 16)
  ; 432,32 -> 549,127
  (road city-1-loc-9 city-1-loc-23)
  (= (road-length city-1-loc-9 city-1-loc-23) 16)
  ; 549,127 -> 603,284
  (road city-1-loc-23 city-1-loc-15)
  (= (road-length city-1-loc-23 city-1-loc-15) 17)
  ; 603,284 -> 549,127
  (road city-1-loc-15 city-1-loc-23)
  (= (road-length city-1-loc-15 city-1-loc-23) 17)
  ; 549,127 -> 344,104
  (road city-1-loc-23 city-1-loc-20)
  (= (road-length city-1-loc-23 city-1-loc-20) 21)
  ; 344,104 -> 549,127
  (road city-1-loc-20 city-1-loc-23)
  (= (road-length city-1-loc-20 city-1-loc-23) 21)
  ; 996,944 -> 828,890
  (road city-1-loc-24 city-1-loc-16)
  (= (road-length city-1-loc-24 city-1-loc-16) 18)
  ; 828,890 -> 996,944
  (road city-1-loc-16 city-1-loc-24)
  (= (road-length city-1-loc-16 city-1-loc-24) 18)
  ; 996,944 -> 980,781
  (road city-1-loc-24 city-1-loc-19)
  (= (road-length city-1-loc-24 city-1-loc-19) 17)
  ; 980,781 -> 996,944
  (road city-1-loc-19 city-1-loc-24)
  (= (road-length city-1-loc-19 city-1-loc-24) 17)
  ; 849,643 -> 692,758
  (road city-1-loc-25 city-1-loc-6)
  (= (road-length city-1-loc-25 city-1-loc-6) 20)
  ; 692,758 -> 849,643
  (road city-1-loc-6 city-1-loc-25)
  (= (road-length city-1-loc-6 city-1-loc-25) 20)
  ; 849,643 -> 913,558
  (road city-1-loc-25 city-1-loc-7)
  (= (road-length city-1-loc-25 city-1-loc-7) 11)
  ; 913,558 -> 849,643
  (road city-1-loc-7 city-1-loc-25)
  (= (road-length city-1-loc-7 city-1-loc-25) 11)
  ; 849,643 -> 828,890
  (road city-1-loc-25 city-1-loc-16)
  (= (road-length city-1-loc-25 city-1-loc-16) 25)
  ; 828,890 -> 849,643
  (road city-1-loc-16 city-1-loc-25)
  (= (road-length city-1-loc-16 city-1-loc-25) 25)
  ; 849,643 -> 714,432
  (road city-1-loc-25 city-1-loc-17)
  (= (road-length city-1-loc-25 city-1-loc-17) 25)
  ; 714,432 -> 849,643
  (road city-1-loc-17 city-1-loc-25)
  (= (road-length city-1-loc-17 city-1-loc-25) 25)
  ; 849,643 -> 980,781
  (road city-1-loc-25 city-1-loc-19)
  (= (road-length city-1-loc-25 city-1-loc-19) 19)
  ; 980,781 -> 849,643
  (road city-1-loc-19 city-1-loc-25)
  (= (road-length city-1-loc-19 city-1-loc-25) 19)
  ; 849,643 -> 867,352
  (road city-1-loc-25 city-1-loc-22)
  (= (road-length city-1-loc-25 city-1-loc-22) 30)
  ; 867,352 -> 849,643
  (road city-1-loc-22 city-1-loc-25)
  (= (road-length city-1-loc-22 city-1-loc-25) 30)
  ; 633,906 -> 692,758
  (road city-1-loc-26 city-1-loc-6)
  (= (road-length city-1-loc-26 city-1-loc-6) 16)
  ; 692,758 -> 633,906
  (road city-1-loc-6 city-1-loc-26)
  (= (road-length city-1-loc-6 city-1-loc-26) 16)
  ; 633,906 -> 828,890
  (road city-1-loc-26 city-1-loc-16)
  (= (road-length city-1-loc-26 city-1-loc-16) 20)
  ; 828,890 -> 633,906
  (road city-1-loc-16 city-1-loc-26)
  (= (road-length city-1-loc-16 city-1-loc-26) 20)
  ; 2721,71 -> 2591,196
  (road city-2-loc-3 city-2-loc-2)
  (= (road-length city-2-loc-3 city-2-loc-2) 18)
  ; 2591,196 -> 2721,71
  (road city-2-loc-2 city-2-loc-3)
  (= (road-length city-2-loc-2 city-2-loc-3) 18)
  ; 2233,791 -> 2046,677
  (road city-2-loc-5 city-2-loc-4)
  (= (road-length city-2-loc-5 city-2-loc-4) 22)
  ; 2046,677 -> 2233,791
  (road city-2-loc-4 city-2-loc-5)
  (= (road-length city-2-loc-4 city-2-loc-5) 22)
  ; 2875,238 -> 2882,370
  (road city-2-loc-7 city-2-loc-1)
  (= (road-length city-2-loc-7 city-2-loc-1) 14)
  ; 2882,370 -> 2875,238
  (road city-2-loc-1 city-2-loc-7)
  (= (road-length city-2-loc-1 city-2-loc-7) 14)
  ; 2875,238 -> 2591,196
  (road city-2-loc-7 city-2-loc-2)
  (= (road-length city-2-loc-7 city-2-loc-2) 29)
  ; 2591,196 -> 2875,238
  (road city-2-loc-2 city-2-loc-7)
  (= (road-length city-2-loc-2 city-2-loc-7) 29)
  ; 2875,238 -> 2721,71
  (road city-2-loc-7 city-2-loc-3)
  (= (road-length city-2-loc-7 city-2-loc-3) 23)
  ; 2721,71 -> 2875,238
  (road city-2-loc-3 city-2-loc-7)
  (= (road-length city-2-loc-3 city-2-loc-7) 23)
  ; 2887,103 -> 2882,370
  (road city-2-loc-8 city-2-loc-1)
  (= (road-length city-2-loc-8 city-2-loc-1) 27)
  ; 2882,370 -> 2887,103
  (road city-2-loc-1 city-2-loc-8)
  (= (road-length city-2-loc-1 city-2-loc-8) 27)
  ; 2887,103 -> 2721,71
  (road city-2-loc-8 city-2-loc-3)
  (= (road-length city-2-loc-8 city-2-loc-3) 17)
  ; 2721,71 -> 2887,103
  (road city-2-loc-3 city-2-loc-8)
  (= (road-length city-2-loc-3 city-2-loc-8) 17)
  ; 2887,103 -> 2875,238
  (road city-2-loc-8 city-2-loc-7)
  (= (road-length city-2-loc-8 city-2-loc-7) 14)
  ; 2875,238 -> 2887,103
  (road city-2-loc-7 city-2-loc-8)
  (= (road-length city-2-loc-7 city-2-loc-8) 14)
  ; 2389,284 -> 2591,196
  (road city-2-loc-9 city-2-loc-2)
  (= (road-length city-2-loc-9 city-2-loc-2) 22)
  ; 2591,196 -> 2389,284
  (road city-2-loc-2 city-2-loc-9)
  (= (road-length city-2-loc-2 city-2-loc-9) 22)
  ; 2389,284 -> 2296,81
  (road city-2-loc-9 city-2-loc-6)
  (= (road-length city-2-loc-9 city-2-loc-6) 23)
  ; 2296,81 -> 2389,284
  (road city-2-loc-6 city-2-loc-9)
  (= (road-length city-2-loc-6 city-2-loc-9) 23)
  ; 2464,650 -> 2233,791
  (road city-2-loc-10 city-2-loc-5)
  (= (road-length city-2-loc-10 city-2-loc-5) 28)
  ; 2233,791 -> 2464,650
  (road city-2-loc-5 city-2-loc-10)
  (= (road-length city-2-loc-5 city-2-loc-10) 28)
  ; 2166,379 -> 2389,284
  (road city-2-loc-11 city-2-loc-9)
  (= (road-length city-2-loc-11 city-2-loc-9) 25)
  ; 2389,284 -> 2166,379
  (road city-2-loc-9 city-2-loc-11)
  (= (road-length city-2-loc-9 city-2-loc-11) 25)
  ; 2686,273 -> 2882,370
  (road city-2-loc-12 city-2-loc-1)
  (= (road-length city-2-loc-12 city-2-loc-1) 22)
  ; 2882,370 -> 2686,273
  (road city-2-loc-1 city-2-loc-12)
  (= (road-length city-2-loc-1 city-2-loc-12) 22)
  ; 2686,273 -> 2591,196
  (road city-2-loc-12 city-2-loc-2)
  (= (road-length city-2-loc-12 city-2-loc-2) 13)
  ; 2591,196 -> 2686,273
  (road city-2-loc-2 city-2-loc-12)
  (= (road-length city-2-loc-2 city-2-loc-12) 13)
  ; 2686,273 -> 2721,71
  (road city-2-loc-12 city-2-loc-3)
  (= (road-length city-2-loc-12 city-2-loc-3) 21)
  ; 2721,71 -> 2686,273
  (road city-2-loc-3 city-2-loc-12)
  (= (road-length city-2-loc-3 city-2-loc-12) 21)
  ; 2686,273 -> 2875,238
  (road city-2-loc-12 city-2-loc-7)
  (= (road-length city-2-loc-12 city-2-loc-7) 20)
  ; 2875,238 -> 2686,273
  (road city-2-loc-7 city-2-loc-12)
  (= (road-length city-2-loc-7 city-2-loc-12) 20)
  ; 2686,273 -> 2887,103
  (road city-2-loc-12 city-2-loc-8)
  (= (road-length city-2-loc-12 city-2-loc-8) 27)
  ; 2887,103 -> 2686,273
  (road city-2-loc-8 city-2-loc-12)
  (= (road-length city-2-loc-8 city-2-loc-12) 27)
  ; 2686,273 -> 2389,284
  (road city-2-loc-12 city-2-loc-9)
  (= (road-length city-2-loc-12 city-2-loc-9) 30)
  ; 2389,284 -> 2686,273
  (road city-2-loc-9 city-2-loc-12)
  (= (road-length city-2-loc-9 city-2-loc-12) 30)
  ; 2699,663 -> 2464,650
  (road city-2-loc-14 city-2-loc-10)
  (= (road-length city-2-loc-14 city-2-loc-10) 24)
  ; 2464,650 -> 2699,663
  (road city-2-loc-10 city-2-loc-14)
  (= (road-length city-2-loc-10 city-2-loc-14) 24)
  ; 2699,663 -> 2718,959
  (road city-2-loc-14 city-2-loc-13)
  (= (road-length city-2-loc-14 city-2-loc-13) 30)
  ; 2718,959 -> 2699,663
  (road city-2-loc-13 city-2-loc-14)
  (= (road-length city-2-loc-13 city-2-loc-14) 30)
  ; 2546,746 -> 2464,650
  (road city-2-loc-15 city-2-loc-10)
  (= (road-length city-2-loc-15 city-2-loc-10) 13)
  ; 2464,650 -> 2546,746
  (road city-2-loc-10 city-2-loc-15)
  (= (road-length city-2-loc-10 city-2-loc-15) 13)
  ; 2546,746 -> 2718,959
  (road city-2-loc-15 city-2-loc-13)
  (= (road-length city-2-loc-15 city-2-loc-13) 28)
  ; 2718,959 -> 2546,746
  (road city-2-loc-13 city-2-loc-15)
  (= (road-length city-2-loc-13 city-2-loc-15) 28)
  ; 2546,746 -> 2699,663
  (road city-2-loc-15 city-2-loc-14)
  (= (road-length city-2-loc-15 city-2-loc-14) 18)
  ; 2699,663 -> 2546,746
  (road city-2-loc-14 city-2-loc-15)
  (= (road-length city-2-loc-14 city-2-loc-15) 18)
  ; 2473,388 -> 2591,196
  (road city-2-loc-16 city-2-loc-2)
  (= (road-length city-2-loc-16 city-2-loc-2) 23)
  ; 2591,196 -> 2473,388
  (road city-2-loc-2 city-2-loc-16)
  (= (road-length city-2-loc-2 city-2-loc-16) 23)
  ; 2473,388 -> 2389,284
  (road city-2-loc-16 city-2-loc-9)
  (= (road-length city-2-loc-16 city-2-loc-9) 14)
  ; 2389,284 -> 2473,388
  (road city-2-loc-9 city-2-loc-16)
  (= (road-length city-2-loc-9 city-2-loc-16) 14)
  ; 2473,388 -> 2464,650
  (road city-2-loc-16 city-2-loc-10)
  (= (road-length city-2-loc-16 city-2-loc-10) 27)
  ; 2464,650 -> 2473,388
  (road city-2-loc-10 city-2-loc-16)
  (= (road-length city-2-loc-10 city-2-loc-16) 27)
  ; 2473,388 -> 2686,273
  (road city-2-loc-16 city-2-loc-12)
  (= (road-length city-2-loc-16 city-2-loc-12) 25)
  ; 2686,273 -> 2473,388
  (road city-2-loc-12 city-2-loc-16)
  (= (road-length city-2-loc-12 city-2-loc-16) 25)
  ; 2276,947 -> 2233,791
  (road city-2-loc-17 city-2-loc-5)
  (= (road-length city-2-loc-17 city-2-loc-5) 17)
  ; 2233,791 -> 2276,947
  (road city-2-loc-5 city-2-loc-17)
  (= (road-length city-2-loc-5 city-2-loc-17) 17)
  ; 2863,786 -> 2718,959
  (road city-2-loc-18 city-2-loc-13)
  (= (road-length city-2-loc-18 city-2-loc-13) 23)
  ; 2718,959 -> 2863,786
  (road city-2-loc-13 city-2-loc-18)
  (= (road-length city-2-loc-13 city-2-loc-18) 23)
  ; 2863,786 -> 2699,663
  (road city-2-loc-18 city-2-loc-14)
  (= (road-length city-2-loc-18 city-2-loc-14) 21)
  ; 2699,663 -> 2863,786
  (road city-2-loc-14 city-2-loc-18)
  (= (road-length city-2-loc-14 city-2-loc-18) 21)
  ; 2032,824 -> 2046,677
  (road city-2-loc-19 city-2-loc-4)
  (= (road-length city-2-loc-19 city-2-loc-4) 15)
  ; 2046,677 -> 2032,824
  (road city-2-loc-4 city-2-loc-19)
  (= (road-length city-2-loc-4 city-2-loc-19) 15)
  ; 2032,824 -> 2233,791
  (road city-2-loc-19 city-2-loc-5)
  (= (road-length city-2-loc-19 city-2-loc-5) 21)
  ; 2233,791 -> 2032,824
  (road city-2-loc-5 city-2-loc-19)
  (= (road-length city-2-loc-5 city-2-loc-19) 21)
  ; 2032,824 -> 2276,947
  (road city-2-loc-19 city-2-loc-17)
  (= (road-length city-2-loc-19 city-2-loc-17) 28)
  ; 2276,947 -> 2032,824
  (road city-2-loc-17 city-2-loc-19)
  (= (road-length city-2-loc-17 city-2-loc-19) 28)
  ; 2323,410 -> 2389,284
  (road city-2-loc-20 city-2-loc-9)
  (= (road-length city-2-loc-20 city-2-loc-9) 15)
  ; 2389,284 -> 2323,410
  (road city-2-loc-9 city-2-loc-20)
  (= (road-length city-2-loc-9 city-2-loc-20) 15)
  ; 2323,410 -> 2464,650
  (road city-2-loc-20 city-2-loc-10)
  (= (road-length city-2-loc-20 city-2-loc-10) 28)
  ; 2464,650 -> 2323,410
  (road city-2-loc-10 city-2-loc-20)
  (= (road-length city-2-loc-10 city-2-loc-20) 28)
  ; 2323,410 -> 2166,379
  (road city-2-loc-20 city-2-loc-11)
  (= (road-length city-2-loc-20 city-2-loc-11) 16)
  ; 2166,379 -> 2323,410
  (road city-2-loc-11 city-2-loc-20)
  (= (road-length city-2-loc-11 city-2-loc-20) 16)
  ; 2323,410 -> 2473,388
  (road city-2-loc-20 city-2-loc-16)
  (= (road-length city-2-loc-20 city-2-loc-16) 16)
  ; 2473,388 -> 2323,410
  (road city-2-loc-16 city-2-loc-20)
  (= (road-length city-2-loc-16 city-2-loc-20) 16)
  ; 2965,580 -> 2882,370
  (road city-2-loc-21 city-2-loc-1)
  (= (road-length city-2-loc-21 city-2-loc-1) 23)
  ; 2882,370 -> 2965,580
  (road city-2-loc-1 city-2-loc-21)
  (= (road-length city-2-loc-1 city-2-loc-21) 23)
  ; 2965,580 -> 2699,663
  (road city-2-loc-21 city-2-loc-14)
  (= (road-length city-2-loc-21 city-2-loc-14) 28)
  ; 2699,663 -> 2965,580
  (road city-2-loc-14 city-2-loc-21)
  (= (road-length city-2-loc-14 city-2-loc-21) 28)
  ; 2965,580 -> 2863,786
  (road city-2-loc-21 city-2-loc-18)
  (= (road-length city-2-loc-21 city-2-loc-18) 23)
  ; 2863,786 -> 2965,580
  (road city-2-loc-18 city-2-loc-21)
  (= (road-length city-2-loc-18 city-2-loc-21) 23)
  ; 2671,511 -> 2882,370
  (road city-2-loc-22 city-2-loc-1)
  (= (road-length city-2-loc-22 city-2-loc-1) 26)
  ; 2882,370 -> 2671,511
  (road city-2-loc-1 city-2-loc-22)
  (= (road-length city-2-loc-1 city-2-loc-22) 26)
  ; 2671,511 -> 2464,650
  (road city-2-loc-22 city-2-loc-10)
  (= (road-length city-2-loc-22 city-2-loc-10) 25)
  ; 2464,650 -> 2671,511
  (road city-2-loc-10 city-2-loc-22)
  (= (road-length city-2-loc-10 city-2-loc-22) 25)
  ; 2671,511 -> 2686,273
  (road city-2-loc-22 city-2-loc-12)
  (= (road-length city-2-loc-22 city-2-loc-12) 24)
  ; 2686,273 -> 2671,511
  (road city-2-loc-12 city-2-loc-22)
  (= (road-length city-2-loc-12 city-2-loc-22) 24)
  ; 2671,511 -> 2699,663
  (road city-2-loc-22 city-2-loc-14)
  (= (road-length city-2-loc-22 city-2-loc-14) 16)
  ; 2699,663 -> 2671,511
  (road city-2-loc-14 city-2-loc-22)
  (= (road-length city-2-loc-14 city-2-loc-22) 16)
  ; 2671,511 -> 2546,746
  (road city-2-loc-22 city-2-loc-15)
  (= (road-length city-2-loc-22 city-2-loc-15) 27)
  ; 2546,746 -> 2671,511
  (road city-2-loc-15 city-2-loc-22)
  (= (road-length city-2-loc-15 city-2-loc-22) 27)
  ; 2671,511 -> 2473,388
  (road city-2-loc-22 city-2-loc-16)
  (= (road-length city-2-loc-22 city-2-loc-16) 24)
  ; 2473,388 -> 2671,511
  (road city-2-loc-16 city-2-loc-22)
  (= (road-length city-2-loc-16 city-2-loc-22) 24)
  ; 2405,905 -> 2233,791
  (road city-2-loc-23 city-2-loc-5)
  (= (road-length city-2-loc-23 city-2-loc-5) 21)
  ; 2233,791 -> 2405,905
  (road city-2-loc-5 city-2-loc-23)
  (= (road-length city-2-loc-5 city-2-loc-23) 21)
  ; 2405,905 -> 2464,650
  (road city-2-loc-23 city-2-loc-10)
  (= (road-length city-2-loc-23 city-2-loc-10) 27)
  ; 2464,650 -> 2405,905
  (road city-2-loc-10 city-2-loc-23)
  (= (road-length city-2-loc-10 city-2-loc-23) 27)
  ; 2405,905 -> 2546,746
  (road city-2-loc-23 city-2-loc-15)
  (= (road-length city-2-loc-23 city-2-loc-15) 22)
  ; 2546,746 -> 2405,905
  (road city-2-loc-15 city-2-loc-23)
  (= (road-length city-2-loc-15 city-2-loc-23) 22)
  ; 2405,905 -> 2276,947
  (road city-2-loc-23 city-2-loc-17)
  (= (road-length city-2-loc-23 city-2-loc-17) 14)
  ; 2276,947 -> 2405,905
  (road city-2-loc-17 city-2-loc-23)
  (= (road-length city-2-loc-17 city-2-loc-23) 14)
  ; 2469,146 -> 2591,196
  (road city-2-loc-24 city-2-loc-2)
  (= (road-length city-2-loc-24 city-2-loc-2) 14)
  ; 2591,196 -> 2469,146
  (road city-2-loc-2 city-2-loc-24)
  (= (road-length city-2-loc-2 city-2-loc-24) 14)
  ; 2469,146 -> 2721,71
  (road city-2-loc-24 city-2-loc-3)
  (= (road-length city-2-loc-24 city-2-loc-3) 27)
  ; 2721,71 -> 2469,146
  (road city-2-loc-3 city-2-loc-24)
  (= (road-length city-2-loc-3 city-2-loc-24) 27)
  ; 2469,146 -> 2296,81
  (road city-2-loc-24 city-2-loc-6)
  (= (road-length city-2-loc-24 city-2-loc-6) 19)
  ; 2296,81 -> 2469,146
  (road city-2-loc-6 city-2-loc-24)
  (= (road-length city-2-loc-6 city-2-loc-24) 19)
  ; 2469,146 -> 2389,284
  (road city-2-loc-24 city-2-loc-9)
  (= (road-length city-2-loc-24 city-2-loc-9) 16)
  ; 2389,284 -> 2469,146
  (road city-2-loc-9 city-2-loc-24)
  (= (road-length city-2-loc-9 city-2-loc-24) 16)
  ; 2469,146 -> 2686,273
  (road city-2-loc-24 city-2-loc-12)
  (= (road-length city-2-loc-24 city-2-loc-12) 26)
  ; 2686,273 -> 2469,146
  (road city-2-loc-12 city-2-loc-24)
  (= (road-length city-2-loc-12 city-2-loc-24) 26)
  ; 2469,146 -> 2473,388
  (road city-2-loc-24 city-2-loc-16)
  (= (road-length city-2-loc-24 city-2-loc-16) 25)
  ; 2473,388 -> 2469,146
  (road city-2-loc-16 city-2-loc-24)
  (= (road-length city-2-loc-16 city-2-loc-24) 25)
  ; 2574,551 -> 2464,650
  (road city-2-loc-25 city-2-loc-10)
  (= (road-length city-2-loc-25 city-2-loc-10) 15)
  ; 2464,650 -> 2574,551
  (road city-2-loc-10 city-2-loc-25)
  (= (road-length city-2-loc-10 city-2-loc-25) 15)
  ; 2574,551 -> 2699,663
  (road city-2-loc-25 city-2-loc-14)
  (= (road-length city-2-loc-25 city-2-loc-14) 17)
  ; 2699,663 -> 2574,551
  (road city-2-loc-14 city-2-loc-25)
  (= (road-length city-2-loc-14 city-2-loc-25) 17)
  ; 2574,551 -> 2546,746
  (road city-2-loc-25 city-2-loc-15)
  (= (road-length city-2-loc-25 city-2-loc-15) 20)
  ; 2546,746 -> 2574,551
  (road city-2-loc-15 city-2-loc-25)
  (= (road-length city-2-loc-15 city-2-loc-25) 20)
  ; 2574,551 -> 2473,388
  (road city-2-loc-25 city-2-loc-16)
  (= (road-length city-2-loc-25 city-2-loc-16) 20)
  ; 2473,388 -> 2574,551
  (road city-2-loc-16 city-2-loc-25)
  (= (road-length city-2-loc-16 city-2-loc-25) 20)
  ; 2574,551 -> 2323,410
  (road city-2-loc-25 city-2-loc-20)
  (= (road-length city-2-loc-25 city-2-loc-20) 29)
  ; 2323,410 -> 2574,551
  (road city-2-loc-20 city-2-loc-25)
  (= (road-length city-2-loc-20 city-2-loc-25) 29)
  ; 2574,551 -> 2671,511
  (road city-2-loc-25 city-2-loc-22)
  (= (road-length city-2-loc-25 city-2-loc-22) 11)
  ; 2671,511 -> 2574,551
  (road city-2-loc-22 city-2-loc-25)
  (= (road-length city-2-loc-22 city-2-loc-25) 11)
  ; 2598,438 -> 2882,370
  (road city-2-loc-26 city-2-loc-1)
  (= (road-length city-2-loc-26 city-2-loc-1) 30)
  ; 2882,370 -> 2598,438
  (road city-2-loc-1 city-2-loc-26)
  (= (road-length city-2-loc-1 city-2-loc-26) 30)
  ; 2598,438 -> 2591,196
  (road city-2-loc-26 city-2-loc-2)
  (= (road-length city-2-loc-26 city-2-loc-2) 25)
  ; 2591,196 -> 2598,438
  (road city-2-loc-2 city-2-loc-26)
  (= (road-length city-2-loc-2 city-2-loc-26) 25)
  ; 2598,438 -> 2389,284
  (road city-2-loc-26 city-2-loc-9)
  (= (road-length city-2-loc-26 city-2-loc-9) 26)
  ; 2389,284 -> 2598,438
  (road city-2-loc-9 city-2-loc-26)
  (= (road-length city-2-loc-9 city-2-loc-26) 26)
  ; 2598,438 -> 2464,650
  (road city-2-loc-26 city-2-loc-10)
  (= (road-length city-2-loc-26 city-2-loc-10) 26)
  ; 2464,650 -> 2598,438
  (road city-2-loc-10 city-2-loc-26)
  (= (road-length city-2-loc-10 city-2-loc-26) 26)
  ; 2598,438 -> 2686,273
  (road city-2-loc-26 city-2-loc-12)
  (= (road-length city-2-loc-26 city-2-loc-12) 19)
  ; 2686,273 -> 2598,438
  (road city-2-loc-12 city-2-loc-26)
  (= (road-length city-2-loc-12 city-2-loc-26) 19)
  ; 2598,438 -> 2699,663
  (road city-2-loc-26 city-2-loc-14)
  (= (road-length city-2-loc-26 city-2-loc-14) 25)
  ; 2699,663 -> 2598,438
  (road city-2-loc-14 city-2-loc-26)
  (= (road-length city-2-loc-14 city-2-loc-26) 25)
  ; 2598,438 -> 2473,388
  (road city-2-loc-26 city-2-loc-16)
  (= (road-length city-2-loc-26 city-2-loc-16) 14)
  ; 2473,388 -> 2598,438
  (road city-2-loc-16 city-2-loc-26)
  (= (road-length city-2-loc-16 city-2-loc-26) 14)
  ; 2598,438 -> 2323,410
  (road city-2-loc-26 city-2-loc-20)
  (= (road-length city-2-loc-26 city-2-loc-20) 28)
  ; 2323,410 -> 2598,438
  (road city-2-loc-20 city-2-loc-26)
  (= (road-length city-2-loc-20 city-2-loc-26) 28)
  ; 2598,438 -> 2671,511
  (road city-2-loc-26 city-2-loc-22)
  (= (road-length city-2-loc-26 city-2-loc-22) 11)
  ; 2671,511 -> 2598,438
  (road city-2-loc-22 city-2-loc-26)
  (= (road-length city-2-loc-22 city-2-loc-26) 11)
  ; 2598,438 -> 2574,551
  (road city-2-loc-26 city-2-loc-25)
  (= (road-length city-2-loc-26 city-2-loc-25) 12)
  ; 2574,551 -> 2598,438
  (road city-2-loc-25 city-2-loc-26)
  (= (road-length city-2-loc-25 city-2-loc-26) 12)
  ; 996,944 <-> 2032,824
  (road city-1-loc-24 city-2-loc-19)
  (= (road-length city-1-loc-24 city-2-loc-19) 105)
  (road city-2-loc-19 city-1-loc-24)
  (= (road-length city-2-loc-19 city-1-loc-24) 105)
  (at package-1 city-1-loc-19)
  (at package-2 city-1-loc-13)
  (at package-3 city-1-loc-12)
  (at truck-1 city-2-loc-8)
  (capacity truck-1 capacity-2)
  (at truck-2 city-2-loc-17)
  (capacity truck-2 capacity-3)
  (at truck_f city-1-loc-1)
 )
 (:goal (and
  (at package-1 city-2-loc-3)
  (at package-2 city-2-loc-25)
  (at package-3 city-2-loc-2)
 ))
 (:metric minimize (total-cost))
)
