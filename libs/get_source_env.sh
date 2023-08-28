if [ ! -z $ENV_NEKORAY ]; then
  export COMMIT_SING_BOX_EXTRA="70387142a28f10663b847988306ff6899fce0176"
  export COMMIT_MATSURI_XRAY="fa35868d6906c3415f0e2e654ec5cac951e7f9fc"
fi

if [ ! -z $ENV_SING_BOX_EXTRA ]; then
  source libs/get_source_env.sh
  # export COMMIT_SING_BOX=""
fi
