;;; proud-of-freetalk.scm: i am proud of freetalk. are you ?
;;; Copyright (c) 2005-2014 Freetalk Core Team
;;; This file is part of GNU Freetalk.
;;;
;;; Freetalk is free software; you can redistribute it and/or modify it
;;; under the terms of the GNU General Public License as published by
;;; the Free Software Foundation; either version 3 of the License, or
;;; (at your option) any later version.
;;;
;;; Freetalk is distributed in the hope that it will be useful, but
;;; WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
;;; General Public License for more details.
;;;
;;; You should have received a copy of the GNU General Public License
;;; along with this program.  If not, see
;;; <http://www.gnu.org/licenses/>.

;;; example:
;;; i know you are proud of using freetalk. so let this file be
;;; automatically loaded thru init.scm
;;; to check if the other buddy is using freetalk, try this command
;;; ~qp~!> /eval (freetalk? 'buddy-name)
;;;
;;; proud-of-freetalk.scm depends on color.scm

(ignore-message! "^.messenger")

(define (proud-of-freetalk timestamp from nickname msg)
  "tell ur buddy that you are proud of using freetalk"
  (and (string=? "?messenger" msg)
       (ft-send-message-no-hook from "?messenger->freetalk")
       (begin (ft-display (string-append (_ "Told [")
                                         from
                                         (_ "] that I'm proud of using Freetalk"))))
       (ft-hook-return)))

(define (freetalk? buddy)
  "does this buddy use freetalk"
  (ft-send-message-no-hook buddy "?messenger"))

(define (proud-of-ft-handler timestamp from nickname msg)
  "handle the freetalk? reply"
  (if (string=? "?messenger-"
                (car (string-split msg #\>)))
      ;; ?messenger->MESSENGER-NAME reply message
      (if (string=? "?messenger->freetalk" msg)
          (begin
            (ft-display
             (string-append (_ "Yes [") from (_ "] is using [")
                            (cadr (string-split msg #\>))
                            "]"))
            (ft-hook-return))
          (begin
            (ft-display
             (string-append (_ "No, but [") from (_ "] is using [")
                            (cadr (string-split msg #\>))
                            "]"))
            (ft-hook-return)))))


(add-hook! ft-message-receive-hook proud-of-freetalk)
(add-hook! ft-message-receive-hook proud-of-ft-handler)

(define (/freetalk args)
  "dynamic command interface to proud-of-freetalk extension"
  (if (> (string-length args) 0)
      (freetalk? (sans-surrounding-whitespace args))
      (display (_ "proud-of-freetalk.scm: wrong number of arguments to /freetalk\n"))))

(add-command! /freetalk "freetalk" "/freetalk BUDDY" "check whether a BUDDY is using freetalk")
