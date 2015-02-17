# mysqldumptab

A console tool that dumps MySQL tables as whatever-separated data.

## TL;DR;

`mysqldump --tab` actually calls `SELECT * INTO OUTFILE` on the *server*, so it's not really useful when you need to dump a table from a remote server.

`mysqldumptab` leverages that as it dumps a table in the exact same format as `SELECT INTO OUTFILE` but does that on the client side.

## Installation

No install scripts right now. Just do:

``` bash
make
cp mysqldumptab /usr/local/bin
```

in a terminal and you're good to go.

## Usage

Some options like `-uphPs` are the same as `mysqldump`.

If you pass `-p` with no argument, you'll get a password prompt.

```
Usage:
  ./mysqldumptab [options] <database> <table>

Options:
  -u, --username                    set mysql user []
  -p, --password                    set mysql password []
  -h, --host                        set mysql host [localhost]
  -P, --port                        set mysql port [3306]
  -s, --socket                      set mysql socket
  -C, --charset                     set connection character set [UTF8]

  -H, --help                        display this help and exit
  -V, --version                     print version information and exit
  -M, --mysql-info                  print mysql client version and exit

  -B, --buffer                      buffer query result set
  -S, --select                      fields to select
  -W, --where                       where clause
  -O, --order                       order by
  -G, --group                       group by
  -A, --having                      having clause

  --fields-terminated-by            terminate output fields with the given string
  --fields-enclosed-by              enclose output fields with the given character
  --fields-optionally-enclosed-by   optionally enclose output fields with the given character
  --fields-escaped-by               escape output fields with the given character
  --lines-terminated-by             terminate lines with the given string
```

## Compatibility

`mysqldumptab` aims to be 100% compatible with the output `SELECT INTO OUTFILE` produces.

If not, please file a [bug](https://github.com/tzvetkoff/mysqldumptab/issues/new).

## Speed

Speed comparisons are rather subjective but when running on the same host, `mysqldumptab` is ~25% faster than `mysqldump` and `SELECT INTO OUTFILE`.

```
# mysqldump -uroot test_db table_with_500k_records > out.sql
real    0m1.810s
user    0m1.608s
sys     0m0.178s

# mysqldump -uroot --tab=out test_db table_with_500k_records
real    0m1.810s
user    0m0.006s
sys     0m0.002s

# mysqldumptab -uroot test_db table_with_500k_records > out.tab
real    0m1.333s
user    0m0.590s
sys     0m0.216s
```

## Bugs

[!@#$](https://github.com/tzvetkoff/mysqldumptab/issues/new)
