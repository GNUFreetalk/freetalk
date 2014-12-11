;;; utils.scm: common utility procedures
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

;;; utils.scm should be automatically loaded thru init.scm

(use-modules (srfi srfi-13))

(define (/version args)
  "display version info"
  (display (string-append (_ "freetalk (FreeTalk) ") (ft-version) "\n"
                          (_ "Copyright (C) 2005-2014 FreeTalk Core Team.\n")
                          (_ "This is free software; see the source for copying conditions.\n")
                          (_ "There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"))))

(add-command! /version "version" "/version" "display freetalk version information")

(define (/run-cmd args)
  "run commands"
  (ft-run-command args))
(add-command! /run-cmd "run-cmd" "/run-cmd" "run command provided in args")

;; range with proper tail recursion --ab
(define (range start end)
  "return a list of numbers in the given range"
  (letrec
      ((local-range
        (lambda (start end range-list)
          (cond
           ((= start end) (append range-list (list end)))
           ((< start end) (local-range (+ start 1) end (append range-list (list start))))
           (else '())))))
    (local-range start end '())))

(define (any->symbol item)
  "converts a number into a symbol"
  (if (number? item)
      (string->symbol (number->string item))
      (if (string? item)
          (string->symbol item)
          item)))

(define (list->asv li any)
  "convert list to any separated vector"
  (if (=(length li) 1)
      (car li)
      (string-append (car li) any (list->asv (cdr li) any))))

(define (list->csv li)
  "convert list to comma separated vector"
  (list->asv li ", "))

(define (list->symlist li)
  "converts a list of numbers/symbols to list of symbols"
  (if (= (length li) 1)
      (list (any->symbol (car li)))
      (append (list (any->symbol (car li))) (list->symlist (cdr li)))))

(define (list->strlist li)
  "converts a list of numbers/symbols to list of strings"
  (if (= (length li) 0)
      '()
      (if (= (length li) 1)
          (list
           (symbol->string
            (any->symbol (car li))))
          (append (list
                   (symbol->string
                    (any->symbol (car li))))
                  (list->strlist (cdr li))))))

(define (send-messages-to-all roster-list message)
  "Send messages to all"
  (for-each (lambda (buddy)
              (ft-send-message-no-hook buddy message))
            roster-list))

(define (local-date-time)
  (string-append
                 (strftime "%F" (localtime (current-time)))
                 " "
                 (strftime "%T" (localtime (current-time)))))

(define (skip-comment port)
  (let ((char (peek-char port)))
    (if (or (eof-object? char)
            (char=? #\newline char))
        char
        (begin
          (read-char port)
          (skip-comment port)))))

(define (skip-whitespace port)
  (let ((char (peek-char port)))
    (cond ((or (eof-object? char)
               (char=? #\newline char))
           char)
          ((char-whitespace? char)
           (read-char port)
           (skip-whitespace port))
;;        ((char=? #\# char)
;;         (read-char port)
;;         (skip-comment port))
          (else char))))

(define (read-token port delim)
  (letrec
      ((loop
        (lambda (chars)
          (let ((char (peek-char port)))
            (cond ((eof-object? char)
                   (do-eof char chars))
                  ((char=? #\newline char)
                   (do-eot chars))
                  ((char-whitespace? char)
                   (do-eot chars))
;                  (let ((terminator (skip-comment port)))
;;                   (if (eof-object? char)
;;                       (do-eof char chars)
;;                       (do-eot chars))))
                  (else
                   (read-char port)
                   (loop (cons char chars)))))))
       (do-eof
        (lambda (eof chars)
          (if (null? chars)
              eof
              (do-eot chars))))
       (do-eot
        (lambda (chars)
          (if (null? chars)
              #f
              (list->string (reverse! chars))))))
    (skip-whitespace port)
    (loop '())))

(define (sentence->words sentence)
  "convert sentence to list of words"
  (with-input-from-string sentence
    (lambda ()
      (letrec
          ((next-token
            (lambda ()
              (read-token (current-input-port) #\#)))
           (append-word
            (lambda (word-list)
              (let
                  ((word (next-token)))
                (if (eof-object? word)
                    word-list
                    (begin
                      (append-word
                       (append word-list (list word)))))))))
        (append-word '())))))


(define (blank-line? line)
  "return true if line is blank"
  (null? (sentence->words line)))

;;; string utils
;; for string-match procedure
(use-modules (ice-9 regex))
(use-modules (ice-9 string-fun))
(use-modules (ice-9 format))

(define (sentence->tokens sentence)
  "convert string to list of tokens"
  (with-input-from-string sentence
    (lambda ()
      (letrec
          ((next-token (lambda ()
                         (read (current-input-port))))
           (append-word (lambda (word-list)
                          (let
                              ((word (next-token)))
                            (if (eof-object? word)
                                word-list
                                (begin
                                  (append-word (append word-list
                                                       (list word)))))))))
        (append-word '())))))


(define (list->sentence li)
  "convert list to space separated sentence"
  (let
      ((word (if (symbol? (car li))
                 (symbol->string (car li))
                 (number->string (car li)))))
    (if (= (length li) 1)
        word
        (string-append word " " (list->sentence (cdr li))))))


(define (blank-line? line)
  "return true if line is blank"
  (or (= 0 (string-length line))
      (null? (sentence->tokens line))))


(define (read-next-line fport)
  "skip blank lines and read the next line"
  (let ((line (read-line fport)))
    (if (eof-object? line)
        ""
        (if (not (blank-line? line))
            line
            (read-next-line fport)))))


(define (string-string? str find-string)
  "returns true if find-string is a substring of str"
  (vector? (string-match find-string str)))


(define (date->list date)
  "convert flat yymmdd num to (yy mm dd) list"
  (let*
      ((date-str (list->string
                  (map (lambda (c)
                         (if (eq? c #\space) #\0 c))
                       (string->list (format #f "~6d" date)))))
       (yy (substring date-str 0 2))
       (mm (substring date-str 2 4))
       (dd (substring date-str 4 6)))
    (map string->number (list yy mm dd))))

(define (for-each-line proc filename)
  "call PROC for each line in FILENAME"
  (with-input-from-file filename
    (lambda ()
      (do ((line (read-line) (read-line)))
          ((eof-object? line))
        (proc line)))))
