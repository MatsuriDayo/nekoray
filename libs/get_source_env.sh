if [ ! -z $ENV_NEKORAY ]; then
  export COMMIT_SING_BOX_EXTRA="66e9fa6b85443e5a9d62bcffa2efadc1c033e912"
  export COMMIT_MATSURI_V2RAY="8134d3cc23aa6b8e2a056887addf22d7d22bd969"
fi

if [ ! -z $ENV_SING_BOX_EXTRA ]; then
  source libs/get_source_env.sh
  # export COMMIT_SING_BOX=""
fi
