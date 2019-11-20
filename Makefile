fits-debayer:
	gcc -o fits-debayer fits-debayer.c -L. -lcfitsio

clean:
	rm fits-debayer
