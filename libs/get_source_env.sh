if [ ! -z $ENV_NEKORAY ]; then
  export COMMIT_SING_BOX_EXTRA="d663cfa6b9f721d306ec3972460b61bb2c935685"
  export COMMIT_MATSURI_V2RAY="8134d3cc23aa6b8e2a056887addf22d7d22bd969"
fi

if [ ! -z $ENV_SING_BOX_EXTRA ]; then
  source libs/get_source_env.sh
  # export COMMIT_SING_BOX=""
fi
