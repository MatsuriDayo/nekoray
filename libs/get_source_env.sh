if [ ! -z $ENV_NEKORAY ]; then
  export COMMIT_SING_BOX_EXTRA="5dd9ad10dde99a32ee8a0566498e5281b52b0cb7"
  export COMMIT_MATSURI_V2RAY="8134d3cc23aa6b8e2a056887addf22d7d22bd969"
fi

if [ ! -z $ENV_SING_BOX_EXTRA ]; then
  source libs/get_source_env.sh
  # export COMMIT_SING_BOX=""
fi
