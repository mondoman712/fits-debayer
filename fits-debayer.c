#include <string.h>
#include <stdio.h>
#include <fitsio.h>
#include <unistd.h>

long * allocate_long_array (int size)
{
	long * longarray = (long *) malloc(sizeof(long) * size);

	for (int i = 0; i < size; ++i)
		longarray[i] = 0;

	return longarray;
}

/* Adds "_colour" to a .fits filename */
char * get_filename (char * before, char colour)
{
	char * out = (char *) malloc(sizeof(char) * strlen(before) + 3);
	memcpy(out, before, sizeof(char) * strlen(before));
	out = strtok(out, ".");
	sprintf(out, "%s_%c.fits", out, colour);
	
	return out;
}

int copy_fits_header (fitsfile ** src, fitsfile ** dst)
{
	char card[FLEN_CARD];
	int status = 0, nkeys;

	fits_get_hdrspace(*src, &nkeys, NULL, &status);

	for (int i = 1; i < nkeys; i++) {
		fits_read_record(*src, i, card, &status);
		fits_write_record(*dst, card, &status);

		if (status)
			fits_report_error(stderr, status);
	}

	return status;
}

int debayer_image(char * filename)
{
	/* Load File */
	fitsfile * fptr;
	int status = 0;

	fits_open_file(&fptr, filename, READONLY, &status);

	/* Get image size */
	long naxis[2];
	fits_get_img_size(fptr, 2, naxis, &status);

	/* Read image */
	long * imagearr = allocate_long_array(naxis[0] * naxis[1]);
	long longnull = 0;

	long fpixel[2], nelements;
	fpixel[0] = fpixel[1] = 1;
	nelements = naxis[0] * naxis[1];

	fits_read_pix(fptr, TLONG, fpixel, nelements, &longnull, imagearr,
			NULL, &status);

	/* Save red channel */
	long * red = allocate_long_array(naxis[0] * naxis[1]);
	memcpy(red, imagearr, sizeof(long) * naxis[0] * naxis[1]);
	int pos = 0;

	for (int ri = 0; ri < naxis[1]; ri += 2) {
		for (int rj = 0; rj < naxis[0]; rj += 2) {
			pos = (ri * naxis[0]) + rj;
			red[pos + 1] = red[pos];
			red[pos + naxis[0]] = red[pos];
			red[pos + naxis[0] + 1] = red[pos];
		}
	}

	fitsfile * red_fp;
	char * rfname = get_filename(filename, 'r');

	if (access(rfname, W_OK) != -1)
		fits_open_file(&red_fp, rfname, READWRITE, &status);
	else
		fits_create_file(&red_fp, rfname, &status);

	copy_fits_header(&fptr, &red_fp); 
	fits_write_pix(red_fp, TLONG, fpixel, nelements, red, &status);
	fits_close_file(red_fp, &status);
	free(red);

	/* Save green channel */
	fits_read_pix(fptr, TLONG, fpixel, nelements, &longnull, imagearr,
			NULL, &status);

	long * green = allocate_long_array(naxis[0] * naxis[1]);
	memcpy(green, imagearr, sizeof(long) * naxis[0] * naxis[1]);

	for (int gi = 1; gi < naxis[1]; gi++) {
		for (int gj = 1; gj < naxis[0]; gj += 2) {
			pos = (gi * naxis[0]) + gj;
			green[pos - 1] = green[pos];
		}

		gi++;

		for (int gj = 0; gj < naxis[0]; gj += 2) {
			pos = (gi * naxis[0]) + gj;
			green[pos + 1] = green[pos];
		}
	}

	fitsfile * green_fp;
	char * gfname = get_filename(filename, 'g');

	if (access(gfname, W_OK) != -1)
		fits_open_file(&green_fp, gfname, READWRITE, &status);
	else
		fits_create_file(&green_fp, gfname, &status);

	copy_fits_header(&fptr, &green_fp); 
	fits_write_pix(green_fp, TLONG, fpixel, nelements, green, &status);
	fits_close_file(green_fp, &status);
	free(green);

	/* Save blue channel */
	fits_read_pix(fptr, TLONG, fpixel, nelements, &longnull, imagearr,
			NULL, &status);

	long * blue = allocate_long_array(naxis[0] * naxis[1]);
	memcpy(blue, imagearr, sizeof(long) * naxis[0] * naxis[1]);

	for (int bi = 1; bi < naxis[1]; bi += 2) {
			for (int bj = 0; bj < naxis[0]; bj += 2) {
					pos = (bi * naxis[0]) + bj;
					blue[pos + 1] = blue[pos];
					blue[pos + naxis[0]] = blue[pos];
					blue[pos + naxis[0] + 1] = blue[pos];
			}
	}

	fitsfile * blue_fp;
	char * bfname = get_filename(filename, 'b');

	if (access(bfname, W_OK) != -1)
		fits_open_file(&blue_fp, bfname, READWRITE, &status);
	else
		fits_create_file(&blue_fp, bfname, &status);

	copy_fits_header(&fptr, &blue_fp); 
	fits_write_pix(blue_fp, TLONG, fpixel, nelements, blue, &status);
	fits_close_file(blue_fp, &status);
	free(blue);

	fits_close_file(fptr, &status);

	if (status)
		fits_report_error(stdout, status);

	return(status);
}

int main (int argc, char ** argv)
{
	int status = 0;

	if (!argv[1]) {
		fprintf(stderr, "Filename missing\n");
		return 1;
	}

	for (int i = 1; i < argc; i++) {
		status = debayer_image(argv[i]);

		if (status)
			fits_report_error(stdout, status);
	}

	return status;
}

// vim: tabstop=4 softtabstop=0 noexpandtab shiftwidth=4
