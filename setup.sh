cd daemon
make

sudo cp netagentd /usr/local/sbin/

cd ..

chmod +x cli/netagentctl
sudo cp cli/netagentctl /usr/local/bin/
