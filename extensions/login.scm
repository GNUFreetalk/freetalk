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

(if (and (not (string=? (ft-get-jid) "")) (not (string=? (ft-get-server) "")))
     (ft-connect)
     (/login ""))

(add-hook! ft-login-hook (lambda (success)
                           (and success
                                (ft-set-prompt! (string-append
                                                 (ft-get-jid)
                                                 "> ")))))

(add-hook! ft-disconnect-hook (lambda (reason)
                                (ft-set-prompt! "~\\/~ ")))
