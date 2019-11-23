fits-debayer: src/fits-debayer.c
	gcc -o fits-debayer src/fits-debayer.c -L. -lcfitsio

clean:
	rm fits-debayer
