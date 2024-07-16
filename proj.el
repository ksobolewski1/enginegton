
;; load all files for the precomp program
(defun precomp-files ()
  (interactive)
  (cd root-dir)
  (split-tab "precomp" "./inc/data.h" "./build/precomp.c")) 

;; load build files, main, and core.h
(defun main-files ()
  (interactive)
  (cd root-dir)
  (named-tab "makefile")
  (find-file "makefile")
  (cd root-dir)
  (split-tab "main" "./src/main.c" "./inc/core.h"))

;; main files
(defun move-files ()
  (interactive)
  (cd root-dir)
  (split-tab "moves" "./inc/moves.h" "./src/moves.c"))

(defun struct-files ()
  (interactive)
  (cd root-dir)
  (split-tab "pos" "./inc/pos.h" "./src/pos.c")
  (split-tab "piece" "./inc/piece.h" "./src/piece.c"))

;; mqueue and engio
(defun misc-files ()
  (interactive)
  (cd root-dir)
  (split-tab "queue" "./inc/mqueue.h" "./src/mqueue.c")
  (split-tab "io" "./inc/engio.h" "./src/engio.c"))

;; load all code
(defun old-code ()
  (interactive)
  (cd root-dir)
  (split-tab "old" "old/PieceGroup.cpp" "old/Enginegton.cpp"))

;; load all files in the test workflow 
(defun test-files ()
  (interactive)
  (cd root-dir)
  (split-tab "scripts" "./test/pgn.py" "./test/moves.py")
  (split-tab "unit" "./test/inc/unit.h" "./test/src/unit.c")
  (split-tab "helpers" "./test/inc/helpers.h" "./test/src/helpers.c"))
  
(find-file "eng.org")
