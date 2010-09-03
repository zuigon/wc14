
all: man

man:
	ronn --html --pipe wc14.ronn > index.html
