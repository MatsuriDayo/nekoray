if [ ! -z $ENV_NEKORAY ]; then
  export COMMIT_SING_BOX_EXTRA="9400fd007ad5a19b5892a7c42aae0951e1163747"
  export COMMIT_MATSURI_XRAY="ded0ebf36791af0d72332a622e262df604b57ebb"
fi

if [ ! -z $ENV_SING_BOX_EXTRA ]; then
  source libs/get_source_env.sh
  # export COMMIT_SING_BOX=""
fi
