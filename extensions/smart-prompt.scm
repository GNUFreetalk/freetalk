;;; smart-prompt: Show in the prompt the jid of the last destination user
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

(use-modules (ice-9 q))

(define msgs-htable (make-hash-table))
(define waiting-users-count '0)
(define verbosity 2)
;;; verbosity levels:
;;; 0 -- quiet: no messages or status changes appear
;;; 1 -- terse: no status changes appear
;;; 2 -- normal

;;; guile < 2.0.9 doesn't implement `hash-count`
(define (hash-count-wrap table)
  (if (hash-table? table)
      ;; check for guile version
      (if (< (string->number (micro-version)) 9)
          (hash-fold (lambda (key value seed) (+ 1 seed)) 0 table)
          (hash-count (const #t) table))
      '0))

(define (jid-to-nick jid)
  (let ((item '()) (nick "") (ret ""))
    (if (not (string-null? jid))
      (begin
        (set! item (ft-roster-lookup jid))
        (if (not (null? item))
            (begin
              (set! nick (list-ref item 2))
              (if (not (null? nick))
                  (set! ret nick))))))
    ret))

(define (update-prompt)
  "update and redisplay the prompt according to the selected chat mode"
  (if (> (ft-get-conn-status) 0)
    (let ((current-buddy "") (nickname "") (prompt-jid ""))
      (set! current-buddy (ft-get-current-buddy))
      (set! nickname (jid-to-nick current-buddy))
      (set! prompt-jid (cond ((not (string-null? nickname)) nickname)
                             ((not (string-null? current-buddy)) current-buddy)
                             (else (ft-get-jid))))
      (if (= verbosity 0)
          (begin
            (set! waiting-users-count (hash-count-wrap msgs-htable))
            (ft-set-prompt!
             (string-append "(new:" ; "\x01\x1b[33;1m\x02"
                            (number->string waiting-users-count)
                                        ; "\x01\x1b[0m\x02"
                            ") "
                            prompt-jid "> "))
            (ft-rl-redisplay))
          (ft-set-prompt! (string-append prompt-jid "> "))))))

(define (store-msg from msg)
  "store the msg in the hash table"
  (let ((buddy-msgs (hash-ref msgs-htable from)))
    (if (equal? buddy-msgs #f)
      (begin
        (set! buddy-msgs (make-q))
        (hash-set! msgs-htable from buddy-msgs)))
    (enq! buddy-msgs msg)))


(define (process-msg timestamp from nickname msg)
  "save in the hastable or print directly on the screen"
  (if (>= verbosity 1)
    (print-chat-msg timestamp from nickname msg)
    (begin
      (store-msg from (list (current-time) timestamp from nickname msg))
      (update-prompt)))
  (ft-hook-return))

(define (/next args)
  "print the next unread msgs belonging to a one sender at a time"
  (let ((next-buddy "") (msgs '()) (msgs-buffer ""))
    (hash-fold (lambda (from msgs-q prior)
                 (let ((current-msg-time (car (q-front msgs-q))))
                   (if (> prior current-msg-time)
                     (set! next-buddy (string-copy from)))
                   current-msg-time))
               +inf.0 msgs-htable)
    (if (< 0 (string-length next-buddy))
      (begin
        (set! msgs (hash-ref msgs-htable next-buddy))
        (if (not (equal? msgs #f))
          (begin
            (while (not (q-empty? msgs))
                   (let ((msg '()))
                     (set! msg (deq! msgs ))
                     (set! msgs-buffer
                       (string-append msgs-buffer
                         (format-msg (strftime "%I:%M%p" (localtime (car msg)))
                                     (caddr msg)
                                     (cadddr msg)
                                     (car (cddddr msg))) "\n"))))
            (ft-pager-display msgs-buffer)
            (hash-remove! msgs-htable next-buddy)
            (ft-set-current-buddy! (regexp-substitute/global
                                    #f "/.*$" next-buddy
                                    'pre "" 'post))))))
    (update-prompt)))

(add-command! /next "next" "/next" "display next message")

(define (/mode arg)
  "set chat mode"
  (cond
    ((equal? arg "quiet")
      (begin
        (set! verbosity 0)
        (ft-display (_ " Quiet chat mode selected "))
        (update-prompt)))
    ((equal? arg "terse")
      (begin
        (set! verbosity 1)
        (ft-display (_ " Terse chat mode selected "))
        (update-prompt)))
    ((equal? arg "normal")
      (begin
        (set! verbosity 2)
        (ft-display (_ " Normal chat mode selected "))
        (update-prompt)))
    ((equal? arg "")
        (ft-display (_ (string-append "Current mode: "
             (list-ref '("quiet" "terse" "normal") verbosity)))))
    (else (ft-display (_ "Invalid syntax")))))

(add-command! /mode "mode" "/mode"
    "Select `quiet', `terse' or `normal' chat mode")

(add-hook! ft-message-send-hook
           (lambda (to message)
             (ft-set-prompt! (string-append to "> "))
             (update-prompt)))

(add-hook! ft-message-receive-hook process-msg)

(define (presence-recv jid online nickname show-msg status-msg)
  (if (= verbosity 2)
    (let ((item (ft-roster-lookup jid)))
      (if (not (null? item))
        (let ((old-jid (list-ref item 0))
              (old-online (list-ref item 1))
              (old-nickname (list-ref item 2))
              (old-show-msg (list-ref item 3))
              (old-status-msg (list-ref item 4)))
          (if (or (not (eq? old-online online))
                  (not (string=? old-show-msg show-msg))
                  (not (string=? old-status-msg status-msg)))
            ; using old-jid is a simple way to strip away the resource part ;)
            (ft-display (string-append old-jid
                                       (if (> (string-length old-nickname) 0)
                                         (string-append " (" old-nickname ")")
                                         "")
                                       (_ " is now")
                                       (if online
                                         (_ " Online")
                                         (_ " Offline"))
                                       (if (> (string-length show-msg) 0)
                                         (string-append " ["
                                                        (pretty-print-show-msg
                                                         show-msg) "]")
                                         "")
                                       (if (> (string-length status-msg) 0)
                                         (string-append " (" status-msg ")")
                                         "")))))))))

(add-hook! ft-presence-receive-hook presence-recv)
