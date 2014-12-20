;;; login.scm: connect and login to the server
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

(use-modules (ice-9 string-fun))
;(use-modules (ice-9 readline))
;(activate-readline)

(if (string=? (ft-get-jid) "")
     (ft-set-jid! (sans-surrounding-whitespace
                   (and (display "Jabber ID: ")
                        (read-line))))
     (display (string-append "[" (ft-get-jid) "] ")))

;; check if both user and domain are present
(if (not (string-rindex (ft-get-jid) #\@))
    (begin
      (display (string-append "freetalk: Jabber ID ["
                              (ft-get-jid)
                              "] should contain full user@domain\n"))
      (exit 1)))

(define (domain->server domain)
  (cond ((string=? domain "gmail.com") "talk.google.com")
        (else domain)))

(define (domain->port domain)
  (cond ((string=? domain "gmail.com") 5223)
        ((string=? domain "jabber.org") 5222)
        (else 5222)))

(define (domain->ssl domain)
  (cond ((string=? domain "gmail.com")
         (ft-set-sslconn! #t))
        (else "")))

(or (string=? (ft-get-jid) "")
    (and (string=? (ft-get-server) "")
         (split-discarding-char #\@ (ft-get-jid)
                                (lambda (jid domain)
                                  (ft-set-server! (domain->server domain))
                                  (ft-set-port! (domain->port domain))
                                  (domain->ssl domain)))))

(and (not (string=? (ft-get-jid) "")) (not (string=? (ft-get-server) ""))
     (ft-connect))

(add-hook! ft-login-hook (lambda (success)
                           (and success
                                (update-prompt))))

(add-hook! ft-disconnect-hook (lambda (reason)
                                (ft-set-prompt! "~\\/~ ")))
