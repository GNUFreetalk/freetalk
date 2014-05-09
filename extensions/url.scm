;;; url.scm: pipe session to urlview
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

(define urlview-cmd "urlview ")

;;; history path
(define url-history-path "HISTORY PATH WILL BE SET IN POST-STARTUP")
(define url-session-file "SESSION FILE NAME WILL BE SET IN POST-STARTUP")

(define (url-post-startup success)
  "post startup hook"
  (if success
    (begin
    (set! url-history-path (string-append
                            (ft-get-config-dir)
                            "/history/"
                            (ft-get-jid)))
    (set! url-session-file (string-append history-path "/SESSION")))))

(add-hook! ft-login-hook url-post-startup)

(define (/urlview args)
  "urlview history session"
;  (set! args (list->strlist args))
  (if (= (string-length args) 0)
      (system (string-append urlview-cmd url-session-file))
;      (if (= (string-length args) 1)
      (system (string-append urlview-cmd url-history-path "/"
                             (string-trim-right (string-trim-right
                                                 args #\space) #\:)))))

(add-command! /urlview "urlview" "/urlview [BUDDY]" "handle URLs")
