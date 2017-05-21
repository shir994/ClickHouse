curl -sS "http://localhost:8123/?session_id=00454&session_timeout=5" --data "CREATE TEMPORARY TABLE t (x String)"
curl -sS "http://localhost:8123/?session_id=00454&session_timeout=5" --data "INSERT INTO t VALUES ('Hello'), ('World')"
sleep 4
curl -sS "http://localhost:8123/?session_id=00454&session_timeout=5" --data "SELECT * FROM t ORDER BY x"
sleep 4
curl -sS "http://localhost:8123/?session_id=00454&session_timeout=10" --data "SELECT * FROM t ORDER BY x"
sleep 8
curl -sS "http://localhost:8123/?session_id=00454&session_timeout=1" --data "SELECT * FROM t ORDER BY x"