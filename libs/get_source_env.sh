if [ ! -z $ENV_NEKORAY ]; then
  export COMMIT_SING_BOX_EXTRA="6f59c055b2bf76f3a44329f0ce266042b463994c"
  export COMMIT_MATSURI_V2RAY="8134d3cc23aa6b8e2a056887addf22d7d22bd969"
fi

if [ ! -z $ENV_SING_BOX_EXTRA ]; then
  source libs/get_source_env.sh
  # export COMMIT_SING_BOX=""
fi
