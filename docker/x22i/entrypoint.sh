#!/usr/bin/env bash
set -euo pipefail

: "${X22I_SIGNING_STORE_PASSWORD_FILE:?Mount the signing password file and set X22I_SIGNING_STORE_PASSWORD_FILE}"
if [[ ! -f "$X22I_SIGNING_STORE_PASSWORD_FILE" || ! -r "$X22I_SIGNING_STORE_PASSWORD_FILE" ]]; then
    echo "The mounted signing password file is unavailable." >&2
    exit 2
fi
X22I_SIGNING_STORE_PASSWORD=$(cat -- "$X22I_SIGNING_STORE_PASSWORD_FILE")
if [[ -z "$X22I_SIGNING_STORE_PASSWORD" ]]; then
    echo "The mounted signing password file is empty." >&2
    exit 2
fi
export X22I_SIGNING_STORE_PASSWORD

exec "$@"
