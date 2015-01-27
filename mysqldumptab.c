#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include <my_global.h>
#include <mysql.h>

#include "mdt_str.h"

#define MYSQLDUMPTAB_VERSION "0.0.1"

/* default connection stuff */
#define MYSQLDUMPTAB_DEFAULT_USERNAME ""
#define MYSQLDUMPTAB_DEFAULT_PASSWORD ""
#define MYSQLDUMPTAB_DEFAULT_HOST "localhost"
#define MYSQLDUMPTAB_DEFAULT_PORT 3306

/* define some types that may be missing on older mysql versions */
#ifndef MYSQL_TYPE_TIMESTAMP2
#define MYSQL_TYPE_TIMESTAMP2 MYSQL_TYPE_VARCHAR
#endif

#ifndef MYSQL_TYPE_DATETIME2
#define MYSQL_TYPE_DATETIME2 MYSQL_TYPE_VARCHAR
#endif

#ifndef MYSQL_TYPE_TIME2
#define MYSQL_TYPE_TIME2 MYSQL_TYPE_VARCHAR
#endif

/* print block */
#define MDT_PRINT_BUFFER_BLOCK_SIZE 0x4000

/* does anyone use this type? */
#define MDT_NULL(x) (\
    (x)->type == MYSQL_TYPE_NULL \
)

/* a nice macro to test if a field should be enclosed */
#define MDT_ENCLOSE(x) (\
    (x)->type == MYSQL_TYPE_VARCHAR || \
    (x)->type == MYSQL_TYPE_DATETIME || \
    (x)->type == MYSQL_TYPE_DATETIME2 || \
    (x)->type == MYSQL_TYPE_DATE || \
    (x)->type == MYSQL_TYPE_NEWDATE || \
    (x)->type == MYSQL_TYPE_TIME || \
    (x)->type == MYSQL_TYPE_TIME2 || \
    (x)->type == MYSQL_TYPE_TIMESTAMP || \
    (x)->type == MYSQL_TYPE_TIMESTAMP2 || \
    ((x)->type >= MYSQL_TYPE_ENUM && (x)->type <= MYSQL_TYPE_GEOMETRY) \
)

/* global variables */
static char *username = NULL, *password = NULL, *host = NULL,
    *sock = NULL, *database = NULL, *table = NULL,
    *select_fields = NULL, *where = NULL, *group = NULL,
    *having = NULL, *order = NULL,
    *fields_terminated_by = NULL, fields_enclosed_by = 0,
    fields_optionally_enclosed_by = 0, fields_escaped_by = '\\',
    *lines_terminated_by = NULL, *print_buffer = NULL;
static unsigned int port, fields_terminated_by_length;
static unsigned long print_buffer_size;
char *enclose;

static MYSQL *connection = NULL;
static MYSQL_RES *result = NULL;
static MYSQL_FIELD **fields = NULL;

/* exit handler - release memory, close connections, etc. */
static void
exit_handler()
{
    if (username) {
        free(username);
    }
    if (password) {
        free(password);
    }
    if (host) {
        free(host);
    }
    if (sock) {
        free(sock);
    }
    if (database) {
        free(database);
    }
    if (table) {
        free(table);
    }
    if (select_fields) {
        free(select_fields);
    }
    if (where) {
        free(where);
    }
    if (group) {
        free(group);
    }
    if (having) {
        free(having);
    }
    if (order) {
        free(order);
    }
    if (fields_terminated_by) {
        free(fields_terminated_by);
    }
    if (lines_terminated_by) {
        free(lines_terminated_by);
    }
    if (enclose) {
        free(enclose);
    }
    if (print_buffer) {
        free(print_buffer);
    }
    if (fields) {
        free(fields);
    }
    if (result) {
        mysql_free_result(result);
    }
    if (connection) {
        mysql_close(connection);
    }
}

/* usage */
static void
usage(const char *program_invocation_name, FILE *out)
{
    fputs("\n", out);

    fputs("Usage:\n", out);
    fprintf(out, "  %s [options] <database> <table>\n",
        program_invocation_name);
    fputs("\n", out);

    fputs("Options:\n", out);
    fputs("  -u, --username                    set mysql user ["
        MYSQLDUMPTAB_DEFAULT_USERNAME "]\n", out);
    fputs("  -p, --password                    set mysql password ["
        MYSQLDUMPTAB_DEFAULT_PASSWORD "]\n", out);
    fputs("  -h, --host                        set mysql host ["
        MYSQLDUMPTAB_DEFAULT_HOST "]\n", out);
    fprintf(out, "  -P, --port                        set mysql port [%d]\n",
        MYSQLDUMPTAB_DEFAULT_PORT);
    fputs("  -s, --socket                      set mysql socket\n", out);
    fputs("\n", out);

    fputs("  -H, --help                        display this help and exit\n",
        out);
    fputs("  -V, --version                     print version information "
        "and exit\n", out);
    fputs("  -M, --mysql-info                  print mysql client version "
        "and exit\n", out);
    fputs("\n", out);

    fputs("  -B, --buffer                      buffer query result set\n",
        out);
    fputs("  -S, --select                      fields to select\n", out);
    fputs("  -W, --where                       where clause\n", out);
    fputs("  -O, --order                       order by\n", out);
    fputs("  -G, --group                       group by\n", out);
    fputs("  -A, --having                      having clause\n", out);
    fputs("\n", out);

    fputs("  --fields-terminated-by            terminate output fields "
        "with the given string\n", out);
    fputs("  --fields-enclosed-by              enclose output fields "
        "with the given character\n", out);
    fputs("  --fields-optionally-enclosed-by   optionally enclose output "
        "fields with the given character\n", out);
    fputs("  --fields-escaped-by               escape output fields with the "
        "given character\n", out);
    fputs("  --lines-terminated-by             terminate lines with the "
        "given string\n", out);
    fputs("\n", out);

    exit(out == stderr ? EXIT_FAILURE : EXIT_SUCCESS);
}

/* entry point */
int
main(int argc, char **argv)
{
    int ch, n, n1, password_prompt = 0, buffer = 0, enclose_char;
    unsigned long i, j, k, size_needed, length, *lengths;
    char *p, *src, *dst;
    struct mdt_str_t *query;
    MYSQL_ROW row;

    static const struct option longopts[] = {
        {"username", required_argument, NULL, 'u'},
        {"password", optional_argument, NULL, 'p'},
        {"host", required_argument, NULL, 'h'},
        {"port", required_argument, NULL, 'P'},
        {"socket", required_argument, NULL, 's'},

        {"help", no_argument, NULL, 'H'},
        {"version", no_argument, NULL, 'V'},
        {"mysql-info", no_argument, NULL, 'M'},

        {"buffer", no_argument, NULL, 'B'},
        {"select", required_argument, NULL, 'S'},
        {"where", required_argument, NULL, 'W'},
        {"order", required_argument, NULL, 'O'},
        {"group", required_argument, NULL, 'G'},
        {"having", required_argument, NULL, 'A'},

        {"fields-terminated-by", required_argument, NULL, 0x1000},
        {"fields-enclosed-by", required_argument, NULL, 0x1001},
        {"fields-optionally-enclosed-by", required_argument, NULL, 0x1002},
        {"fields-escaped-by", required_argument, NULL, 0x1003},
        {"lines-terminated-by", required_argument, NULL, 0x1004},
        {NULL, 0, NULL, 0}
    };

    /* initialize default options */
    username = strdup(MYSQLDUMPTAB_DEFAULT_USERNAME);
    password = strdup(MYSQLDUMPTAB_DEFAULT_PASSWORD);
    host = strdup(MYSQLDUMPTAB_DEFAULT_HOST);
    port = MYSQLDUMPTAB_DEFAULT_PORT;
    sock = NULL;

    fields_terminated_by = strdup("\t");
    lines_terminated_by = strdup("\n");
    print_buffer_size = MDT_PRINT_BUFFER_BLOCK_SIZE;
    print_buffer = malloc(print_buffer_size);

    /* register the exit handler */
    atexit(exit_handler);

    /* parse command line options */
    while ((ch = getopt_long(argc, argv, "+u:p::h:P:s:HVMBS:W:O:G:A:",
        longopts, NULL)) != -1) {
        switch (ch) {
            case 'u':
                free(username);
                username = strdup(optarg);
                break;
            case 'p':
                free(password);
                if (optarg) {
                    password = strdup(optarg);
                } else {
                    password_prompt = 1;
                }
                break;
            case 'h':
                free(host);
                host = strdup(optarg);
                break;
            case 'P':
                port = strtol(optarg, &p, 10);
                if (p == optarg) {
                    fprintf(stderr, "Invalid port: %s\n", optarg);
                    return EXIT_FAILURE;
                }
                break;
            case 's':
                sock = strdup(optarg);
                break;

            case 'H':
                usage(argv[0], stdout);
                break;
            case 'V':
                puts(MYSQLDUMPTAB_VERSION);
                return EXIT_SUCCESS;
            case 'M':
                puts(mysql_get_client_info());
                return EXIT_SUCCESS;

            case 'B':
                buffer = 1;
                break;
            case 'S':
                select_fields = strdup(optarg);
                break;
            case 'W':
                where = strdup(optarg);
                break;
            case 'O':
                order = strdup(optarg);
                break;
            case 'G':
                group = strdup(optarg);
                break;
            case 'A':
                having = strdup(optarg);
                break;

            case 0x1000:
                free(fields_terminated_by);
                fields_terminated_by = strdup(optarg);
                break;
            case 0x1001:
                fields_enclosed_by = optarg[0];
                break;
            case 0x1002:
                fields_optionally_enclosed_by = optarg[0];
                break;
            case 0x1003:
                fields_escaped_by = optarg[0];
                break;
            case 0x1004:
                free(lines_terminated_by);
                lines_terminated_by = strdup(optarg);
                break;

            default:
                usage(argv[0], stderr);
        }
    }

    /* check for 2 positional arguments */
    if (argc - optind != 2) {
        usage(argv[0], stderr);
    }

    fields_terminated_by_length = strlen(fields_terminated_by);

    if (password_prompt) {
        p = getpass("Enter password: ");
        password = strdup(p);
        memset(p, 0, strlen(password));
    }

    database = strdup(argv[optind]);
    table = strdup(argv[optind + 1]);

    /* initialize connection */
    if ((connection = mysql_init(NULL)) == NULL) {
        fprintf(stderr, "%s\n", mysql_error(connection));
        return EXIT_FAILURE;
    }

    /* connect to database */
    if (mysql_real_connect(connection, host, username,
        password, database, port, sock, 0) == NULL) {
        fprintf(stderr, "%s\n", mysql_error(connection));
        return EXIT_FAILURE;
    }

    /* just in case */
    free(password);
    password = NULL;

    /* build query */
    query = mdt_str_alloc();
    mdt_str_concat(query, "SELECT /*!40001 SQL_NO_CACHE */ %s FROM ",
        select_fields ? select_fields : "*");
    mdt_str_concat(query, "%s", table);

    if (where) {
        mdt_str_concat(query, " WHERE %s", where);
    }
    if (group) {
        mdt_str_concat(query, " GROUP BY %s", group);
    }
    if (having) {
        mdt_str_concat(query, " HAVING %s", having);
    }
    if (order) {
        mdt_str_concat(query, " ORDER BY %s", order);
    }

    /* execute query */
    if (mysql_query(connection, query->ptr) != 0) {
        mdt_str_free(query);
        fprintf(stderr, "%s\n", mysql_error(connection));
        return EXIT_FAILURE;
    }
    mdt_str_free(query);

    /* store || use result */
    result = buffer ? mysql_store_result(connection) :
        mysql_use_result(connection);
    if (!result) {
        fprintf(stderr, "%s\n", mysql_error(connection));
        return EXIT_FAILURE;
    }

    /* fields count */
    n = mysql_num_fields(result);
    n1 = n - 1;

    /* fetch fields information */
    fields = calloc(n, sizeof(struct st_mysql_field));
    enclose = calloc(n, sizeof(char));
    if (!fields || !enclose) {
        fputs("Cannot allocate memory for fields information\n", stderr);
        return EXIT_FAILURE;
    }

    for (i = 0; i < n; ++i) {
        if ((fields[i] = mysql_fetch_field(result)) == NULL) {
            fprintf(stderr, "myslq_field() failed for field #%lu\n", i);
            return EXIT_FAILURE;
        }

        if (fields_enclosed_by) {
            enclose[i] = enclose_char = fields_enclosed_by;
        } else if (fields_optionally_enclosed_by) {
            enclose[i] = enclose_char = MDT_ENCLOSE(fields[i]) ?
                fields_optionally_enclosed_by : 0;
        }
    }

    /* iterate over rows and print data */
    while ((row = mysql_fetch_row(result))) {
        lengths = mysql_fetch_lengths(result);

        size_needed = 1;
        for (j = 0; j < n; ++j) {
            size_needed += lengths[j] * 2 + fields_terminated_by_length;
        }

        if (print_buffer_size < size_needed) {
            print_buffer_size = floor((double)size_needed /
                (double)MDT_PRINT_BUFFER_BLOCK_SIZE) *
                MDT_PRINT_BUFFER_BLOCK_SIZE;
            print_buffer = realloc(print_buffer, print_buffer_size);
            if (!print_buffer) {
                fprintf(stderr, "Cannot grow print buffer to %lu bytes\n",
                    print_buffer_size);
                return EXIT_FAILURE;
            }
        }

        dst = print_buffer;

        for (i = 0; i < n; ++i) {
            src = row[i];
            length = lengths[i];

            if (!src || MDT_NULL(fields[i])) {
                *dst++ = fields_escaped_by;
                *dst++ = 'N';
            } else {
                if (enclose[i]) {
                    *dst++ = enclose_char;
                }

                /* walk and escape what we need to escape */
                for (j = 0; j < length; ++j) {
                    ch = *src++;

                    switch (ch) {
                        case '\0':
                            *dst++ = fields_escaped_by;
                            *dst++ = '0';
                            break;

                        case '\t':
                        case '\n':
                            *dst++ = fields_escaped_by;
                            *dst++ = ch;
                            break;

                        default:
                            if (ch == fields_escaped_by ||
                                ch == enclose_char) {
                                *dst++ = fields_escaped_by;
                            }

                            *dst++ = ch;
                    }
                }

                if (enclose[i]) {
                    *dst++ = enclose_char;
                }
            }

            if (i < n1) {
                strncpy(dst, fields_terminated_by,
                    fields_terminated_by_length);
                dst += fields_terminated_by_length;
            }
        }

        *dst = '\0';

        fputs(print_buffer, stdout);
        fputs(lines_terminated_by, stdout);
    }

    /* hasta la vista, baby! */
    return EXIT_SUCCESS;
}
