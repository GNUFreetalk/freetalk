;;; beep.scm: Beep on specific events.
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

(if (= (system "beep -f 1 -l 0 >> /dev/null 2>&1") 0)
    (add-hook! ft-message-receive-hook
               (lambda (time from nickname message)
                 (system "beep -f 600 -l 10; beep -f 800 -l 10; beep -f 200 -l 10&")))
    (add-hook! ft-message-receive-hook
               (lambda (time from nickname message)
                 (ft-beep 200 20))))
