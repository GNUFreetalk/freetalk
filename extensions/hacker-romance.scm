;;; hacker-romance.scm: extensible romance :p
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

(define min-chars 3)
(define max-chars 33)

(define rand-state (seed->random-state (current-time)))

(define (burst-of-romance buddy count message)
  "burst of MESSAGEs to BUDDY"
  (ft-send-message buddy message)
  (if (= count 1)
      '()
      (begin
        (sleep (+ 1 (random 3 (seed->random-state (current-time)))))
        (burst-of-romance buddy (- count 1) message)
        )))


(define (/burst-of-romance args)
  (let ((args-list (string-split args #\ )))
    (if (>= (length (string-split args #\ )) 3)
        (let
            ((buddy (car args-list))
             (count (string->number (cadr args-list)))
             (message (string-join (cddr args-list))))
          (if (and (> (string-length message) 0)
                   (> count 0))
              (burst-of-romance (string-trim-right buddy #\:) count message)))
        (ft-display (_ "usage: /burst-of-romance BUDDY COUNT MESSAGE")))))

(add-command! /burst-of-romance "burst-of-romance"
              "/burst-of-romance BUDDY COUNT MESSAGE"
              "send COUNT number of MESSAGEs to BUDDY as though you typed by hand")

(define (nstr str count)
  "return COUNT number of CHARs"
  (if (string=? str " ")
      " "
      (if (> count 0)
          (string-append (nstr str (- count 1)) str)
          "")))

(define (burst str min max)
  "explode the STR string with MIN and MAX character count"
  (if (string-null? str)
      ""
      (begin
        (string-append
         (nstr (list->string (list (car (string->list str))))
               (+ min (random max rand-state)))
         (burst (list->string (cdr (string->list str))) min max)))))

(define (/burst args)
  "dynamic command interface to burst procedure"
  (let* ((args-list (split-discarding-char #\space args (lambda (x y)
                                                          (list x y))))
         (buddy     (car args-list))
         (message   (cadr args-list)))
    (if (> (string-length message) 0)
        (ft-send-message (string-trim-right buddy #\:) (burst message
                                                              min-chars
                                                              max-chars))
        (ft-display (_ "usage: /burst BUDDY MESSAGE")))))

(add-command! /burst "burst"
              "/burst BUDDY MESSAGE"
              "Send IRC greeting style MESSAGE")

(define (/greet args)
  "IRC style greeting command"
  (if (> (string-length args) 0)
      (ft-send-message (string-trim-right
                        (string-trim-right args #\space)
                        #\:)
                       (burst (car (string-split args #\@))
                              min-chars max-chars))
      (ft-display (_ "usage: /greet BUDDY"))))

(add-command! /greet "greet"
              "/greet BUDDY"
              "greet like in IRC")
