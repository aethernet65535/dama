#!/bin/bash

if [ "$(id -u)" -ne 0 ]; then
  echo "please run as root"
  exit 1
fi

echo "============================="
echo "DAMA FEDORA 44 INSTALL SCRIPT"
echo "============================="

dnf install \
make \
gcc \
ollama \
sysstat \
python3

ollama serve
ollama pull gemma3:4b

systemctl enable --now sysstat
