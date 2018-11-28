papman - Your's truly, the command line PAPer MANager
=====================================================

Papman is a command line PAPer MANager to help you sort, search and manage
your paper collection, and especially export biblatex style citations.

A FAIR WORD OF WARNING
----------------------

Papman was conceived out of deep procrastination in order to acquire and manage
an overview of the current state of the art in relevant fields, while its
author should obviously have been busy writing his dissertation instead, but
very much disliked all other readily available solutions for this gargantuan
task.

Being a work of academic endeavour, papman has been, in the traditional style
of almost all academic works, created in an utter rush at the last possible
moment. Thus consider every feature unstable, the code to be a chaotic mess,
expect dragons everywhere(TM) and enjoy every second this tool doesn't
completely destroy your whole library. That being said...I should be off
writing and not fixing bugs!

DESIGN CHOICES
--------------

 * do use PLAIN bibtex style files
 * do NO single central indexing (as long as speed allows for it)
 * do NOT assume anything about filenames or bibliography entry ids, but...
 * do TRY to enforce consistency, i.e., \<id> uses \<id>.pdf and \<id>.bib

LOCATIONS
---------
Papman's default config is in `$HOME/.config/papman/conf` and it's default
bibliography is set to `$HOME/bib`.

GIT-PRE-COMMIT-HOOK
-------------------
If you archive/distribute/manage/share your bibliography using git, there's a
git pre-commit hook in `git.hook.pre-commit` that you can drop into
`<BIB>/.git/hooks/pre-commit`. It will compress new committed PDF files (using
ghostscript), extract the PDF into a TXT file (using pdftotext) and (re-)format
new bib entries (using papman, of course).

TODO
----
 * BUG: prevent duplicate entry addition/creation
 * FTR: bibtex: add possibility to sort output
 * FTR: conf: create in .config/papman/conf and use bibtex format
 * FTR: log + verbosity level!
 * FTR: handle multiple item matches in all actions -> edit, open, show, bibtex
 * FTR: add "sources" to actions -> list, edit, show, open(url)
 * FTR: list references
 * FTR: stats (#, per year/conference/author)
 * FTR: author details (#, chronological order)
