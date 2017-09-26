(define
  (problem strips-mysty-x-19)
  (:domain no-mystery-strips)
  (:objects auggen boetzingen waldhaus loerrach denzlingen freiburg
      endingen inzlingen bad-bellingen hugstetten tumringen breisach
      brombach merdingen kandern bollerwagen motorrad motorroller ICE
      krankenwagen muellabfuhr pferdetransport sulki
      flasche-jaegermeister bananenkiste nichts radio gimp
      leipziger-allerlei broiler pfingstochse terrorist fernseher
      doener-mit-scharf halber-hirsch seitenbacher-muesli faschiertes
      fuel-0 fuel-1 fuel-2 fuel-3 fuel-4 fuel-5 capacity-0 capacity-1
      capacity-2 capacity-3 truck-fix)
  (:init
    (at ICE inzlingen)
    (at bananenkiste boetzingen)
    (at bollerwagen auggen)
    (at broiler inzlingen)
    (at doener-mit-scharf breisach)
    (at faschiertes kandern)
    (at fernseher tumringen)
    (at flasche-jaegermeister auggen)
    (at gimp denzlingen)
    (at halber-hirsch brombach)
    (at krankenwagen bad-bellingen)
    (at leipziger-allerlei endingen)
    (at motorrad waldhaus)
    (at motorroller loerrach)
    (at muellabfuhr hugstetten)
    (at nichts waldhaus)
    (at pferdetransport brombach)
    (at pfingstochse bad-bellingen)
    (at radio loerrach)
    (at seitenbacher-muesli merdingen)
    (at sulki merdingen)
    (at terrorist hugstetten)
    (capacity ICE capacity-2)
    (capacity bollerwagen capacity-3)
    (capacity krankenwagen capacity-3)
    (capacity motorrad capacity-3)
    (capacity motorroller capacity-1)
    (capacity muellabfuhr capacity-1)
    (capacity pferdetransport capacity-2)
    (capacity sulki capacity-2)
    (capacity-number capacity-0)
    (capacity-number capacity-1)
    (capacity-number capacity-2)
    (capacity-number capacity-3)
    (capacity-predecessor capacity-0 capacity-1)
    (capacity-predecessor capacity-1 capacity-2)
    (capacity-predecessor capacity-2 capacity-3)
    (connected auggen inzlingen)
    (connected auggen loerrach)
    (connected bad-bellingen denzlingen)
    (connected bad-bellingen freiburg)
    (connected bad-bellingen kandern)
    (connected boetzingen endingen)
    (connected boetzingen tumringen)
    (connected breisach hugstetten)
    (connected breisach kandern)
    (connected brombach inzlingen)
    (connected brombach merdingen)
    (connected denzlingen bad-bellingen)
    (connected denzlingen tumringen)
    (connected denzlingen waldhaus)
    (connected endingen boetzingen)
    (connected endingen merdingen)
    (connected freiburg bad-bellingen)
    (connected freiburg tumringen)
    (connected hugstetten breisach)
    (connected hugstetten loerrach)
    (connected hugstetten waldhaus)
    (connected inzlingen auggen)
    (connected inzlingen brombach)
    (connected inzlingen merdingen)
    (connected inzlingen tumringen)
    (connected kandern bad-bellingen)
    (connected kandern breisach)
    (connected loerrach auggen)
    (connected loerrach hugstetten)
    (connected merdingen brombach)
    (connected merdingen endingen)
    (connected merdingen inzlingen)
    (connected tumringen boetzingen)
    (connected tumringen denzlingen)
    (connected tumringen freiburg)
    (connected tumringen inzlingen)
    (connected waldhaus denzlingen)
    (connected waldhaus hugstetten)
    (fuel auggen fuel-1)
    (fuel bad-bellingen fuel-1)
    (fuel boetzingen fuel-1)
    (fuel breisach fuel-3)
    (fuel brombach fuel-1)
    (fuel denzlingen fuel-3)
    (fuel endingen fuel-0)
    (fuel freiburg fuel-0)
    (fuel hugstetten fuel-1)
    (fuel inzlingen fuel-0)
    (fuel kandern fuel-5)
    (fuel loerrach fuel-1)
    (fuel merdingen fuel-1)
    (fuel tumringen fuel-3)
    (fuel waldhaus fuel-5)
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
    (location auggen)
    (location bad-bellingen)
    (location boetzingen)
    (location breisach)
    (location brombach)
    (location denzlingen)
    (location endingen)
    (location freiburg)
    (location hugstetten)
    (location inzlingen)
    (location kandern)
    (location loerrach)
    (location merdingen)
    (location tumringen)
    (location waldhaus)
    (package bananenkiste)
    (package broiler)
    (package doener-mit-scharf)
    (package faschiertes)
    (package fernseher)
    (package flasche-jaegermeister)
    (package gimp)
    (package halber-hirsch)
    (package leipziger-allerlei)
    (package nichts)
    (package pfingstochse)
    (package radio)
    (package seitenbacher-muesli)
    (package terrorist)
    (vehicle ICE)
    (vehicle bollerwagen)
    (vehicle krankenwagen)
    (vehicle motorrad)
    (vehicle motorroller)
    (vehicle muellabfuhr)
    (vehicle pferdetransport)
    (vehicle sulki)
    (fix_vehicle truck-fix)
    (at truck-fix auggen))
  (:goal
    (and
      (at gimp loerrach))))
