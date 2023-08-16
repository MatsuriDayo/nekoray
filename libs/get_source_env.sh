if [ ! -z $ENV_NEKORAY ]; then
  export COMMIT_SING_BOX_EXTRA="c419bc9a28b23f528072aedbb2d956dbfda0cbb3"
  export COMMIT_MATSURI_XRAY="f6843e4e1442341e88ac34c09ca2d243cdcc09f6"
fi

if [ ! -z $ENV_SING_BOX_EXTRA ]; then
  source libs/get_source_env.sh
  # export COMMIT_SING_BOX=""
fi
