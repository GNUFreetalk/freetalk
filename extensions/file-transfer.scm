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

;;;; file-transfer support

(define (/send-file args)
  "transfer file to a buddy"
  (if (= (string-length args) 0)
    (ft-display (_ "No file to send"))
    (begin
	(split-discarding-char #\space
			       (sans-surrounding-whitespace args)
			       (lambda (id file)
				 (and (if (> (string-length id) 0)
					  #t
					  (begin
					    (display (_ "Bad ID\n"))
					    #f))
				      (if (> (string-length file) 0)
					  #t
					  (begin
					    (display (_ "Invalid filename\n"))
					    #f))
				      (ft-send-file id file)))))))

(add-command! /send-file "/send-file" "/send-file [USER@SERVER]" "send a file to a buddy")

;;; when a buddy sends a file, we handle it in this way
(define (/allow-file args)
  "Allow a buddy to transfer file"
  (split-discarding-char #\space
			 (sans-surrounding-whitespace args)
			 (lambda (cookie file)
			   (if (> (string-length cookie) 0)
			       (ft-set-allow-file (string->number cookie) file)
			       (ft-display (_ "Missing cookie number")))))
  (remove-command! "/allow-file"))

(define (notify-file jid file errno cookie)
  (if (= errno 1)
      (begin
	(add-command! /allow-file "/allow-file" "/allow-file" "Allow transfer of file from far end")
	(ft-display (string-append "[" jid " wants to send file " file "]"))
	(ft-display (string-append (_ "[use /allow-file ") (number->string cookie) (_ " [file-name] to accept]"))))
      (begin
	(if (= errno 2)
	    (ft-display (string-append "[" jid (_ " does not support IBB for file transfer]")))
	    (ft-display (string-append (_ "[failed to open file ") file "]"))))))

(add-hook! ft-notify-file-hook notify-file)