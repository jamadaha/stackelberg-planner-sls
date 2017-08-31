(define
  (problem strips-mysty-x-23)
  (:domain no-mystery-strips)
  (:objects bahlingen brombach guendlingen lauchringen koendringen
      loerrach breisach waldhaus freiburg muellheim sexau kleinkems
      weil denzlingen waldkirch haltingen tumringen bad-bellingen
      schallstadt sulki bollerwagen kutsche muellabfuhr segway
      trollwagen krankenwagen schlagobers fernseher twix kaesefondue
      pinkel strunkbolzen kaesebaellchen ochsencremesuppe faschiertes
      halber-hirsch snickers kuechenmaschine feinkost-bratling
      kiste-bier neujahrsbrezel radio fuel-0 fuel-1 fuel-2 fuel-3
      fuel-4 fuel-5 capacity-0 capacity-1 capacity-2 capacity-3 truck-fix)
  (:init
    (at bollerwagen koendringen)
    (at faschiertes muellheim)
    (at feinkost-bratling denzlingen)
    (at fernseher brombach)
    (at halber-hirsch sexau)
    (at kaesebaellchen breisach)
    (at kaesefondue lauchringen)
    (at kiste-bier waldkirch)
    (at krankenwagen schallstadt)
    (at kuechenmaschine weil)
    (at kutsche waldhaus)
    (at muellabfuhr sexau)
    (at neujahrsbrezel haltingen)
    (at ochsencremesuppe freiburg)
    (at pinkel koendringen)
    (at radio bad-bellingen)
    (at schlagobers bahlingen)
    (at segway kleinkems)
    (at snickers kleinkems)
    (at strunkbolzen loerrach)
    (at sulki brombach)
    (at trollwagen haltingen)
    (at twix guendlingen)
    (capacity bollerwagen capacity-2)
    (capacity krankenwagen capacity-3)
    (capacity kutsche capacity-2)
    (capacity muellabfuhr capacity-2)
    (capacity segway capacity-1)
    (capacity sulki capacity-1)
    (capacity trollwagen capacity-2)
    (capacity-number capacity-0)
    (capacity-number capacity-1)
    (capacity-number capacity-2)
    (capacity-number capacity-3)
    (capacity-predecessor capacity-0 capacity-1)
    (capacity-predecessor capacity-1 capacity-2)
    (capacity-predecessor capacity-2 capacity-3)
    (connected bad-bellingen schallstadt)
    (connected bad-bellingen tumringen)
    (connected bahlingen sexau)
    (connected bahlingen waldhaus)
    (connected breisach koendringen)
    (connected breisach loerrach)
    (connected breisach muellheim)
    (connected breisach sexau)
    (connected brombach freiburg)
    (connected brombach waldhaus)
    (connected denzlingen tumringen)
    (connected denzlingen waldkirch)
    (connected freiburg brombach)
    (connected freiburg koendringen)
    (connected guendlingen lauchringen)
    (connected guendlingen loerrach)
    (connected haltingen kleinkems)
    (connected haltingen tumringen)
    (connected haltingen waldkirch)
    (connected kleinkems haltingen)
    (connected kleinkems schallstadt)
    (connected koendringen breisach)
    (connected koendringen freiburg)
    (connected lauchringen guendlingen)
    (connected lauchringen waldhaus)
    (connected loerrach breisach)
    (connected loerrach guendlingen)
    (connected muellheim breisach)
    (connected muellheim sexau)
    (connected schallstadt bad-bellingen)
    (connected schallstadt kleinkems)
    (connected schallstadt weil)
    (connected sexau bahlingen)
    (connected sexau breisach)
    (connected sexau muellheim)
    (connected tumringen bad-bellingen)
    (connected tumringen denzlingen)
    (connected tumringen haltingen)
    (connected tumringen waldhaus)
    (connected waldhaus bahlingen)
    (connected waldhaus brombach)
    (connected waldhaus lauchringen)
    (connected waldhaus tumringen)
    (connected waldkirch denzlingen)
    (connected waldkirch haltingen)
    (connected waldkirch weil)
    (connected weil schallstadt)
    (connected weil waldkirch)
    (fuel bad-bellingen fuel-2)
    (fuel bahlingen fuel-4)
    (fuel breisach fuel-4)
    (fuel brombach fuel-1)
    (fuel denzlingen fuel-4)
    (fuel freiburg fuel-0)
    (fuel guendlingen fuel-5)
    (fuel haltingen fuel-1)
    (fuel kleinkems fuel-4)
    (fuel koendringen fuel-3)
    (fuel lauchringen fuel-3)
    (fuel loerrach fuel-5)
    (fuel muellheim fuel-4)
    (fuel schallstadt fuel-4)
    (fuel sexau fuel-1)
    (fuel tumringen fuel-0)
    (fuel waldhaus fuel-3)
    (fuel waldkirch fuel-1)
    (fuel weil fuel-2)
    (fuel-number fuel-0)
    (fuel-number fuel-1)
    (fuel-number fuel-2)
    (fuel-number fuel-3)
    (fuel-number fuel-4)
    (fuel-number fuel-5)
    (fuel-predecessor fuel-0 fuel-1)
    (fuel-predecessor fuel-1 fuel-2)
    (fuel-predecessor fuel-2 fuel-3)
    (fuel-predecessor fuel-3 fuel-4)
    (fuel-predecessor fuel-4 fuel-5)
    (location bad-bellingen)
    (location bahlingen)
    (location breisach)
    (location brombach)
    (location denzlingen)
    (location freiburg)
    (location guendlingen)
    (location haltingen)
    (location kleinkems)
    (location koendringen)
    (location lauchringen)
    (location loerrach)
    (location muellheim)
    (location schallstadt)
    (location sexau)
    (location tumringen)
    (location waldhaus)
    (location waldkirch)
    (location weil)
    (package faschiertes)
    (package feinkost-bratling)
    (package fernseher)
    (package halber-hirsch)
    (package kaesebaellchen)
    (package kaesefondue)
    (package kiste-bier)
    (package kuechenmaschine)
    (package neujahrsbrezel)
    (package ochsencremesuppe)
    (package pinkel)
    (package radio)
    (package schlagobers)
    (package snickers)
    (package strunkbolzen)
    (package twix)
    (vehicle bollerwagen)
    (vehicle krankenwagen)
    (vehicle kutsche)
    (vehicle muellabfuhr)
    (vehicle segway)
    (vehicle sulki)
    (vehicle trollwagen)
    (fix_vehicle truck-fix)
    (at truck-fix bahlingen))
  (:goal
    (and
      (at faschiertes denzlingen))))
