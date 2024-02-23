all:
	gcc login.c cgic.c -o login.cgi -lsqlite3
	gcc living_view.c cgic.c -o living_view.cgi
	gcc living_set.c cgic.c -o living_set.cgi
	gcc living_dev.c cgic.c -o living_dev.cgi
install:
	sudo mv *.cgi ../cgi-bin
