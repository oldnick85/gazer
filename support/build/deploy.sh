#!/bin/bash
echo "Deploy gazer"
host=$(cat support/secrets/settings.json | jq -r .deploy.host)
user=$(cat support/secrets/settings.json | jq -r .deploy.user)
path=$(cat support/secrets/settings.json | jq -r .deploy.path)
scp build/gazer ${user}@${host}:${path}
scp support/config.json ${user}@${host}:${path}
echo "    start"
ssh -l ${user} ${host} "cd ${path} ; ./gazer config.json &"
echo "    all done"
