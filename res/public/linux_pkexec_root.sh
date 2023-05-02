set -e

if [ "$EUID" -ne 0 ]; then
  echo "[Warning] Not running as root"
fi

#echo $$ >$PID_FILE

$@
