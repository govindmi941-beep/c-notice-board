# Linux / macOS / AWS EC2:  run "make", then "./noticeboard"
noticeboard: server.c notice.c http.c notice.h http.h
	gcc -Wall -Wextra -o noticeboard server.c notice.c http.c

clean:
	rm -f noticeboard notices.dat notices.tmp
