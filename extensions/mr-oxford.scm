;;; mr-oxford.scm: Remembers and completes all known words from
;;;               disctionary and what ever you have typed.
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


(display "Loading dictionary [/usr/share/dict/words]... ")
(let ((word-count 0))
  (catch 'system-error
         (lambda ()
           (for-each-line (lambda (word)
                            (ft-dict-prepend! word)
                            (set! word-count (1+ word-count)))
                          "/usr/share/dict/words"))
         (lambda args #f))
  (display (string-append "[" (number->string word-count) "] words\n")))

;; Add sent-words to readline context"

(add-hook! ft-message-send-hook
           (lambda (to msg)
             (map ft-dict-insert! (sentence->words msg))))
