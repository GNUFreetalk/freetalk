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

; (fh-register-command! '("dict" "dict [OPTIONS]\n\t- ask dictionary server"))

(define (/dict args)
  (if (= (string-length args) 0)
      (system "dict --help")
      (begin
        ; (fh-set-current-target-buddy! "dict" "send")
        (system (string-append "dict -P more \"" args "\"")))))

(add-command! /dict "dict" "/dict [OPTIONS] [WORD]" "lookup in dictionary")
