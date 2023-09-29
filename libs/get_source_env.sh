if [ ! -z $ENV_NEKORAY ]; then
  export COMMIT_SING_BOX_EXTRA="bd07dbeca505668da38741b8fec87489c74275b4"
  export COMMIT_MATSURI_XRAY="01208225ee7e508044cca8eb6776a117bcecd997"
fi

if [ ! -z $ENV_SING_BOX_EXTRA ]; then
  source libs/get_source_env.sh
#  export COMMIT_LIBNEKO="a26f0dbb1467bd181f76e2560e956633d9637e9d"
fi
