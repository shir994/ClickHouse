curl -sS 'http://localhost:8123/?session_id=00455' --data "CREATE TEMPORARY TABLE t (x String)"
curl -sS 'http://localhost:8123/?session_id=00455&session_check=1' --data "INSERT INTO t VALUES ('Hello'), ('World')"
curl -sS 'http://localhost:8123/?session_id=00455&session_check=1' --data "SELECT * FROM t ORDER BY x"