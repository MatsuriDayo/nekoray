if [ ! -z $ENV_NEKORAY ]; then
  export COMMIT_SING_BOX_EXTRA="00286b4baa9dabdce79d40ccd63a90c862a72254"
  export COMMIT_MATSURI_XRAY="f6843e4e1442341e88ac34c09ca2d243cdcc09f6"
fi

if [ ! -z $ENV_SING_BOX_EXTRA ]; then
  source libs/get_source_env.sh
  # export COMMIT_SING_BOX=""
fi
