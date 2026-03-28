python3 -m venv .venv
source .venv/bin/activate
which python
which pip
pip install -e ./lib
python -c "from netagent import NetAgentClient; print('OK')"
./cli/netagentctl list
