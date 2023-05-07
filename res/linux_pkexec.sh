set -e

command -v pkexec >/dev/null 2>&1 || echo "[Warning] pkexec not found"
command -v pkill >/dev/null 2>&1 || echo "[Warning] pkill not found"

BASEDIR="$(dirname -- "$(readlink -f -- "$0")")"

pkexec --keep-cwd \
  bash "$BASEDIR"/linux_pkexec_root.sh $@
