TEX = tex -interaction=nonstopmode
LATEX = pdflatex -interaction=nonstopmode

default: .NEWS rjlnewsp4.cls rjlnewsp4.pdf Manual.pdf Readme.pdf

# Build the C++ program by shelling out to another makefile:
.NEWS:
	make -kC src

# LaTeX bit

rjlnewsp4.pdf : rjlnewsp4.dtx
	$(LATEX) rjlnewsp4.dtx && $(LATEX) rjlnewsp4.dtx

rjlnewsp4.cls : rjlnewsp4.ins rjlnewsp4.dtx
	rm -f rjlnewsp4.cls; \
	$(TEX) rjlnewsp4.ins

Manual.pdf : Manual.tex
	$(LATEX) Manual.tex

Readme.pdf : Readme.tex
	$(LATEX) Readme.tex


# Utility

clean :
	rm -f *.o *.log *.aux *.dvi *.glo *.idx

test : .NEWS rjlnewsp4.cls testpage/newspaper.tex
	cd testpage && \
	../src/news -file newspaper.tex -output-directory .. -verbose t --stage all && \
	cat newspaper.lay
