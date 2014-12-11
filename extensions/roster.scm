;;; roster.scm - Roster related commands
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

(define (/add args)
  (if (= (string-length args) 0)
      (ft-display (_ "Incomplete syntax"))
      (begin
        (ft-add-buddy! (string-trim-right (sans-surrounding-whitespace
                                           args) #\:))
        (ft-subscription-allow (string-trim-right (sans-surrounding-whitespace
                                                   args) #\:)))))

(add-command! /add "add" "/add [USER@SERVER]" "add new buddy to list")

(define (/remove args)
  (if (= (string-length args) 0)
      (ft-display (_ "Incomplete syntax"))
      (begin
        (ft-remove-buddy! (string-trim-right (sans-surrounding-whitespace
                                              args) #\:))
        (ft-subscription-deny (string-trim-right (sans-surrounding-whitespace
                                                  args) #\:)))))

(add-command! /remove "remove" "/remove [USER@SERVER]"
              "remove buddy from list")

(define (pretty-print-show-msg msg)
  (cond
    ((string-ci=? msg "chat") "Chatty")
    ((string-ci=? msg "away") "Away")
    ((string-ci=? msg "xa") "Extended Away")
    ((string-ci=? msg "dnd") "Do not Disturb")
    (else msg)))

(define (/who args)
  (for-each (lambda (item)
              (let ((jid (list-ref item 0))
                    (online (list-ref item 1))
                    (nickname (list-ref item 2))
                    (show-msg (list-ref item 3))
                    (status-msg (list-ref item 4)))
                (if online
                    (ft-display (string-append " * " jid (if (> (string-length
                                                                 nickname) 0)
                                                             (string-append
                                                              " (" nickname ") ")
                                                             " ")
                                         (if (> (string-length show-msg) 0)
                                             (string-append "-> ["
                                                            (pretty-print-show-msg show-msg) "]")
                                             "")
                                         (if (> (string-length status-msg) 0)
                                             (string-append " (" status-msg ")")
                                             "")))
                    (and (string=? args "all") (ft-display (string-append "   " jid (if (> (string-length nickname) 0)
                                                                                        (string-append " (" nickname ") ")
                                                                                        " ")))))))
            (ft-get-roster-list)))

(add-command! /who "who" "/who" "display buddy list")

(define (/whoami args)
  (ft-display (string-append (_ "Jabber ID: ") (ft-get-jid) "\n"
                             (_ "Jabber Server: ") (ft-get-server) "\n"
                             (_ "Status: ") (ft-get-status-msg))))

(add-command! /whoami "whoami" "/whoami" "display who is this")

(define (/allow args)
  (if (= (string-length args) 0)
      (ft-display (_ "Incomplete syntax"))
      (ft-subscription-allow (string-trim-right (sans-surrounding-whitespace
                                                 args) #\:))))

(add-command! /allow "allow" "/allow [USER@SERVER]" "Allow buddy to see your status")

(define (/deny args)
  (if (= (string-length args) 0)
      (ft-display (_ "Incomplete syntax"))
      (ft-subscription-deny (string-trim-right (sans-surrounding-whitespace
                                                args) #\:))))
(add-command! /deny "deny" "/deny [USER@SERVER]" "Deny buddy permission to see your status")

(define (/alias args)
  (if (= (string-length args) 0)
      (ft-display (_ "Incomplete syntax"))
      (apply ft-roster-set-nickname (map sans-surrounding-whitespace
             (split-discarding-char #\space (sans-surrounding-whitespace args)
                                    list)))))

(add-command! /alias "alias" "/alias buddy nickname" "Set the nickname of a buddy")

(define (subscribe-recv jid)
  (ft-display (string-append (_ "[Buddy request recieved from ") jid (_ " use /allow or /deny]")))
  (ft-display (string-append (_ "[Use /add ") jid (_ " to add him/her to your buddy list]"))))

(add-hook! ft-subscribe-receive-hook subscribe-recv)
