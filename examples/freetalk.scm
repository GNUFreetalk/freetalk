;; Sample ~/.freetalk/freetalk.scm

;; Passwordless automatic login
; (and (string=? (ft-get-jid) "")
;      (ft-set-jid! "yourid@jabber.org")
;      (ft-set-password! "p4ssw0rd")
;      (ft-set-sslconn! #t)
;      (ft-set-prompt! "freetalk: ")
;      (ft-set-server! "talk.jabber.org"))

;; Example proxy configuration
; (and (string=? (ft-get-jid) "")
;      (ft-set-jid! "yourid@gmail.com")
;      (ft-set-password! "p4ssw0rd")
;      (ft-set-tlsconn! #t)
;      (ft-set-prompt! "freetalk: ")
;      (ft-set-server! "talk.google.com")
;      (ft-set-proxyserver! "your.proxyserver.net")
;      (ft-set-proxyport!  "8080"))

;; Get control after successful login
; (add-hook! ft-login-hook
;            (lambda (status)
;              (if status
;                  (begin
                     ;; Set user-id as your prompt
;                    (ft-set-prompt! (string-append
;                                     (car (string-split (ft-get-jid) #\@))
;                                     ": "))
                     ;; Change status to "do not disturb"
;                    (ft-set-status-msg! "dnd")))))


;; Key bindings
;; Let ctrl-a display full roster, ctrl-e who i am
; (ft-bind-to-ctrl-key #\a "(/who \"all\")")
; (ft-bind-to-ctrl-key #\e "(/whoami \"\")")
