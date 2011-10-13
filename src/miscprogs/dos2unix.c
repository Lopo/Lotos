#include <stdio.h> 

main(int argc, char *argv[])
{
	FILE *ifp, *ofp;
	char c, filename[1024], fname[1024];

	if (argc<2) {
		printf("Chyba: nezadany vstupny subor !\n");
		return;
		}
	if ((ifp=fopen(argv[1], "r"))==NULL) {
		printf("Chyba: nemozem otvorit vstupny subor '%s'\n", argv[1]);
		return;
		}
	if (argc<3) {
		printf("Upozornenie: nezadany vystupny subor,\noriginal subor bude ulozeny ako %s.bak\n", argv[1]);
		sprintf(filename, "%s.tmp", argv[1]);
		if ((ofp=fopen(filename, "w"))==NULL) {
			printf("Chyba: nemozem otvorit subor '%s' !\n", argv[1]);
			fclose(ifp);
			return;
			}
		}
	else {
		if ((ofp=fopen(argv[2], "w"))==NULL) {
			printf("Chyba: nemozem otvorit vystupny subor '%s'\n", argv[2]);
			fclose(ifp);
			return;
			}
		}

	c=getc(ifp);
	while(!feof(ifp)) {
		if (c!='\r') putc(c, ofp);
		c=getc(ifp);
		}

	fclose(ifp);
	fclose(ofp);
	if (argc==2) {
		sprintf(filename, "%s.bak", argv[1]);
		rename(argv[1], filename);
		sprintf(filename, "%s.tmp", argv[1]);
		rename(filename, argv[1]);
		}

	printf("Subor bol uspesne prevedeny\n");
}