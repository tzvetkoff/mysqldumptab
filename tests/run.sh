#!/bin/bash

DIR=$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)

[[ -n ${MYSQL_USER} ]] && MYSQL_USER="-u${MYSQL_USER}"
[[ -n ${MYSQL_PASS} ]] && MYSQL_PASS="-p${MYSQL_PASS}"
[[ -z ${MYSQL_USER} ]] && MYSQL_USER='-uroot'

prepare() {
	rm -f ${DIR}/*.tab

	mysql ${MYSQL_USER} ${MYSQL_PASS} -Bse 'DROP DATABASE IF EXISTS mdt_test'
	mysql ${MYSQL_USER} ${MYSQL_PASS} -Bse 'CREATE DATABASE mdt_test CHARSET=utf8 COLLATE=utf8_general_ci'
	mysql ${MYSQL_USER} ${MYSQL_PASS} mdt_test -Bse "CREATE TABLE test_cases (
		id INTEGER UNSIGNED NOT NULL AUTO_INCREMENT,
		f_null TINYINT(1) UNSIGNED NULL,

		f_int INTEGER NOT NULL,
		f_dec DECIMAL(5, 2) NULL,

		f_date DATE NULL,
		f_datetime DATETIME NULL,
		f_timestamp TIMESTAMP NULL,
		f_time TIME NULL,
		f_year_2 YEAR(2) NULL,
		f_year_4 YEAR(4) NULL,

		f_char CHAR(40) NOT NULL,
		f_varchar VARCHAR(255) NOT NULL,
		f_text TEXT NULL,

		f_enum ENUM('val1', 'val2'),
		f_set SET('set1', 'set2', 'set3'),

		f_geometry GEOMETRY NULL,

		PRIMARY KEY (id)
	)"

	for x in {1..3}; do
		mysql ${MYSQL_USER} ${MYSQL_PASS} mdt_test -Bse "INSERT INTO test_cases VALUES (
			NULL,
			NULL,

			0x5f3759df,
			3.14,

			'9999-12-31',
			'9999-12-31 23:59:59',
			'2038-01-19 03:14:07',
			'-838:59:59',
			42,
			1987,

			'Static CHAR(40)',
			'Variable CHAR(255)',
			'Escaping test: \!@#\$%^&*():;\\'\"\\,.<>/?\`~{}[] => Кирилица, мейт! 鋸',

			'val2',
			'set1,set3',

			POINT(15, 20)
		)"
	done
}

compare() {
	d="${DIR}/${2}"
	m="${d/_d.tab/_m.tab}"
	q="${1}"
	f=""

	shift
	shift

	mysql mdt_test -Bse "${q}"
	${DIR}/../mysqldumptab ${MYSQL_USER} ${MYSQL_PASS} ${@} mdt_test test_cases > ${d}

	cmp ${m} ${d} || {
		echo "${q}"
		echo
		cat ${m}
		echo
		cat ${d}
		exit 1
	}
}

prepare

compare "SELECT * FROM test_cases INTO OUTFILE '${DIR}/dump_1_simple_m.tab'" 'dump_1_simple_d.tab'
compare "SELECT * FROM test_cases INTO OUTFILE '${DIR}/dump_2_escaped_m.tab' FIELDS ESCAPED BY '\!'" 'dump_2_escaped_d.tab' '--fields-escaped-by=!'
compare "SELECT * FROM test_cases INTO OUTFILE '${DIR}/dump_3_optionally_enclosed_m.tab' FIELDS OPTIONALLY ENCLOSED BY '*'" 'dump_3_optionally_enclosed_d.tab' '--fields-optionally-enclosed-by=*'
compare "SELECT * FROM test_cases INTO OUTFILE '${DIR}/dump_4_enclosed_by_m.tab' FIELDS ENCLOSED BY '*'" 'dump_4_enclosed_by_d.tab' '--fields-enclosed-by=*'
compare "SELECT * FROM test_cases INTO OUTFILE '${DIR}/dump_5_terminated_by_m.tab' FIELDS TERMINATED BY 'FOO'" 'dump_5_terminated_by_d.tab' '--fields-terminated-by=FOO'
compare "SELECT * FROM test_cases INTO OUTFILE '${DIR}/dump_6_lines_terminated_by_m.tab' LINES TERMINATED BY 'FOO'" 'dump_6_lines_terminated_by_d.tab' '--lines-terminated-by=FOO'
compare "SELECT * FROM test_cases INTO OUTFILE '${DIR}/dump_7_escaped_by_lines_terminated_by_m.tab' FIELDS ESCAPED BY '\!' LINES TERMINATED BY 'FOO'" 'dump_7_escaped_by_lines_terminated_by_d.tab' '--fields-escaped-by=!' '--lines-terminated-by=FOO'
compare "SELECT * FROM test_cases INTO OUTFILE '${DIR}/dump_8_csv_m.tab' FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '\"'" 'dump_8_csv_d.tab' '--fields-terminated-by=,' '--fields-optionally-enclosed-by="'

echo 'Tests passed'
