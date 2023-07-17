if [ ! -z $ENV_NEKORAY ]; then
  export COMMIT_SING_BOX_EXTRA="03df5df5b343c2d6da6bfd27656a12353cc457dd"
  export COMMIT_MATSURI_XRAY="3c826392b6c830de88aef209b56782dee0b6fbb5"
fi

if [ ! -z $ENV_SING_BOX_EXTRA ]; then
  source libs/get_source_env.sh
  # export COMMIT_SING_BOX=""
fi
