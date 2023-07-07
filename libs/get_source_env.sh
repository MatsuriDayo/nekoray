if [ ! -z $ENV_NEKORAY ]; then
  export COMMIT_SING_BOX_EXTRA="c0127f6e833ecad55f9ba8f6ece60c9a8d7acd86"
  export COMMIT_MATSURI_XRAY="ded0ebf36791af0d72332a622e262df604b57ebb"
fi

if [ ! -z $ENV_SING_BOX_EXTRA ]; then
  source libs/get_source_env.sh
  # export COMMIT_SING_BOX=""
fi
