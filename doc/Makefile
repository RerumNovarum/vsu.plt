LATEX = pdflatex
ASY = asy

all: 3d.pdf

3d.pdf: coordinates.pdf

%.pdf: %.tex
	$(LATEX) $<
	$(LATEX) $<
	$(LATEX) $<

%.pdf: %.asy
	$(ASY) $< -outname $@
