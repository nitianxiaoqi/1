all:
	gcc login.c cgic.c -o login.cgi -lsqlite3
	gcc huichang_env_view.c cgic.c -o huichang_env_view.cgi -static
install:
	sudo cp login.cgi huichang_env_view.cgi ../cgi-bin
	rm -rf login.cgi huichang_env_view.cgi
