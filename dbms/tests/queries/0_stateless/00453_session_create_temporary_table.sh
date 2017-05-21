curl -sS 'http://localhost:8123/?session_id=00453' --data "CREATE TEMPORARY TABLE t (x String)"
curl -sS 'http://localhost:8123/?session_id=00453' --data "INSERT INTO t VALUES ('Hello'), ('World')"
curl -sS 'http://localhost:8123/?session_id=00453' --data "SELECT * FROM t ORDER BY x"