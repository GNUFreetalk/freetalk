;;; shell.scm: provides basic shell like facility
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

(define (/quit args)
  "exit freetalk"
  (ft-quit 0))
(add-command! /quit "quit" "quit" "quit this messenger")

(define (/shell args)
  "dynamic command interface to shell facility"
  (if (= (string-length args) 0)
      (begin
        (display (_ "Press \"C-d\" to get back to freetalk"))
        (newline)
        (system "sh"))
      (system args)))

(add-command! /shell "shell" "/shell [COMMAND] [ARGS]" "shell mode")

(define (/restart args)
  "dynamic command interface to /restart facility"
  (ft-disconnect)
  (apply execlp "freetalk" "freetalk" '()))

(add-command! /restart "restart" "/restart" "restart freetalk")

(define (/date args)
  "dynamic command interface to /date facility"
  (if (= (string-length args) 0)
      (system "date")
      (system (string-append "date " args))))

(add-command! /date "date" "/date [OPTIONS]" "print current date with all date OPTIONS")

(add-hook! ft-quit-hook (lambda (dummy) (display
                                         (string-append "         ...   ...                            \n"
                                                        "       ..         ..                          \n"
                                                        "       ..          ..                         \n"
                                                        "        ...~~`'~~...                          \n"
                                                        "         '(0##(0).***                         \n"
                                                        "          |##......*******......-_            \n"
                                                        "          |##......................           \n"
                                                        "          ##./ `.....Freetalk........         \n"
                                                        "         (--)  `.................   ..        \n"
                                                        "          ##   `.................     **      \n"
                                                        "                .............. .       **     \n"
                                                        "                .....    v .. ..        `*    \n"
                                                        "                `. ..     ......              \n"
                                                        "                 ....      .. ..              \n"
                                                        "                 ....       .. ..             \n"
                                                        "                 WW WW      WW WW             \n"
                                                        "   ----------------------------------------   \n"
                                                        "         Thank you for using freetalk         \n"
                                                        "   ----------------------------------------   \n"))))


(define (message-from-reason reason)
  (cond ((= reason 0) "User Request")
        ((= reason 1) "Network error (Timeout)")
        ((= reason 2) "Protocol error (Hangup)")
        ((= reason 3) (cond ((= (ft-get-conn-status) 3)
                             "Possible login from another location")
                            (else "Invalid authentication")))
        (else (string-append "Unknown error: " (number->string reason)))))

(add-hook! ft-disconnect-hook (lambda (reason)
                                (ft-display (string-append
                                             "Disconnected from "
                                             (ft-get-server)
                                             ": "
                                             (message-from-reason reason)))
                                (if (= reason 1)
                                    (ft-connect))))

