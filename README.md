# mysqldumptab

A console tool that dumps MySQL tables as whatever-separated data.

## TL;DR;

`mysqldump --tab` actually calls `SELECT * INTO OUTFILE` on the *server*, so it's not really useful when you need to dump a table from a remote server.

`mysqldumptab` leverages that as it dumps a table in the exact same format as `SELECT INTO OUTFILE` but does that on the client side.

## Dependencies

A compiler.

MySQL client libraries and `build-essential` or `devel`, depending on your OS.

## Installation

Just like any software that comes with `configure` scripts:

``` bash
./bootstrap
./configure --disable-shared --prefix=/usr/local
make install clean
```

## Tweaking

The configure script provides several options for tweaking:


| Option                         | Description                                                                              |
| ------------------------------ | ---------------------------------------------------------------------------------------- |
| `--disable-shared`             | Link the resulting binary against static libraries only. **Recommended**.                |
| `--with-mysql-config`          | Provide an alternate path to `mysql_config`.                                             |
| `--with-buffer-block-size=VAL` | Set the growing output buffer block size to `VAL`. The default value is `0x4000` (16KB). |

## Usage

Some options like `-uphPs` are the same as `mysqldump`.

If you pass `-p` with no argument, you'll get a password prompt.

Options that take characters support hex format - `--fields-escaped-by=0x5c`.

```
Usage:
  mysqldumptab [options] <database> <table>

Options:
  -u, --username                    set mysql user []
  -p, --password                    set mysql password []
  -h, --host                        set mysql host [localhost]
  -P, --port                        set mysql port [3306]
  -s, --socket                      set mysql socket
  -C, --charset                     set connection character set [UTF8]
  -o, --output-file                 write output to file instead of stdout

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
