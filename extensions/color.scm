;;; color.scm: adds color to  messages from buddies
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
;;;
;;; color.scm should be automatically loaded thru init.scm
;;; /eval (ft-set-buddy-color! <buddy name> <color>)
;;; /eval (set! ft-default-color <color>)
;;;
;;; example:
;;; /color-buddy balugi yellow
;;; /color-disable
;;; /color-enable

(use-modules (ice-9 regex))

(define enable-colors-flag "yes")
(define ignored-msg-pattern-list '())

(define color-list
  '(("yellow" . "33")
    ("magenta" . "35")
    ("cyan" . "36")
    ("white" . "37")
    ("red" . "31")
    ("green" . "32")
    ("blue" . "34")))


(define auto-color-list '())

(define (ignore-message! pattern)
  "ignore messages matching the pattern"
  (set! ignored-msg-pattern-list
        (cons pattern ignored-msg-pattern-list)))

(define (ignored-message? message)
  "tell if this message has to be ignored"
  (letrec
      ((local-ignored-message?
        (lambda (pattern-list message)
          (if (= (length pattern-list) 0)
              #f
              (if (= (length pattern-list) 1)
                  (regexp-match? (string-match
                                  (car pattern-list) message))
                  (if (regexp-match? (string-match
                                      (car pattern-list) message))
                      #t
                      (local-ignored-message?
                       (cdr pattern-list) message)))))))
       (local-ignored-message? ignored-msg-pattern-list message)))


(define default-color "cyan")

(define (set-buddy-color! buddy color)
  "specify color for buddies"
  (if (assoc buddy auto-color-list)
      (set! auto-color-list
            (delete (cons
                     buddy
                     (cdr (assoc buddy auto-color-list)))
                    auto-color-list)))
  (set! auto-color-list
        (append auto-color-list
                (list (cons buddy color)))))

(define (get-buddy-color buddy)
  (if (assoc buddy auto-color-list)
      '()
      (begin
        (set! auto-color-list
              (append
               auto-color-list
               (list (cons
                      buddy
                      (car (list-ref color-list
                                     (modulo
                                      (length auto-color-list)
                                      (length color-list))))))))))
  (cdr (assoc buddy auto-color-list)))

(define (color-message msg color)
  "adds color to message"
  (if (equal? enable-colors-flag "yes")
   (let ((col-no (cdr (assoc default-color color-list))))
    (and (assoc color color-list)
     (set! col-no (cdr (assoc color color-list  ))))
    (string-append "\x1b[1;" col-no ";40m" msg "\x1b[0m"))
   (string-append msg)))

(define (format-msg timestamp from nickname msg)
    (string-append
      (if (> (string-length timestamp) 0)
        (color-message (string-append "[" timestamp "] ")
                       (get-buddy-color from))
        (color-message (strftime "%I:%M%p " (localtime
                                              (current-time)))
                       (get-buddy-color from)))
      (color-message (if (> (string-length nickname) 0)
                       nickname
                       from)
                     (get-buddy-color from))
      (if (string-prefix? "/me " msg)
        (color-message (substring msg 3) (get-buddy-color from))
        (color-message (string-append " -> " msg)
                       (get-buddy-color from))
        )))

(define (print-chat-msg timestamp from nickname msg)
  "append color"
  (if (ignored-message? msg)
      (ft-hook-return)
          (begin
            (if (get-buddy-color from)
                (begin
                  (ft-display (format-msg timestamp from nickname msg))
                  (ft-hook-return))))))

(define (/color args)
  " enable or disable coloring received messages "
  (cond ((equal? args "off")
            (begin
              (set! enable-colors-flag "no")
              (ft-display (_ " BUDDY coloring disabled "))))
        ((equal? args "on")
            (begin
              (set! enable-colors-flag "yes")
              (ft-display (_ " BUDDY coloring enabled "))))
	(else (ft-display (_ "Invalid syntax")))))

(add-command! /color "color" "/color" "enables (`on') or disables (`off') buddy coloring")


