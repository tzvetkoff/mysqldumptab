all: mysqldumptab

mysqldumptab: mysqldumptab.c mdt_str.c mdt_str.h
	$(CC) -O3 -o mysqldumptab mysqldumptab.c mdt_str.c `mysql_config --cflags` `mysql_config --libs`

test: mysqldumptab
	./tests/run.sh

clean:
	$(RM) mysqldumptab
