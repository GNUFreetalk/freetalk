;;; history.scm: logs all the sent and received messages in
;;; ~/.freetalk/history directory
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

(define enable-history-flag "yes")

;;; shell command for pagination
(define history-page-cmd "less --quit-at-eof ")

;;; history path
(define history-path "/dev/")
(define session-file "/dev/null")

;;; append this message to session history file with time stamp
(define (history-raw-session message)
  "log the message to ~/.freetalk/history/USER/session file"
  (define session-fd (open-file session-file "a"))
  (display message session-fd)
  (close-output-port session-fd))

;;; append this message to history file with time stamp
(define (history-raw filename message)
  "log the message to history file"
  (define history-file (open-file filename "a"))
  (display message history-file)
  (close-output-port history-file)
  ;; Hook messages to SESSION file
  (history-raw-session message))

;;; append this message to history file with time stamp
(define (history filename buddy message)
  "log the message to history file"
  (history-raw filename
               (string-append
                (local-date-time) " [" buddy "] " message "\n")))

(define (history-create-dirs)
  "create history folders"
  ;;; if we are running for the first time, then create
  ;;; ~/.freetalk/history
  (catch 'system-error
         (lambda ()
           (mkdir (string-append (ft-get-config-dir)
                                 "/history")))
         (lambda args #f))
  ;;; if we are logging in for first time, then create
  ;;; ~/.freetalk/history/<google-id>
  (catch 'system-error
         (lambda ()
           (mkdir history-path))
         (lambda args #f)))

;;; hook procedure for logging all sent messages
(define (log-sent-message to message)
  "hook procedure for logging all sent messages"
  (if (equal? enable-history-flag "yes")
    (begin
        (history (string-append history-path "/" to)
                 (ft-get-jid)
                 message))))

;;; hook procedure for logging all received messages
(define (log-received-message time from nickname message)
  "hook procedure for logging all received messages"
  (if (equal? enable-history-flag "yes")
    (begin
        (history (string-append history-path "/"
                                (regexp-substitute/global #f "/.*$" from
                                                          'pre "" 'post))
                 from
                 message))))

;;; hook procedure for logging all received offline messages
;(define (log-received-offline-message from message time)
;  "hook procedure for logging all received messages"
;  (define history-filename
;    (string-append history-path "/" from))
;  (define history-message
;    (string-append time " [" from "] [OFFLINE] " message "\n"))
;  (history-raw history-filename history-message))

(define (post-startup success)
  "post startup hook"
  (if success
    (begin
    (set! history-path (string-append
                        (ft-get-config-dir)
                        "/history/"
                        (ft-get-jid)))
    (set! session-file (string-append history-path "/SESSION"))
    (history-create-dirs))))

(define (post-disconnect reason)
  (and (= reason 0)
       (close-port (open-output-file session-file))))

;;; hook the logging procedures to send and reveice hooks
(add-hook! ft-message-send-hook log-sent-message)
(add-hook! ft-message-receive-hook log-received-message)
;(add-hook! fh-message-receive-offline-hook log-received-offline-message)
(add-hook! ft-login-hook post-startup)
(add-hook! ft-disconnect-hook post-disconnect)

(define (/history args)
  "less on history session"
;  (set! args (list->strlist args))
  (if (= (string-length args) 0)
      (system (string-append history-page-cmd session-file))
;      (if (= (string-length args) 1)
      (system (string-append history-page-cmd history-path "/"
                             (string-trim-right (string-trim-right
                                                 args #\space) #\:)))))

(add-command! /history "history" "/history [BUDDY]"
              "Display history page by page")

(define (/enable-logging args)
  " enable/disable conversations logging"
  (cond ((equal? args "no")
            (begin
              (set! enable-history-flag "no")
              (ft-display (_ " BUDDY history disabled "))))
        ((equal? args "yes")
            (begin
              (set! enable-history-flag "yes")
              (ft-display (_ " BUDDY history enabled "))))
	(else (ft-display (_ "Invalid syntax")))))

(add-command! /enable-logging "enable-logging" "/enable-logging"
              "enables (`yes') or disables (`no') conversation logging")
