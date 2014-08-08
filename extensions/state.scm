;;; dist-buddy.scm: talk to dict buddy
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

(define (usual-crap get-fn set-fn! get-msg args)
  (if (= (string-length args) 0)
      (ft-display (string-append get-msg (get-fn)))
      (set-fn! args)))

(define (/prompt args)
  (usual-crap ft-get-prompt ft-set-prompt! "Current prompt: " args))
(add-command! /prompt "prompt" "/prompt [NEWPROMPT]" "set command line prompt")

(define (/status args)
  (usual-crap ft-get-status-msg ft-set-status-msg! "Current status: " args))
(add-command! /status "status"
              "/status [online|away|chat|xa|dnd|invisible][/PRIORITY] [MESSAGE]"
              "set status message (and optionally resource priority)")

(define (/server args)
  (usual-crap ft-get-server ft-set-server! "Current server: " args))
(add-command! /server "server" "/server [HOST|IP]"
              "set server for next /connect")

(define (/jid args)
  (usual-crap ft-get-jid ft-set-jid! "Current JID: " args))
(add-command! /jid "jid" "/jid [USER@SERVER]" "set jabber id for next /connect")

(define (/password args)
  (ft-set-password! (getpass "Password: ")))
(add-command! /password "password" "/password" "set password for next /connect")

(define (/port args)
  (usual-crap
   (lambda () (number->string (ft-get-port)))
   (lambda (str_port) (ft-set-port! (string->number str_port)))
   "Current Port (0 = default): "
   args))
(add-command! /port "port" "/port [PORT]" "set server port for next /connect")

(define (/proxyserver args)
  (usual-crap ft-get-proxyserver ft-set-proxyserver!
              "Current ProxyServer: "
              args))
(add-command! /proxyserver "proxyserver"
              "/proxyserver [HOST|IP]" "set proxy server for next /connect")

(define (/proxyport args)
  (usual-crap
   (lambda () (number->string (ft-get-proxyport)))
   (lambda (str_proxyport) (ft-set-proxyport! (string->number str_proxyport)))
   "Current Port (8080 = default): "
   args))
(add-command! /proxyport "proxyport" "/proxyport [PORT]"
              "set proxyserver port for next /connect")

(define (/proxyuname args)
  (usual-crap ft-get-proxyuname ft-set-proxyuname! "Current ProxyUname: " args))
(add-command! /proxyuname "proxyuname" "/proxyuname [PROXYUSERNAME]"
              "set proxy username for next /connect")

(define (/proxypasswd args)
  (ft-set-proxypasswd! (getpass "ProxyPassword: ")))
(add-command! /proxypasswd "proxypasswd" "/proxypasswd"
              "set proxy password for next /connect")

(add-command! (lambda (str)
                (if (> (string-length str) 0)
                    (ft-load (sans-surrounding-whitespace str))
                    (ft-display (_ "usage: /load [FILE]"))))
              "load" "/load [FILE]" "load an extension file")

(add-command! (lambda (args) (ft-reset-fs-state!)) "setup"
              "/setup" "Write fresh ~/.freetalk")
