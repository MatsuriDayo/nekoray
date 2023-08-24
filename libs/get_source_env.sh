if [ ! -z $ENV_NEKORAY ]; then
  export COMMIT_SING_BOX_EXTRA="3c5d4ae9b771f216bc0eeabdf19d840c77e29858"
  export COMMIT_MATSURI_XRAY="02b41a79b8053031ffa6a546ae0de08c553bf391"
fi

if [ ! -z $ENV_SING_BOX_EXTRA ]; then
  source libs/get_source_env.sh
  # export COMMIT_SING_BOX=""
fi
