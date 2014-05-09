;;; loudscream.scm: Guile interface to the Loudmouth library
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

(use-modules (ice-9 pretty-print))

(define-syntax node-set-attrs
  (syntax-rules ()
    ((_ node attrs) (if (null? attrs) '()
                        (map (lambda (attr)
                               (lm-message-node-set-attribute node (car attr)
                                                              (cadr attr)))
                             attrs)))))

(define-syntax node-transform
  (syntax-rules ()
    ((_ parent ()) '())
    ((_ parent ((name attrs childer) . rest)) ; childer is child of the child
     (let ((child (lm-message-node-add-child parent name "")))
       (node-set-attrs child attrs)
       (node-transform child childer)
       (node-transform parent rest)))
    ((_ parent (str)) (lm-message-node-set-value parent str))
    ((_ parent str) (lm-message-node-set-value parent str))))

(define-syntax iq
  (syntax-rules ()
    ((_ attrs . body) (let* ((m (lm-message-new "" 'iq))
                             (n (lm-message-get-node m)))
                        (node-set-attrs n attrs)
                        (node-transform n body)))))
(define-syntax presence
  (syntax-rules ()
    ((_ to attrs . body) (let* ((m (lm-message-new to 'presence))
                                (n (lm-message-get-node m)))
                           (node-set-attrs n (quote attrs))
                           (node-transform n body)
                           m))))

;;; Guile's macroexpand doesn't work for hygenic macros.
;;; This is the next best thing, although it has the unfortunate
;;; side-effect of *executing* the expanded code before showing
;;; the expansion. (Thanks, #guile)

(define-syntax expand
  (syntax-rules ()
    ((_ form) (let ((expand-aux-fn (lambda () form)))
                (expand-aux-fn)
                (pretty-print (procedure-source expand-aux-fn))))))

; (define-syntax expand
;   (syntax-rules ()
;     ((_ form) (unsyntax (syntax form (nearest-repl/environment))))))

(define-syntax my-macro
  (syntax-rules ()
      ((_) (+ 4 5))))

;;; XXX find a better place for this
(define go-to-repl #f)
(define seed #f)

(define (ft)
  (call-with-current-continuation go-to-repl))

(define (safe-repl args)
  (if (not seed)
      (set! seed (call-with-current-continuation
                  (lambda (escape)
                    (dynamic-wind
                        (lambda ()
                          (set! go-to-repl escape))
                        (lambda ()
                          (ft-give-repl)
                          #f ;; Never reached so second time on
                          ;; we always get back to same seed'ed repl
                          )
                        (lambda ()
                          (set! go-to-repl #f))
                      )
                    )))
      (seed)))

(add-command! safe-repl "repl" "/repl" "drop into a repl")
