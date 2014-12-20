;;; connection.scm: talk to dict buddy
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

(define (connect-handle ret)
  (cond ((= ret 0) #t)
        ((= ret -6) (ft-display (_ "Already connected")))
        ((= ret -1) (ft-display (_ "Server not set")))
        ((= ret -2) (ft-display (_ "JID not set")))
        ((= ret -3) (ft-display (_ "SSL support not available")))
        ((= ret -5) (ft-display (_ "Proxy Server not set")))
        (else (ft-display (string-append (_ "Error, could not connect : ")
                                     (number->string ret))))))

(define (set-if-not-empty! set-fn! var default-var)
  (and (string=? var "")
       (set! var default-var))
  (if (string=? var "")
      #f
      (and (set-fn! var)
           #t)))

(define (read-line-clean)
  (sans-surrounding-whitespace (read-line)))

(define (jid->domain jid)
  (split-discarding-char #\@ jid (lambda (jid domain) domain)))

(define (domain->server domain)
  (cond ((string=? domain "jabber.org") "jabber.org")
        ((string=? domain "gmail.com") "talk.google.com")
        (else domain)))

(define (read-jid)
  (display (string-append "Jabber ID"
                          (if (not (string=? "" (ft-get-jid)))
                              (string-append "[" (ft-get-jid) "]")
                              "")
                          ": "))
  (set-if-not-empty! ft-set-jid! (read-line-clean) (ft-get-jid)))

(define (read-password)
  (set-if-not-empty! ft-set-password! (getpass "Password: ") ""))

(define (read-server)
  (let ((server (domain->server (jid->domain (ft-get-jid)))))
    (display (string-append "Server [" server "]: "))
    (set-if-not-empty! ft-set-server! (read-line-clean) server)))

(define (read-sslconn)
  (display (string-append (_ "Enable SSL (Y/N)? [Y]: ")))
  (let ((ans (read-line-clean)))
    (ft-set-sslconn! (if (or (string=? ans "n")
                             (string=? ans "N"))
                         #f
                         #t))))

(define (read-tlsconn)
  (display (string-append (_ "Enable TLS (Y/N)? [Y]: ")))
  (let ((ans (read-line-clean)))
    (ft-set-tlsconn! (if (or (string=? ans "n")
                             (string=? ans "N"))
                         #f
                         #t))))

(define (read-num-clean)
  (let ((port-str (read-line-clean)))
    (if (string->number port-str)
        port-str
        "")))

(define (read-port)
  (let ((port (if (ft-get-sslconn?)
                  5223
                  5222)))
    (display (string-append (_ "Port [") (number->string port) "]: "))
    (set-if-not-empty! (lambda (str-num)
                         (ft-set-port! (string->number str-num)))
                       (read-num-clean)
                       "0")))

(define (read-proxy)
  (display (string-append (_ "Enable Proxy (Y/N)? [Y]: ")))
  (let ((ans (read-line-clean)))
    (ft-set-proxy! (if (or (string=? ans "n")
                           (string=? ans "N"))
                       #f
                       #t))))

(define (read-proxyserver)
  (display (string-append (_ "ProxyServer: ")))
  (set-if-not-empty! ft-set-proxyserver! (read-line-clean) (ft-get-proxyserver)))

(define (read-proxyport)
  (let ((proxyport 8080))
    (display (string-append (_ "ProxyPort [") (number->string proxyport) "]: "))
    (set-if-not-empty! (lambda (str-num)
                         (ft-set-proxyport! (string->number str-num)))
                       (read-num-clean)
                       "8080")))

(define (read-proxyuname)
  (display (string-append (_ "ProxyUsername: ")))
  (set-if-not-empty! ft-set-proxyuname! (read-line-clean) (ft-get-proxyuname)))

(define (read-proxypasswd)
  (set-if-not-empty! ft-set-proxypasswd! (getpass "ProxyPassword: ") ""))

(define (/connect args)
  (connect-handle (ft-connect)))
(add-command! /connect "connect" "/connect"
              "connect to jabber server - non blocking")

(define (/login args)
  (and
   (if (> (ft-get-conn-status) 0)
       (begin
         (ft-display (_ "Already Logged in. /disconnect first"))
         #f))
    (read-jid)
    (read-password)
    (read-server)
    (read-sslconn)
    (read-tlsconn)
    (read-port)
    (read-proxy)
    (if (ft-get-proxy?)
        (begin
          (read-proxyserver)
          (read-proxyport)
          (read-proxyuname)
          (read-proxypasswd)
          ""))
    (connect-handle (ft-connect))))
(add-command! /login "login" "/login"
              "Interactive login to jabber server - blocking")

(define (/disconnect args)
  (ft-disconnect))
(add-command! /disconnect "disconnect" "/disconnect"
              "disconnect from jabber server")
(add-command! /disconnect "logout" "/logout"
              "logout from jabber server (same as disconnect)")

(define (/change-password args)
  (ft-change-password (sans-surrounding-whitespace args)))
(add-command! /change-password "change-password" "/change-password"
              "change password for current JID")

(define (login-cb success?)
  (if (not success?)
      (ft-display (_ "Could not login."))))

(add-hook! ft-login-hook login-cb)
