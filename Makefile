build:
	gcc main.c smtp.c -DSMTP_OPENSSL -lssl -lcrypto -o invisible_friend.out
