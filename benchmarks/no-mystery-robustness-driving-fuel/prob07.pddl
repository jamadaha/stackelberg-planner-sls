(define
  (problem strips-mysty-x-7)
  (:domain no-mystery-strips)
  (:objects endingen breisach freiburg tumringen wollbach weil
      brombach denzlingen schopfheim inzlingen elfenrad feuerwehr
      neujahrsbrezel gruenkohl saumagen radio twix apfel fussball
      kukuruz weihnachtsbaum kuechenmaschine zehn-nackte-friseusen
      feine-bratwurst strunkbolzen schlagobers gimp karfiol
      leipziger-allerlei haehnchenbein ochsencremesuppe donnerkiesel
      seitenbacher-muesli broiler eisbein kaesebaellchen fuel-0 fuel-1
      fuel-2 fuel-3 capacity-0 capacity-1 truck-fix)
  (:init
    (at apfel freiburg)
    (at broiler schopfheim)
    (at donnerkiesel schopfheim)
    (at eisbein schopfheim)
    (at elfenrad wollbach)
    (at feine-bratwurst wollbach)
    (at feuerwehr brombach)
    (at fussball tumringen)
    (at gimp brombach)
    (at gruenkohl endingen)
    (at haehnchenbein brombach)
    (at kaesebaellchen inzlingen)
    (at karfiol brombach)
    (at kuechenmaschine wollbach)
    (at kukuruz tumringen)
    (at leipziger-allerlei brombach)
    (at neujahrsbrezel endingen)
    (at ochsencremesuppe denzlingen)
    (at radio breisach)
    (at saumagen endingen)
    (at schlagobers weil)
    (at seitenbacher-muesli schopfheim)
    (at strunkbolzen weil)
    (at twix breisach)
    (at weihnachtsbaum wollbach)
    (at zehn-nackte-friseusen wollbach)
    (capacity elfenrad capacity-1)
    (capacity feuerwehr capacity-1)
    (capacity-number capacity-0)
    (capacity-number capacity-1)
    (capacity-predecessor capacity-0 capacity-1)
    (connected breisach schopfheim)
    (connected breisach tumringen)
    (connected brombach endingen)
    (connected brombach weil)
    (connected brombach wollbach)
    (connected denzlingen endingen)
    (connected denzlingen freiburg)
    (connected denzlingen schopfheim)
    (connected endingen brombach)
    (connected endingen denzlingen)
    (connected freiburg denzlingen)
    (connected freiburg schopfheim)
    (connected freiburg wollbach)
    (connected inzlingen schopfheim)
    (connected inzlingen weil)
    (connected schopfheim breisach)
    (connected schopfheim denzlingen)
    (connected schopfheim freiburg)
    (connected schopfheim inzlingen)
    (connected tumringen breisach)
    (connected tumringen weil)
    (connected tumringen wollbach)
    (connected weil brombach)
    (connected weil inzlingen)
    (connected weil tumringen)
    (connected wollbach brombach)
    (connected wollbach freiburg)
    (connected wollbach tumringen)
    (fuel breisach fuel-0)
    (fuel brombach fuel-2)
    (fuel denzlingen fuel-0)
    (fuel endingen fuel-0)
    (fuel freiburg fuel-1)
    (fuel inzlingen fuel-0)
    (fuel schopfheim fuel-3)
    (fuel tumringen fuel-2)
    (fuel weil fuel-1)
    (fuel wollbach fuel-0)
    (fuel-number fuel-0)
    (fuel-number fuel-1)
    (fuel-number fuel-2)
    (fuel-number fuel-3)
    (fuel-predecessor fuel-0 fuel-1)
    (fuel-predecessor fuel-1 fuel-2)
    (fuel-predecessor fuel-2 fuel-3)
    (location breisach)
    (location brombach)
    (location denzlingen)
    (location endingen)
    (location freiburg)
    (location inzlingen)
    (location schopfheim)
    (location tumringen)
    (location weil)
    (location wollbach)
    (package apfel)
    (package broiler)
    (package donnerkiesel)
    (package eisbein)
    (package feine-bratwurst)
    (package fussball)
    (package gimp)
    (package gruenkohl)
    (package haehnchenbein)
    (package kaesebaellchen)
    (package karfiol)
    (package kuechenmaschine)
    (package kukuruz)
    (package leipziger-allerlei)
    (package neujahrsbrezel)
    (package ochsencremesuppe)
    (package radio)
    (package saumagen)
    (package schlagobers)
    (package seitenbacher-muesli)
    (package strunkbolzen)
    (package twix)
    (package weihnachtsbaum)
    (package zehn-nackte-friseusen)
    (vehicle elfenrad)
    (vehicle feuerwehr)
    (fix_vehicle truck-fix)
    (at truck-fix endingen))
  (:goal
    (and
      (at apfel denzlingen))))
